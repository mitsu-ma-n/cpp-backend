#pragma once

#include <string>
#include <utility>

#include "http_server.h"
#include "model.h"
#include "http_handler_types.h"
#include "app.h"

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
    StringResponse HandleApiRequest(const StringRequest& req);
    const Strand& GetStrand();

private:
    // Вспомогательные функции
    std::vector<std::string> GetSegmentsFromPath(boost::core::string_view s) const;
    std::string GetPathFromUri(boost::core::string_view s) const;
    std::string_view GetTokenFromRequestStr(std::string_view str);

    // Функции обработки запросов к API
    StringResponse GetMapsResponse(const StringRequest& req, const std::vector<std::string>& segments) const;
    StringResponse GetGameResponse(const StringRequest& req, const std::vector<std::string>& segments);
    StringResponse GetJoinResponse(const StringRequest& req, const std::vector<std::string>& segments);
    StringResponse GetPlayersResponse(const StringRequest& req, const std::vector<std::string>& segments);
    StringResponse GetStateResponse(const StringRequest& req, const std::vector<std::string>& segments);

    // Функции проверки доступа к элементам АПИ
    bool isMapsRequest(const std::vector<std::string>& segments) const;
    bool isGameRequest(const std::vector<std::string>&  segments) const;
    bool isPlayersRequest(const std::vector<std::string>&  segments) const;
    bool isJoinRequest(const std::vector<std::string>&  segments) const;
    bool isStateRequest(const std::vector<std::string>&  segments) const;

    http::status GetMaps(std::string& response, const std::vector<std::string>& segments) const;
    http::status GetMap(std::string& response, const std::vector<std::string>& segments) const;
    http::status JoinGame(JoinParams params, std::string& response_body);
    http::status GetPlayers(std::string_view token, std::string& response_body);
    http::status GetState(std::string_view token, std::string& response_body);

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status, std::string_view body, size_t size, unsigned http_version,
                                  bool keep_alive, std::string_view content_type, std::string_view allowed_method) const;

    // Генератор сообщения об ошибке
    std::string GenerateErrorResponse(const std::string& code, const std::string& msg) const;

    Strand api_strand_;
    app::Application& app_;
};

}  // namespace http_handler
