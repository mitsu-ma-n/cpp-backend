#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include "http_server.h"
#include "model.h"

#include <iostream>
#include <filesystem>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

namespace fs = std::filesystem;


using namespace std::literals;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Ответ, тело которого представлено в виде бинарной последовательности
using FileResponse = http::response<http::file_body>;

struct ResponseError {
    std::string code;
    std::string message;
};

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view APP_JSON          = "application/json"sv;
    constexpr static std::string_view APP_OCT_STREAM    = "application/octet-stream"sv;
    constexpr static std::string_view APP_XML           = "application/xml"sv;
    constexpr static std::string_view AUDIO_MPEG        = "audio/mpeg"sv;
    constexpr static std::string_view IMG_BMP           = "image/bmp"sv;
    constexpr static std::string_view IMG_GIF           = "image/gif"sv;
    constexpr static std::string_view IMG_ICON          = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMG_JPEG          = "image/jpeg"sv;
    constexpr static std::string_view IMG_PNG           = "image/png"sv;
    constexpr static std::string_view IMG_SVG           = "image/svg+xml"sv;
    constexpr static std::string_view IMG_TIFF          = "image/tiff"sv;
    constexpr static std::string_view TEXT_CSS          = "text/css"sv;
    constexpr static std::string_view TEXT_HTML         = "text/html"sv;
    constexpr static std::string_view TEXT_JS           = "text/javascript"sv;
    constexpr static std::string_view TEXT_PLAIN        = "text/plain"sv;
};

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit RequestHandler(Strand api_strand, model::Game& game)
        : api_strand_{api_strand}
        , game_{game} {
        // Поддерживаемые расширения файлов для отдачи, для которых можем указать ContentType
        supported_files_[".htm"s]  = ContentType::TEXT_HTML;
        supported_files_[".html"s] = ContentType::TEXT_HTML;
        supported_files_[".css"s]  = ContentType::TEXT_CSS;
        supported_files_[".txt"s]  = ContentType::TEXT_PLAIN;
        supported_files_[".js"s]   = ContentType::TEXT_JS;
        supported_files_[".json"s] = ContentType::APP_JSON;
        supported_files_[".xml"s]  = ContentType::APP_XML;
        supported_files_[".png"s]  = ContentType::IMG_PNG;
        supported_files_[".jpg"s]  = ContentType::IMG_JPEG;
        supported_files_[".jpe"s]  = ContentType::IMG_JPEG;
        supported_files_[".jpeg"s] = ContentType::IMG_JPEG;
        supported_files_[".gif"s]  = ContentType::IMG_GIF;
        supported_files_[".bmp"s]  = ContentType::IMG_BMP;
        supported_files_[".ico"s]  = ContentType::IMG_ICON;
        supported_files_[".tiff"s] = ContentType::IMG_TIFF;
        supported_files_[".tif"s]  = ContentType::IMG_TIFF;
        supported_files_[".svg"s]  = ContentType::IMG_SVG;
        supported_files_[".svgz"s] = ContentType::IMG_SVG;
        supported_files_[".mp3"s]  = ContentType::AUDIO_MPEG;
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    // Тип ответа. std::variant позволяет возвращать разные типы ответа.
    typedef std::variant<FileResponse, StringResponse> Response;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        auto version = req.version();
        auto keep_alive = req.keep_alive();

        try {
            // Запрос к АПИ обрабатываем в выделенном Strand, чтобы избежать гонки
            if (IsApiRequest(req.target())) {
                auto handle = [self = shared_from_this(), send,
                               req = std::forward<decltype(req)>(req), version, keep_alive] {
                    try {
                        // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                        assert(self->api_strand_.running_in_this_thread());
                        return send(self->HandleApiRequest(req));
                    } catch (...) {
                        send(self->ReportServerError(version, keep_alive));
                    }
                };
                return net::dispatch(api_strand_, handle);
            }
            // Не пошли в запрос к АПИ, значит запрос к ФС. Возвращаем результат обработки запроса к файлу
            return std::visit(
                [&send](auto&& result) {
                    send(std::forward<decltype(result)>(result));
                },
                HandleFileRequest(req));
        } catch (...) {
            send(ReportServerError(version, keep_alive));
        }

        // Обработали запрос и сгенерировали ответ
        Response response = HandleRequest(std::forward<decltype(req)>(req));
        // Отправляем ответ с определением типа
        // response -> arg
        std::visit([&](auto&& arg){ send(arg); }, response);
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
    std::string GetFileExtention(const std::string& file_name);
    std::string GetContentType(const std::string& file_name);

private:

    bool IsApiRequest(std::string_view target);

    // @todo
    // using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;
    using FileRequestResult = std::variant<StringResponse, FileResponse>;

    // Обработчик запросов к файловой системе
    FileRequestResult HandleFileRequest(const StringRequest& req) const;
    // Обработчик запросов к АПИ - всегда возвращает ответ в виде строки. 
    StringResponse HandleApiRequest(const StringRequest& req) const;
    // Генератор сообщения об ошибке
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;

    Strand api_strand_;
    model::Game& game_;
    fs::path server_files_path_;
    std::unordered_map<std::string, std::string> supported_files_;
};

}  // namespace http_handler
