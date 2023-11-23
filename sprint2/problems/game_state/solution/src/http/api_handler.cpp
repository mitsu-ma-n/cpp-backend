#include "api_handler.h"

#include "join_use_case.h"
#include "players_use_case.h"
#include "state_use_case.h"
#include "maps_use_case.h"

#include "boost/beast/http/status.hpp"
#include <boost/beast/http/file_body.hpp>
#include <boost/json.hpp>
#include "boost/json/serialize.hpp"
#include <boost/url.hpp>
#include <iostream>
#include <string_view>

#include "boost/json/value_from.hpp"
#include "http_handler_types.h"
#include "model.h"
#include "json_fields.h"
#include "json_loader.h"
#include "http_handler_defs.h"

namespace json = boost::json;
namespace urls = boost::urls;
namespace core = boost::core;

namespace http_handler {

const ApiHandler::Strand& ApiHandler::GetStrand() {
    return api_strand_;
}

bool ApiHandler::IsApiRequest(std::string_view target) {
    auto clear_url = GetPathFromUri(target);
    return clear_url.find(api_strings::MAIN_PATH) == 0;    // Запрос к АПИ, если путь начинается с /api/
}

std::string ApiHandler::GetPathFromUri(core::string_view s) const {
    urls::url_view u(s);
    std::string decoded = u.path();
    // Почему-то плюсы автоматом не переводятся в пробелы. Добавлена дополнительная обработка
    std::replace(decoded.begin(), decoded.end(), '+', ' ');
    return decoded;
}

// Функция представляет путь вида "/my/nice/path" как массив строк ["my", "nice", "path"]
std::vector<std::string> ApiHandler::GetSegmentsFromPath(core::string_view s) const {
    urls::url_view u (s);
    std::vector<std::string> seq;
    for(auto seg : u.segments())
        seq.push_back(seg);
    return seq;
}

StringResponse ApiHandler::HandleApiRequest(const StringRequest& req) {
    // Определяем цель запроса
    std::string req_target(req.target());

    // Удаляем завершающий слэш, чтобы не мешал разбору
    while (!req_target.empty() && req_target.back() == '/') {
        req_target.pop_back();
    }
    // Представление пути в виде массива с именами
    auto segments = GetSegmentsFromPath(std::string_view(req_target));
    auto clear_url = GetPathFromUri(std::string_view(req_target));

    if ( isGameRequest(segments) ) {
        return GetGameResponse(req, segments);
    } else if ( isMapsRequest(segments) ) {
        return GetMapsResponse(req, segments);
    } 
    // Неизвестная цель запроса
    auto body = GenerateErrorResponse(json_field::API_CODE_BAD_REQUEST, "Unknown requst target"s);
    return this->MakeStringResponse(http::status::bad_request, body, body.size(), 
        req.version(), req.keep_alive(), ContentType::APP_JSON, AllowedMethods::ERROR);
}

bool ApiHandler::isMapsRequest(const std::vector<std::string>&  segments) const {
    return segments[api_strings::TARGET_POS] == api_strings::MAPS_PATH;
}

StringResponse ApiHandler::GetMapsResponse(const StringRequest& req, const std::vector<std::string>& segments) const {
    std::string req_target(req.target());
    std::string content_type(ContentType::APP_JSON);
    std::string response_body;
    std::string_view allowed_method(AllowedMethods::MAPS);

    http::status status = GetMaps(response_body, segments);

    // Сначала хотелось выделить следующий блок в отдельную функцию, но оказалось, что к разным веткам АПИ
    // допустимы разные методы запросов. Поэтому обрабатывать метод нужно в каждой ветке отдельно
    int response_size = 0;
    auto req_method = req.method();
    if (req_method == http::verb::get) {
        response_size = response_body.size();
    } else if (req_method == http::verb::head) {
        response_size = response_body.size();   // Запоминаем размер
        response_body = ""s;    // Зачищаем тело запроса
    } else {    // Недопустимый метод
        status = http::status::method_not_allowed;
        // Сгенерировать JSON с ошибкой
        response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_METHOD, "Only GET, HEAD method is expected"s);
        response_size = response_body.size();   // Запоминаем размер
    }

    return this->MakeStringResponse(status, response_body, response_size, req.version(), req.keep_alive(), content_type, allowed_method);
}

bool ApiHandler::isGameRequest(const std::vector<std::string>&  segments) const {
    return segments[api_strings::TARGET_POS] == api_strings::GAME_PATH;
}

StringResponse ApiHandler::GetGameResponse(const StringRequest& req, const std::vector<std::string>& segments) {
    if ( isPlayersRequest(segments) ) {
        return GetPlayersResponse(req, segments);
    } else if ( isJoinRequest(segments) ) {
        return GetJoinResponse(req, segments);
    } else if ( isStateRequest(segments) ) {
        return GetStateResponse(req, segments);
    }
    // Неизвестная цель запроса
    auto body = GenerateErrorResponse(json_field::API_CODE_BAD_REQUEST, "Bad request to the game");
    return this->MakeStringResponse(http::status::bad_request, body, body.size(), 
        req.version(), req.keep_alive(), ContentType::APP_JSON, AllowedMethods::ERROR);
}

