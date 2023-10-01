#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "json_loader.h"
#include "request_handler.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

// del
namespace beast = boost::beast;
namespace http = beast::http;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
// del

using namespace std::literals;

namespace {

// del
// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, size_t size, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    if (status == http::status::method_not_allowed) {
        response.set(http::field::allow, "GET, HEAD");
    }
    response.body() = body;
    response.content_length(size);
    response.keep_alive(keep_alive);
    return response;
}

StringResponse HandleRequest(StringRequest&& req) {
    std::string req_view(req.method_string());
    std::cout << "RequestHandler::HandleRequest" << req_view << std::endl;
    const auto text_response = [&req](http::status status, std::string_view text, size_t size) {
        return MakeStringResponse(status, text, size, req.version(), req.keep_alive(), std::move("text/html"sv));
    };

    std::string response_body("Hello, "sv);
    auto req_method = req.method();
    std::string_view req_target = req.target();
    response_body += std::string(req_target.begin()+1,req_target.end());

    if (req_method == http::verb::get) {
        return text_response(http::status::ok, response_body, response_body.size());
    } else if (req_method == http::verb::head) {
        return text_response(http::status::ok, "", response_body.size());
    } else {
        std::string response_not_allowed_body("Invalid method"sv);
        return text_response(http::status::method_not_allowed, response_not_allowed_body, response_not_allowed_body.size());
    }
}
// del



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
    if (argc != 2) {
        std::cerr << "Usage: game_server <game-config-json>"sv << std::endl;
        return EXIT_FAILURE;
    }
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);

        // 2. Инициализируем io_context
//        const unsigned num_threads = std::thread::hardware_concurrency();
        const unsigned num_threads = 1u;
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        // Подписываемся на сигналы и при их получении завершаем работу сервера
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
            }
            std::cout << "Server has been finished by system signal..."sv << std::endl;
        });

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        http_handler::RequestHandler handler{game};

        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
/*
        http_server::ServeHttp(ioc, {address, port}, [&handler](auto&& req, auto&& send) {
            std::cout << "lambada call, that defined in ServeHttp in main" << std::endl;
            handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });
*/
    http_server::ServeHttp(ioc, {address, port}, [](auto&& req, auto&& sender) {
            std::cout << "lambada call, that defined in ServeHttp in main" << std::endl;
            sender(HandleRequest(std::forward<decltype(req)>(req)));
    });

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        std::cout << "Server has started..."sv << std::endl;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
