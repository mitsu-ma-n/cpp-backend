#pragma once

#include <utility>
#include <boost/variant.hpp>

#include "http_server.h"
#include "model.h"

#include <iostream>
#include <filesystem>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;

namespace fs = std::filesystem;


using namespace std::literals;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Ответ, тело которого представлено в виде бинарной последовательности
using FileResponse = http::response<http::file_body>;

struct ExeptionInfo {
    http::status status;
    std::string body;
};

struct ResponseError {
    std::string code;
    std::string message;
};

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view APP_JSON          = "application/json"sv;
    constexpr static std::string_view APP_OCT_STREAM    = "application/octet-stream"sv;
    constexpr static std::string_view TEXT_HTML         = "text/html"sv;
    constexpr static std::string_view TEXT_PLAIN        = "text/plain"sv;
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    // Тип ответа. boost::variant позволяет возвращать разные типы ответа.
    typedef boost::variant<FileResponse, StringResponse> Response;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработали запрос и сгенерировали ответ
        Response response = HandleRequest(std::forward<decltype(req)>(req));

        // Отправляем ответ с определением типа
        // Можно попробовать сделать красиво через visitor, но пока не получилось
        if (response.type() == typeid(FileResponse)) {
            send(boost::get<FileResponse>(response));
        } else if (response.type() == typeid(StringResponse)) {
            send(boost::get<StringResponse>(response));
        }
    }

    // Функции обработки запросов к API
    http::status GetApiResponse(std::string& response, const std::vector<std::string>& segments);
    http::status GetMaps(std::string& response, const std::vector<std::string>& segments);
    http::status GetMap(std::string& response, const std::vector<std::string>& segments);

    // Функции обработки запросов к файловой системе
    http::status GetFsResponse(http::file_body& response, const std::string& segments);

    Response HandleRequest(StringRequest&& req);
    StringResponse MakeStringResponse(http::status status, std::string_view body, size_t size, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = ContentType::TEXT_HTML);
    FileResponse MakeFileResponse(http::status status, http::file_body::value_type& body, size_t size, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type);

    void SetServerFilesPath(fs::path path);

private:
    model::Game& game_;
    fs::path server_files_path;
};

}  // namespace http_handler