bool ApiHandler::isPlayersRequest(const std::vector<std::string>&  segments) const {
    return segments[api_strings::ACTION_POS] == api_strings::PLAYERS_PATH;
}

StringResponse ApiHandler::GetPlayersResponse(const StringRequest& req, const std::vector<std::string>& segments) {
    std::string content_type(ContentType::APP_JSON);
    std::string response_body;
    http::status status;
    std::string_view allowed_method(AllowedMethods::PLAYERS);

    // Полуаем токен авторизации
    auto token = GetTokenFromRequestStr(req[http::field::authorization]);
    if (!token.empty()) {
        // Получаем список игроков в виде response_body для игрока с токеном token
        try {
            status = GetPlayers(token, response_body);
        } catch (app::ListPlayersError err) {
            if(err.reason_ == app::ListPlayersErrorReason::InvalidToken) {
                response_body = GenerateErrorResponse(json_field::API_CODE_UNKNOWN_TOKEN, "Player token has not been found");
                status = http::status::unauthorized;
            }
        } catch (...) {
            response_body = GenerateErrorResponse(json_field::API_CODE_UNKNOWN_TOKEN, "Unknown error");
            status = http::status::bad_request;
        }
    } else {
        status = http::status::unauthorized;
        response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_TOKEN, "Authorization header is missing"s);
    }

    // Сначала хотелось выделить следующий блок в отдельную функцию, но оказалось, что к разным веткам АПИ
    // допустимы разные методы запросов. Поэтому обрабатывать метод нужно в каждой ветке отдельно
    int response_size = 0;
    auto req_method = req.method();
    if (req_method == http::verb::get) {
        response_size = response_body.size();
    } else if (req_method == http::verb::head) {
        response_size = response_body.size();   // Запоминаем размер
        response_body = ""s;    // Зачищаем тело запроса
    } else {    // Недопустимый метод
        status = http::status::method_not_allowed;
        // Сгенерировать JSON с ошибкой
        response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_METHOD, "Invalid method"s);
        response_size = response_body.size();   // Запоминаем размер
    }

    auto response = this->MakeStringResponse(status, response_body, response_size, req.version(), req.keep_alive(), content_type, allowed_method);
    response.set(http::field::cache_control, HttpFildsValue::NO_CACHE);
    return response;
}

std::string_view ApiHandler::GetTokenFromRequestStr(std::string_view str) {
    if ( auto pos = str.find(TokenParams::START_STR) != str.npos ) {
        return str.substr(pos+TokenParams::START_STR.size()-1);
    }
    return {};
}

http::status ApiHandler::GetPlayers(std::string_view token, std::string& response_body) {
    app::ListPlayersResult res = app_.GetPlayers(token);
    response_body = boost::json::serialize(json::value_from(res));
    return http::status::ok;
}

bool ApiHandler::isJoinRequest(const std::vector<std::string>&  segments) const {
    return segments[api_strings::ACTION_POS] == api_strings::JOIN_PATH;
}

StringResponse ApiHandler::GetJoinResponse(const StringRequest& req, const std::vector<std::string>& segments) {
    std::string req_target(req.target());
    std::string content_type(ContentType::APP_JSON);
    std::string response_body;
    std::string_view allowed_method(AllowedMethods::JOIN);

    auto req_content_type = req[http::field::content_type];
    // Проверям корректность запроса (сразу извлекаем данные???)
    if (req_content_type != ContentType::APP_JSON) {
        // обработка ошибки
    }

    http::status status = http::status::ok;

    // Парсим JSON
    std::string req_body(req.body());
    JoinParams params;
    try {
        json_loader::ReadJoinParamsFromString(params, req_body);
        status = JoinGame(params, response_body);
    } catch (std::exception err) {
        response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_ARGUMENT, "Join game request parse error");
        status = http::status::bad_request;
    } catch (app::JoinGameError err) {
        if(err.reason_ == app::JoinGameErrorReason::InvalidName) {
            response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_ARGUMENT, "Invalid name");
            status = http::status::bad_request;
        } else if (err.reason_ == app::JoinGameErrorReason::InvalidMap) {
            response_body = GenerateErrorResponse(json_field::API_CODE_MAP_NOT_FOUND, "Map not found");
            status = http::status::not_found;
        }
    }

    // Сначала хотелось выделить следующий блок в отдельную функцию, но оказалось, что к разным веткам АПИ
    // допустимы разные методы запросов. Поэтому обрабатывать метод нужно в каждой ветке отдельно
    auto req_method = req.method();
    if (req_method != http::verb::post) {   // Недопустимый метод
        status = http::status::method_not_allowed;
        // Сгенерировать JSON с ошибкой
        response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_METHOD, "Only POST method is expected"s);
    }

    auto response = this->MakeStringResponse(status, response_body, response_body.size(), req.version(), req.keep_alive(), content_type, allowed_method);
    response.set(http::field::cache_control, HttpFildsValue::NO_CACHE);
    return response;
}

