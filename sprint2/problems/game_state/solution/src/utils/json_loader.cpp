#include "json_loader.h"

#include <boost/json.hpp>
#include "api_handler.h"
#include "boost/json/value_to.hpp"
// Позволяет загрузить содержимое файла в виде строки:
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <string_view>

#include "json_fields.h"
#include "model.h"

namespace json = boost::json;
using namespace std::literals;

using namespace model;

// tag_invoke должны быть определны в том же namespace, в котором определны классы,
// которые ими обрабатываются. В наше случае это model
namespace model {
Building tag_invoke(json::value_to_tag<Building>, json::value const& jv)
{
    json::object const& obj = jv.as_object();
    return Building ({
        value_to<Coord>(obj.at(std::string(json_field::BUILDING_POS_X))),
        value_to<Coord>(obj.at(std::string(json_field::BUILDING_POS_Y))),
        value_to<Dimension>(obj.at(std::string(json_field::BUILDING_SIZE_W))),
        value_to<Dimension>(obj.at(std::string(json_field::BUILDING_SIZE_H)))
    });
}

Road tag_invoke(json::value_to_tag<Road>, json::value const& jv)
{
    json::object const& obj = jv.as_object();

    Coord x0(value_to<Coord>(obj.at(std::string(json_field::ROAD_START_X))));
    Coord y0(value_to<Coord>(obj.at(std::string(json_field::ROAD_START_Y))));

    if (obj.contains(std::string(json_field::ROAD_END_X))) {
        Coord end(value_to<Coord>(obj.at(std::string(json_field::ROAD_END_X))));
        return Road{Road::HORIZONTAL, {x0, y0}, end};
    } else {
        Coord end(value_to<Coord>(obj.at(std::string(json_field::ROAD_END_Y))));
        return Road{Road::VERTICAL, {x0, y0}, end};
    }
}

Office tag_invoke(json::value_to_tag<Office>, json::value const& jv)
{
    json::object const& obj = jv.as_object();
    return Office {
        Office::Id(value_to<std::string>(obj.at(std::string(json_field::OFFICE_ID)))),
        {
            value_to<Coord>(obj.at(std::string(json_field::OFFICE_POS_X))),
            value_to<Coord>(obj.at(std::string(json_field::OFFICE_POS_Y)))
        },
        {
            value_to<Dimension>(obj.at(std::string(json_field::OFFICE_OFFSET_DX))),
            value_to<Dimension>(obj.at(std::string(json_field::OFFICE_OFFSET_DY)))
        }
    };
}

void AddRoads(Map& map, json::object const& obj) {
    std::vector<Road> roads = value_to<std::vector<Road>>(obj.at(std::string(json_field::MAP_ROADS)));
    for (const auto& road : roads) {
        map.AddRoad(road);
    }
}

void AddBuildings(Map& map, json::object const& obj) {
    std::vector<Building> buildings = value_to<std::vector<Building>>(obj.at(std::string(json_field::MAP_BUILDINGS)));
    for (const auto& building : buildings) {
        map.AddBuilding(building);
    }
}

void AddOffices(Map& map, json::object const& obj) {
    // Добавляем офисы
    std::vector<Office> offices = value_to<std::vector<Office>>(obj.at(std::string(json_field::MAP_OFFICES)));
    for (const auto& office : offices) {
        map.AddOffice(office);
    }
}

Map tag_invoke(json::value_to_tag<Map>, json::value const& jv )
{
    json::object const& obj = jv.as_object();

    // Конструируем карту
    Map map{
        Map::Id(value_to<std::string>(obj.at(std::string(json_field::MAP_ID)))),
        value_to<std::string>(obj.at(std::string(json_field::MAP_NAME)))
    };

    if( auto it = obj.find(json_field::MAP_DOG_SPEED); it != obj.end() ) {
        map.dogSpeed_ = value_to<double>(it->value());
    }

    AddRoads(map,obj);
    AddBuildings(map,obj);
    AddOffices(map,obj);
    
    return map;
}

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

namespace app {

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::JoinGameResult const& join_result) {
    json::object object;
    object[json_field::JOIN_TOKEN] = join_result.GetTokenAsString();
    object[json_field::JOIN_PLAYER_ID] = join_result.GetPlayerId();
    jv.emplace_object() = object;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::PlayerInfo const& player) {
    json::object object_name;
    object_name[json_field::LIST_PLAYERS_NAME] = player.GetNameAsString();
    // Записываем сводную информацию
    json::object object;
    object[player.GetIdAsString().c_str()] = object_name;
    jv.emplace_object() = object;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::ListPlayersResult const& players_result) {
    json::object object;

    for (const auto& player_info : players_result) {
        json::object object_name;
        object_name[json_field::LIST_PLAYERS_NAME] = player_info.GetNameAsString();
        // Записываем сводную информацию
        object[player_info.GetIdAsString().c_str()] = object_name;
    }

    jv.emplace_object() = object;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::GetStateResult const& state_result) {
    json::object object_players;    // Сводная информация об игроках свойство-объект

    json::object object_players_info;   // Объект информации об игроках
    for (const auto& player_info : state_result) {
        object_players_info[player_info.GetIdAsString()] = player_info.GetIdAsString();
    }

    object_players[json_field::GET_STATE_PLAYERS] = object_players_info;

    jv.emplace_object() = object_players;
}

} // namespace app


namespace http_handler {

JoinParams tag_invoke(json::value_to_tag<JoinParams>, json::value const& jv)
{
    json::object const& obj = jv.as_object();
    return JoinParams ({
        value_to<std::string>(obj.at(std::string(json_field::JOIN_NAME))),
        value_to<std::string>(obj.at(std::string(json_field::JOIN_MAP_ID)))
    });
}

}

namespace json_loader {

Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path в виде строки
    std::ifstream ifs(json_path);
    std::string input(std::istreambuf_iterator<char>(ifs), {});
    
    // Создаём пустую игру
    Game game;

    // Распарсить строку как JSON, используя boost::json::parse
    // Получаем json-объект из строки (тип value)
    auto parsed_config_json = json::parse(input);
    auto obj = parsed_config_json.as_object();

    game.defuaultDogSpeed_ = value_to<double>(obj.at(std::string(json_field::GAME_DEFAULT_DOG_SPEED)));

    // Добавляем карты в игру
    auto maps = value_to<std::vector<Map>>(obj.at(std::string(json_field::GAME_MAPS)));
    for (auto map : maps) {
        game.AddMap(map);
    }
   
    return game;
}

bool ReadJoinParamsFromString(http_handler::JoinParams& params, std::string str) {
    // Распарсить строку как JSON, используя boost::json::parse
    // Получаем json-объект из строки (тип value)
    auto parsed_config_json = json::parse(str);
    auto obj = parsed_config_json.as_object();

    params.name   = value_to<std::string>(obj.at(std::string(json_field::JOIN_NAME)));
    params.map_id = value_to<std::string>(obj.at(std::string(json_field::JOIN_MAP_ID)));

    return true;
}


}  // namespace json_loader
