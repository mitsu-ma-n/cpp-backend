#include "json_loader.h"

#include <boost/json.hpp>
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

    AddRoads(map,obj);
    AddBuildings(map,obj);
    AddOffices(map,obj);
    
    return map;
}

} // namespace model


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

    // Добавляем карты в игру
    auto maps = value_to<std::vector<Map>>(obj.at(std::string(json_field::GAME_MAPS)));
    for (auto map : maps) {
        game.AddMap(map);
    }
   
    return game;
}

}  // namespace json_loader
