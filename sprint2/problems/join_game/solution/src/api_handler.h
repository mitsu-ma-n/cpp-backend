#pragma once

#include <string>
#include <utility>

#include "http_server.h"
#include "model.h"
#include "http_handler_types.h"
#include "app_use_cases.h"

#include <iostream>
#include <filesystem>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using namespace std::literals;

class ApiHandler : public std::enable_shared_from_this<ApiHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit ApiHandler(Strand api_strand, app::Application& app)
        : api_strand_{api_strand}
        , app_{app} {
    }

    ApiHandler(const ApiHandler&) = delete;
    ApiHandler& operator=(const ApiHandler&) = delete;

    bool IsApiRequest(std::string_view target);

    // Обработчик запросов к АПИ - всегда возвращает ответ в виде строки. 
    StringResponse HandleApiRequest(const StringRequest& req) const;
    const Strand& GetStrand();

private:
    std::vector<std::string> GetSegmentsFromPath(boost::core::string_view s) const;
    std::string GetPathFromUri(boost::core::string_view s);
    // Функции обработки запросов к API
    http::status GetApiResponse(std::string& response, const std::vector<std::string>& segments) const;
    http::status GetMaps(std::string& response, const std::vector<std::string>& segments) const;
    http::status GetMap(std::string& response, const std::vector<std::string>& segments) const;

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status, std::string_view body, size_t size, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type) const;

    // Генератор сообщения об ошибке
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;

    Strand api_strand_;
    app::Application& app_;
};

}  // namespace http_handler
