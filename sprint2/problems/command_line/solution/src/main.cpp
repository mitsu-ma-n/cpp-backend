#include "sdk.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>  // add_console_log()
#include <boost/program_options.hpp>

#include <iostream>
#include <memory>
#include <thread>

#include "server_params.h"
#include "json_loader.h"
#include "request_handler.h"
#include "model.h"
#include "logger.h"
#include "request_handler_logging.h"
#include "json_fields.h"
#include "app.h"
#include "utils.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

namespace fs = std::filesystem;
namespace logging = boost::log;


namespace {

struct Args {
    bool is_dt_set = false;
    unsigned long dt;
    std::string config_file;
    std::string static_path;
    bool is_random_spawn;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc{"All options"s};

    Args args;
    desc.add_options()
        // Добавляем опцию --help и её короткую версию -h
        ("help,h", "Show help")
        // Опция --tick-period milliseconds, сохраняющая свои аргументы в поле args.dt
        ("tick-period,t", po::value(&args.dt)->value_name("milliseconds"s), "set tick period")
        // Опция --config-file file, сохраняющая свой аргумент в поле args.config_file
        ("config-file,c", po::value(&args.config_file)->value_name("file"s), "set config file path")
        // Опция --www-root path, сохраняющая свой аргумент в поле args.static_path
        ("www-root,w", po::value(&args.static_path)->value_name("dir"s), "set static files root")
        // Опция --randomize-spawn-points, сохраняющая свой аргумент в поле args.is_random_spawn
        ("randomize-spawn-points", po::bool_switch(&args.is_random_spawn),"spawn dogs at random positions");

    // variables_map хранит значения опций после разбора
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        // Если был указан параметр --help, то выводим справку и возвращаем nullopt
        std::cout << desc;
        return std::nullopt;
    }

    // Проверяем наличие опций
    if (vm.contains("tick-period"s)) {
        args.is_dt_set = true;
    }
    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("Config file path have not been specified"s);
    }
    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("Static files root is not specified"s);
    }

    // С опциями программы всё в порядке, возвращаем структуру args
    return args;
}

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    try {
        std::optional<Args> args = ParseCommandLine(argc, argv);
        if ( !args.has_value() ) {
            return EXIT_SUCCESS;
        }

        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(args->config_file);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
//        const unsigned num_threads = 1u;  // Для отладки в одном потоке
        net::io_context ioc(num_threads);

        // strand для выполнения запросов к API
        auto api_strand = net::make_strand(ioc);

        // Объект Application содержит сценарии использования
        app::Application app(game);

        if (args->is_dt_set) {
            // Настраиваем вызов метода Application::Tick каждые 50 миллисекунд внутри strand
            auto ticker = std::make_shared<utils::Ticker>(api_strand, std::chrono::milliseconds(args->dt),
                [&app](std::chrono::milliseconds delta) { app.ExecuteTick(delta.count()); }
            );
            ticker->Start();
        }

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        // Подписываемся на сигналы и при их получении завершаем работу сервера
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
            }
            BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, boost::json::value({json_field::ERROR_CODE, 0}))
                                    << server_params::EXIT_MESSAGE;
        });

        // Устанавливаем путь к статическим файлам
        fs::path base_path{std::string(args->static_path)};

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        auto handler = make_shared<http_handler::RequestHandler>(api_strand, app, base_path);

        http_handler::LoggingRequestHandler logging_handler{*handler};

        const auto address = net::ip::make_address(server_params::ADRESS);
        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        http_server::ServeHttp(ioc, {address, server_params::PORT}, [&logging_handler](auto&& req, auto&& send, auto&& endpoint) {
            logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send), std::forward<decltype(endpoint)>(endpoint));
        });

        boost::log::add_common_attributes(); 
        logging::add_console_log( 
            std::cout,
            boost::log::keywords::format = &logger::MyFormatter,
            logging::keywords::auto_flush = true
        );

        boost::json::object server_params_jobject;
        server_params_jobject[json_field::SERVER_PORT] = server_params::PORT;
        server_params_jobject[json_field::SERVER_ADDRESS] = server_params::ADRESS;
        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, boost::json::value(server_params_jobject))
                                << server_params::START_MESSAGE;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
