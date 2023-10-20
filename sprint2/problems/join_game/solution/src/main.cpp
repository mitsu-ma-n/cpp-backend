#include "sdk.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>  // add_console_log()
#include <iostream>
#include <memory>
#include <thread>

#include "server_params.h"
#include "json_loader.h"
#include "request_handler.h"
#include "model.h"
#include "logger.h"
#include "logging_request_handler.h"
#include "json_fields.h"


using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

namespace fs = std::filesystem;
namespace logging = boost::log;


namespace {

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
    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <static-files-folder>"sv << std::endl;
        return EXIT_FAILURE;
    }
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
//        const unsigned num_threads = 1u;  // Для отладки в одном потоке
        net::io_context ioc(num_threads);

        // strand для выполнения запросов к API
        auto api_strand = net::make_strand(ioc);

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
        fs::path base_path{std::string(argv[2])};

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        auto handler = make_shared<http_handler::RequestHandler>(api_strand, game, base_path);

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
