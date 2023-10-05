#include "request_handler.h"

#include "boost/beast/http/status.hpp"
#include <boost/json.hpp>
#include "boost/json/serialize.hpp"
#include <boost/url.hpp>
#include <iostream>
#include <string_view>

#include "boost/json/value_from.hpp"
#include "model.h"
#include "json_fields.h"
#include "api_strings.h"

namespace json = boost::json;
namespace urls = boost::urls;
namespace core = boost::core;

// tag_invoke должны быть определны в том же namespace, в котором определны классы,
// которые ими обрабатываются. В наше случае это model
namespace model {
// сериализация экземпляра класса Office в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Office const& office)
{
    auto id = *office.GetId();
    auto pos = office.GetPosition();
    auto offset = office.GetOffset();
    jv = {
        {json_field::OFFICE_ID, id},
        {json_field::OFFICE_POS_X, pos.x},
        {json_field::OFFICE_POS_Y, pos.y},
        {json_field::OFFICE_OFFSET_DX, offset.dx},
        {json_field::OFFICE_OFFSET_DY, offset.dy}
    };
}

// сериализация экземпляра класса Building в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Building const& building)
{
    auto bounds = building.GetBounds();
    jv = {
        {json_field::BUILDING_POS_X, bounds.position.x},
        {json_field::BUILDING_POS_Y, bounds.position.y},
        {json_field::BUILDING_SIZE_W, bounds.size.width},
        {json_field::BUILDING_SIZE_H, bounds.size.height}
    };
}

// сериализация экземпляра класса Road в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Road const& road)
{
    auto start = road.GetStart();
    auto end = road.GetEnd();
    if (end.x == start.x) { // VERTICAL
        jv = {
            {json_field::ROAD_START_X, start.x},
            {json_field::ROAD_START_Y, start.y},
            {json_field::ROAD_END_Y, end.y}
        };
    } else {    // HORIZONTAL
        jv = {
            {json_field::ROAD_START_X, start.x},
            {json_field::ROAD_START_Y, start.y},
            {json_field::ROAD_END_X, end.x}
        };
    }
}

// сериализация экземпляра класса Map в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Map const& map)
{
    auto form_array = [](auto container){
        json::array arr;
        for (const auto& item : container) {
            arr.push_back(json::value_from(item));
        }
        return arr;
    };

    json::object object;
    // Записываем сводную информацию
    object[json_field::MAP_ID] = *map.GetId();
    object[json_field::MAP_NAME] = map.GetName();

    // Теперь нужно писать массивы
    object[json_field::MAP_ROADS] = form_array(map.GetRoads());
    object[json_field::MAP_BUILDINGS] = form_array(map.GetBuildings());;
    object[json_field::MAP_OFFICES] = form_array(map.GetOffices());
    jv.emplace_object() = object;
}

// сериализация массива std::vector<Map> в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, std::vector<Map> const& maps)
{
    auto form_array = [](auto container){
        json::array arr;
        for (const auto& item : container) {
            json::object object;
            // Записываем сводную информацию
            object[json_field::MAP_ID] = *item.GetId();
            object[json_field::MAP_NAME] = item.GetName();

            arr.push_back(object);
        }
        return arr;
    };

    jv = form_array(maps);
}
} // namespace model


