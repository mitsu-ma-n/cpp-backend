#include "request_handler.h"
#include "boost/beast/http/status.hpp"
#include "boost/json/serialize.hpp"
#include "model.h"

#include <iostream>
#include <string_view>

namespace model {

using namespace std::literals;

// сериализация экземпляра класса Office в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Office const& office)
{
    auto id = *office.GetId();
    auto pos = office.GetPosition();
    auto offset = office.GetOffset();
    jv = {
        {"id", id},
        {"x", pos.x},
        {"y", pos.y},
        {"offsetX", offset.dx},
        {"offsetY", offset.dy}
    };
}

// сериализация экземпляра класса Building в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Building const& building)
{
    auto bounds = building.GetBounds();
    jv = {
        {"x", bounds.position.x},
        {"y", bounds.position.y},
        {"w", bounds.size.width},
        {"h", bounds.size.height}
    };
}

// сериализация экземпляра класса Road в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Road const& road)
{
    auto start = road.GetStart();
    auto end = road.GetEnd();
    if (end.x == start.x) { // VERTICAL
        jv = {
            {"x0", start.x},
            {"y0", start.y},
            {"y1", end.y}
        };
    } else {    // HORIZONTAL
        jv = {
            {"x0", start.x},
            {"y0", start.y},
            {"x1", end.x}
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
    object["id"] = *map.GetId();
    object["name"] = map.GetName();

    // Теперь нужно писать массивы
    object["roads"] = form_array(map.GetRoads());
    object["buildings"] = form_array(map.GetBuildings());;
    object["offices"] = form_array(map.GetOffices());
    jv.emplace_object() = object;
}

// сериализация экземпляра класса Map в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, std::vector<Map> const& maps)
{
    auto form_array = [](auto container){
        json::array arr;
        for (const auto& item : container) {
            json::object object;
            // Записываем сводную информацию
            object["id"] = *item.GetId();
            object["name"] = item.GetName();

            arr.push_back(object);
        }
        return arr;
    };

    jv = form_array(maps);
}


Office tag_invoke(json::value_to_tag<Office>, json::value const& jv )
{
    json::object const& obj = jv.as_object();
    std::string id_str (obj.at("id"s).as_string());
    Office::Id id(std::move(id_str));

    int x = obj.at("x"s).as_int64();
    int y = obj.at("y"s).as_int64();
    int offsetX = obj.at("offsetX"s).as_int64();
    int offsetY = obj.at("offsetY"s).as_int64();

    Office office{id,{x,y},{offsetX,offsetY}};
    return office;
}

} // namespace model

namespace http_handler {

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

StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
    //std::string req_view(req.method_string());
    //std::cout << "RequestHandler::HandleRequest" << req_view << std::endl;
    const auto text_response = [&req,this](http::status status, std::string_view text, size_t size, std::string_view content_type) {
        return this->MakeStringResponse(status, text, size, req.version(), req.keep_alive(), content_type);
    };

    auto req_method = req.method();
    // Определяем цель запроса
    std::string_view req_target = req.target();
    std::string target(req_target.begin()+1,req_target.end());

    std::string_view content_type(ContentType::TEXT_HTML);
    auto status = http::status::ok;
    std::string response_body("");

    try {
        std::string api_path("/api/");
        std::string maps_path("/api/v1/maps");
        std::string maps_id_path("/api/v1/maps/");

        if (!req_target.starts_with(api_path)) {
            // Если пошли не в /api/, сразу пишем, что страница не найдена
            std::string err_body = "Page Not Found!"s;
            throw ExeptionInfo{http::status::not_found, err_body};
        } 
        
        content_type = ContentType::APP_JSON;
        if (!req_target.starts_with(maps_path)) {
            // Можем отдавать только карты. Если попросили не карты:
            std::string err_body = R"({
"code": "badRequest",
"message": "Bad request to API"
})"s;
            throw ExeptionInfo{http::status::bad_request, err_body};
        }

        auto maps = game_.GetMaps();

        // Попросили карты - отдаём список
        if (req_target == maps_path) {
            std::string err_body(serialize(json::value_from( maps )));
            throw ExeptionInfo{http::status::ok, err_body};
        }

        auto map_id = req_target.substr(maps_path.size()+1);
        auto map_delimiter = req_target.substr(maps_path.size(),1);
        if (!map_delimiter.empty() && map_delimiter == "/"s) {
            // Попросили конкретную карту - ищем карту
            for (const auto& map : maps) {
                if (*map.GetId() == map_id) {
                    std::string err_body(serialize(json::value_from( map )));
                    throw ExeptionInfo{http::status::ok, err_body};
                }
            }
            // Всё прошли и не нашли - возвращаем ошибку
            std::string err_body(R"({
"code": "mapNotFound",
"message": "Map not found"
})"sv);
            throw ExeptionInfo{http::status::not_found, err_body};
        }
        else {
            // Можем отдавать только карты. Если попросили не карты:
            std::string err_body = R"({
"code": "badRequest",
"message": "Bad request to API"
})"s;
            throw ExeptionInfo{http::status::bad_request, err_body};
        }

    } catch (const ExeptionInfo& ex) {
        std::cout << "Status: " << ex.status << " Body: " << ex.body << std::endl;
        status = ex.status;
        response_body = ex.body;
    }

    // возможные варианты:
    // status: ok, 400, 404
    // body: maps, map, error
    // status - body
    // ok - maps
    // ok - map
    // 400 - error(badRequest)
    // 404 - error(mapNotFound)

    // Проверяем цель на корректность
    // проверили
    // Кидаем исключение?

    if (req_method == http::verb::get) {
        return text_response(status, response_body, response_body.size(), content_type);
    } else if (req_method == http::verb::head) {
        return text_response(http::status::ok, ""sv, response_body.size(), content_type);
    } else {
        std::string response_not_allowed_body("Invalid method"sv);
        return text_response(http::status::method_not_allowed, response_not_allowed_body, response_not_allowed_body.size(), content_type);
    }
}

}  // namespace http_handler
