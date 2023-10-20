#pragma once

#include "http_server.h"
#include "api_handler.h"
#include "file_handler.h"

namespace http_handler {

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit RequestHandler(Strand api_strand, model::Game& game, fs::path path)
        : api_handler_{api_strand, game}
        , file_handler_{path} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        auto version = req.version();
        auto keep_alive = req.keep_alive();

        try {
            // Запрос к АПИ обрабатываем в выделенном Strand, чтобы избежать гонки
            if (api_handler_.IsApiRequest(req.target())) {
                auto handle = [self = shared_from_this(), send,
                               req = std::forward<decltype(req)>(req), version, keep_alive] {
                    try {
                        // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                        assert(self->api_handler_.GetStrand().running_in_this_thread());
                        return send(self->api_handler_.HandleApiRequest(req));
                    } catch (...) {
                        send(self->ReportServerError(version, keep_alive));
                    }
                };
                return net::dispatch(api_handler_.GetStrand(), handle);
            }
            // Не пошли в запрос к АПИ, значит запрос к ФС. Возвращаем результат обработки запроса к файлу
            return std::visit(
                [&send](auto&& result) {
                    send(std::forward<decltype(result)>(result));
                },
                file_handler_.HandleFileRequest(req));
        } catch (...) {
            send(ReportServerError(version, keep_alive));
        }
    }

private:
    // Обработчик запросов к файловой системе
    FileRequestResult HandleFileRequest(const StringRequest& req) const;
    // Обработчик запросов к АПИ - всегда возвращает ответ в виде строки. 
    StringResponse HandleApiRequest(const StringRequest& req) const;
    // Генератор сообщения об ошибке
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;

    ApiHandler api_handler_;
    FileHandler file_handler_;
};

}  // namespace http_handler
