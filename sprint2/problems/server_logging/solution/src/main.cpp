#include "sdk.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>  // add_console_log()
#include <iostream>
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

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        // Подписываемся на сигналы и при их получении завершаем работу сервера
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
            }
            boost::json::value normal_exit_json_message{{json_field::ERROR_CODE, 0}};
            BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, normal_exit_json_message)
                                    << "Server has been finished by system signal..."sv;
        });

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        http_handler::RequestHandler handler{game};
        http_handler::LoggingRequestHandler logging_handler{handler};

        // Устанавливаем путь к статическим файлам
        fs::path base_path{std::string(argv[2])};
        handler.SetServerFilesPath(base_path);

        const auto address = net::ip::make_address(server_params::ADRESS);
        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        http_server::ServeHttp(ioc, {address, server_params::PORT}, [&handler](auto&& req, auto&& send) {
            handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });

        boost::log::add_common_attributes();
        logging::add_console_log( 
            std::clog,
            boost::log::keywords::format = &logger::MyFormatter
        );


        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, {json_field::ERROR_CODE, 0})
                                << boost::log::add_value(additional_data, {json_field::ERROR_MESSAGE, server_params::START_MESSAGE});

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
