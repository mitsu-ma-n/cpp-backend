#pragma once
// boost.beast будет использовать std::string_view вместо boost::string_view
#include <utility>
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "http_server.h"
#include "model.h"

#include <iostream>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;

using namespace std::literals;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view APP_JSON = "application/json"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        std::cout << "RequestHandler::operator() process req" << std::endl;
        send(HandleRequest(std::forward<decltype(req)>(req)));
    }

    StringResponse HandleRequest(StringRequest&& req);
    StringResponse MakeStringResponse(http::status status, std::string_view body, size_t size, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = ContentType::TEXT_HTML);

private:
    model::Game& game_;
};

}  // namespace http_handler
