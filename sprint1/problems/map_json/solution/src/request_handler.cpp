#include "request_handler.h"
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

// сериализация экземпляра класса Building в JSON-значение
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

// сериализация экземпляра класса Building в JSON-значение
void tag_invoke(json::value_from_tag, json::value& jv, Map const& map)
{
    json::object object;
    // Записываем сводную информацию
    object["id"] = *map.GetId();
    object["name"] = map.GetName();

    // Теперь нужно писать массивы
    // Создаём массив дорог
    json::array roads;
    for (const auto& road : map.GetRoads()) {
        roads.push_back(json::value_from(road));
    }
    object["roads"] = roads;

    // Создаём массив зданий
    json::array buildings;
    for (const auto& building : map.GetBuildings()) {
        buildings.push_back(json::value_from(building));
    }
    object["buildings"] = buildings;

    // Создаём массив офисов
    json::array offices;
    for (const auto& office : map.GetOffices()) {
        offices.push_back(json::value_from(office));
    }
    object["offices"] = offices;
    jv.emplace_object() = object;
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
    const auto text_response = [&req,this](http::status status, std::string_view text, size_t size) {
        return this->MakeStringResponse(status, text, size, req.version(), req.keep_alive());
    };



/*
    std::string res("[");
    auto& maps = game_.GetMaps();
    for (auto& map : maps) {
        res += R"({"id)"s + map.GetId() + ": "
        auto id = map.GetId();
        auto name = map.GetName();
        std::string map_info

    }
*/

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

}  // namespace http_handler