http::status ApiHandler::JoinGame(JoinParams params, std::string& response_body) {
    app::JoinGameResult res = app_.JoinGame(params.name, params.map_id);
    response_body = boost::json::serialize(json::value_from(res));
    return http::status::ok;
}

bool ApiHandler::isStateRequest(const std::vector<std::string>&  segments) const {
    return segments[api_strings::ACTION_POS] == api_strings::STATE_PATH;
}

StringResponse ApiHandler::GetStateResponse(const StringRequest& req, const std::vector<std::string>& segments) {
    std::string content_type(ContentType::APP_JSON);
    std::string response_body;
    http::status status;
    std::string_view allowed_method(AllowedMethods::STATE);

    // Полуаем токен авторизации
    auto token = GetTokenFromRequestStr(req[http::field::authorization]);
    if (!token.empty()) {
        // Получаем список игроков в виде response_body для игрока с токеном token
        try {
            status = GetState(token, response_body);
        } catch (app::GetStateError err) {
            if(err.reason_ == app::GetStateErrorReason::InvalidToken) {
                response_body = GenerateErrorResponse(json_field::API_CODE_UNKNOWN_TOKEN, "Player token has not been found");
                status = http::status::unauthorized;
            }
        } catch (...) {
            response_body = GenerateErrorResponse(json_field::API_CODE_UNKNOWN_TOKEN, "Unknown error");
            status = http::status::bad_request;
        }
    } else {
        status = http::status::unauthorized;
        response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_TOKEN, "Authorization header is missing"s);
    }

    // Сначала хотелось выделить следующий блок в отдельную функцию, но оказалось, что к разным веткам АПИ
    // допустимы разные методы запросов. Поэтому обрабатывать метод нужно в каждой ветке отдельно
    int response_size = 0;
    auto req_method = req.method();
    if (req_method == http::verb::get) {
        response_size = response_body.size();
    } else if (req_method == http::verb::head) {
        response_size = response_body.size();   // Запоминаем размер
        response_body = ""s;    // Зачищаем тело запроса
    } else {    // Недопустимый метод
        status = http::status::method_not_allowed;
        // Сгенерировать JSON с ошибкой
        response_body = GenerateErrorResponse(json_field::API_CODE_INVALID_METHOD, "Invalid method"s);
        response_size = response_body.size();   // Запоминаем размер
    }

    auto response = this->MakeStringResponse(status, response_body, response_size, req.version(), req.keep_alive(), content_type, allowed_method);
    response.set(http::field::cache_control, HttpFildsValue::NO_CACHE);
    return response;
}

http::status ApiHandler::GetState(std::string_view token, std::string& response_body) {
    app::GetStateResult res = app_.GetState(token);
    response_body = boost::json::serialize(json::value_from(res));
    return http::status::ok;
}

// Создаёт StringResponse с заданными параметрами
StringResponse ApiHandler::MakeStringResponse(http::status status, std::string_view body, size_t size, unsigned http_version,
                                  bool keep_alive, std::string_view content_type, std::string_view allowed_method) const {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    if (status == http::status::method_not_allowed) {
        response.set(http::field::allow, allowed_method);
    }
    response.body() = body;
    response.content_length(size);
    response.keep_alive(keep_alive);
    return response;
}

std::string ApiHandler::GenerateErrorResponse(const std::string& code, const std::string& msg) const {
    ResponseError response{code,msg};
    return json::serialize(json::value_from(response));
}

http::status ApiHandler::GetMaps(std::string& response, const std::vector<std::string>& segments) const {
    // В данный момент доступна только версия v1 и только карты
    if (segments.size() == api_strings::ACTION_POS) {   // Запросили только список карт
        response = serialize(json::value_from( app_.ListMaps() ));
        return http::status::ok;
    } else {    // Запрос конкретной карты
        return GetMap(response,segments);
    }
}

http::status ApiHandler::GetMap(std::string& response, const std::vector<std::string>& segments) const {
    auto map_id = segments[api_strings::ACTION_POS];
    // ищем карту
    for (const auto& map : app_.ListMaps()) {
        if (*map.GetId() == map_id) {   // Нашли карту
            response = serialize(json::value_from( map ));
            return http::status::ok;
        }
    }
    // обошли весь список, но карту не нашли
    response = GenerateErrorResponse(json_field::API_CODE_MAP_NOT_FOUND, "Map not found"s);
    return http::status::not_found;
}

}  // namespace http_handler