namespace http_handler {

// сериализация экземпляра класса ResponseError в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, http_handler::ResponseError const& resp_err)
{
    jv = {
        {json_field::ERROR_CODE, resp_err.code},
        {json_field::ERROR_MESSAGE, resp_err.message}
    };
}

// Создаёт StringResponse с заданными параметрами
StringResponse RequestHandler::MakeStringResponse(http::status status, std::string_view body, size_t size, unsigned http_version,
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

std::string GenerateErrorResponse(const std::string& code, const std::string& msg) {
    ResponseError response{code,msg};
    return json::serialize(json::value_from(response));
}

// Функция представляет путь вида "/my/nice/path" как массив строк ["my", "nice", "path"]
std::vector<std::string> GetSegmentsFromPath( core::string_view s )
{
    urls::url_view u ( s );
    std::vector< std::string > seq;
    for( auto seg : u.segments() )
        seq.push_back( seg );
    return seq;
}

// Функция генерит ответ response на запрос к API. Запрос представлен в виде массива "папок" segments.
// Возвращает статус, который соответствует сгенерированному ответу.
http::status RequestHandler::GetApiResponse(std::string& response, const std::vector<std::string>& segments) {
    // В данный момент доступна только версия v1 и только карты
    if (segments[api_strings::VERSION_POS] == api_strings::VERSION_PATH && 
        segments[api_strings::MAPS_POS] == api_strings::MAPS_PATH) 
    {   // Запросили карты
        return GetMaps(response,segments);
    } else {    // Неизвестный запрос
        response = GenerateErrorResponse("badRequest", "Bad Request");
        return http::status::bad_request;
    }
}

http::status RequestHandler::GetMaps(std::string& response, const std::vector<std::string>& segments) {
    // В данный момент доступна только версия v1 и только карты
    if (segments.size() == api_strings::MAP_ID_POS) {   // Запросили только список карт
        response = serialize(json::value_from( game_.GetMaps() ));
        return http::status::ok;
    } else {    // Запрос конкретной карты
        return GetMap(response,segments);
    }
}

http::status RequestHandler::GetMap(std::string& response, const std::vector<std::string>& segments) {
    auto map_id = segments[api_strings::MAP_ID_POS];
    // ищем карту
    for (const auto& map : game_.GetMaps()) {
        if (*map.GetId() == map_id) {   // Нашли карту
            response = serialize(json::value_from( map ));
            return http::status::ok;
        }
    }
    // обошли весь список, но карту не нашли
    response = GenerateErrorResponse("mapNotFound"s, "Map not found"s);
    return http::status::not_found;
}


StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
    const auto text_response = [&req,this](http::status status, std::string_view text, size_t size, std::string_view content_type) {
        return this->MakeStringResponse(status, text, size, req.version(), req.keep_alive(), content_type);
    };

    auto req_method = req.method();
    // Определяем цель запроса
    std::string req_target(req.target());
    // Удаляем завершающий слэш, чтобы не мешал разбору
    while (!req_target.empty() && req_target.back() == '/') {
        req_target.pop_back();
    }

    // Представление пути в виде массива с именами
    auto segments = GetSegmentsFromPath(std::string_view(req_target));

    http::status status;
    std::string content_type;
    std::string response_body;

    if(segments.empty()) {  // В запросе пусто
        // Формируем текстовый ответ "Страница не найдена"
        content_type = ContentType::TEXT_HTML;
        status = http::status::not_found;
        response_body = "Page not found!"s;
    } else {
        if (std::string_view(segments[api_strings::MAIN_POS]) == api_strings::MAIN_PATH) {    // Запрос к API
            content_type = ContentType::APP_JSON;   // Из API отвечаем JSON-ом
            status = GetApiResponse(response_body, segments);
        } else {    // Неизвестный запрос
            // Формируем текстовый ответ "Страница не найдена"
            content_type = ContentType::TEXT_HTML;
            status = http::status::not_found;
            response_body = "Page not found!"s;
        }
    }

    int response_size = 0;
    if (req_method == http::verb::get) {
        response_size = response_body.size();
    } else if (req_method == http::verb::head) {
        response_size = response_body.size();   // Запоминаем размер
        response_body = ""s;    // Зачищаем тело запроса
    } else {    // Недопустимый метод
        status = http::status::method_not_allowed;
        response_body = "Invalid method"s;
        response_size = response_body.size();   // Запоминаем размер
    }
    return text_response(status, response_body, response_size, content_type);
}

}  // namespace http_handler
