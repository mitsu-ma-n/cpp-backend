#include "json_loader.h"
#include "model.h"

#include <boost/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <string>

namespace json = boost::json;
using namespace std::literals;

using namespace model;

namespace json_loader {

void print(boost::property_tree::ptree const& pt)
{
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print(it->second);
    }
}

auto GetArrayFromValue(json::value& val, std::string key) {
    //std::cout << key << " : "sv << val.at(key) << std::endl;
    return val.as_object().at(key).as_array();
}

auto GetStringFromValue(json::value& val, std::string key) {
    //std::cout << key << " : "sv << val.at(key) << std::endl;
    return val.as_object().at(key).as_string();
}

auto GetIntFromValue(json::value& val, std::string key) {
    //std::cout << key << " : "sv << val.at(key) << std::endl;
    return val.as_object().at(key).as_int64();
}

Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path в виде строки
    std::ifstream ifs(json_path);
    std::string input(std::istreambuf_iterator<char>(ifs), {});
    
    // Создаём пустую игру
    Game game;

    // Распарсить строку как JSON, используя boost::json::parse
    // Получаем json-объект из строки (тип value)
    auto parsed_config_json = json::parse(input);

    // Получили список карт
    auto maps_json = GetArrayFromValue(parsed_config_json, "maps"s);

    // Цикл по картам
    for (auto map_json : maps_json) {
        // Достаём id
        std::string map_id_str(GetStringFromValue(map_json, "id"s));
        Map::Id map_id(std::move(map_id_str));
        // Достаём name
        std::string  map_name(GetStringFromValue(map_json, "name"s));

        // Конструируем карту
        Map model_map{map_id,map_name};

        // Достаём roads
        auto roads_json = GetArrayFromValue(map_json, "roads"s);
        for (auto road_json : roads_json) {
            // Достаём координаты 
            Coord x0(GetIntFromValue(road_json, "x0"s));
            Coord y0(GetIntFromValue(road_json, "y0"s));

            // Добавялем дорогу на карту
            if (road_json.as_object().contains("x1"s)) {
                Coord x1(GetIntFromValue(road_json, "x1"s));
                Road model_road{Road::HORIZONTAL,Point{x0,y0},x1};
                model_map.AddRoad(model_road);
            } else {
                Coord y1(GetIntFromValue(road_json, "y1"s));
                Road model_road{Road::VERTICAL,Point{x0,y0},y1};
                model_map.AddRoad(model_road);
            }
        }

        // Достаём buildings
        auto buildings_json = GetArrayFromValue(map_json, "buildings"s);
        for (auto building_json : buildings_json) {
            // Достаём координаты 
            Coord x(GetIntFromValue(building_json, "x"s));
            Coord y(GetIntFromValue(building_json, "y"s));
            // Достаём размеры 
            Coord w(GetIntFromValue(building_json, "w"s));
            Coord h(GetIntFromValue(building_json, "h"s));

            // Добавляем здание на карту
            Building model_building({x,y,w,h});
            model_map.AddBuilding(model_building);
        }

        // Достаём buildings
        auto offices_json = GetArrayFromValue(map_json, "offices"s);
        for (auto office_json : offices_json) {
            // Достаём id
            std::string office_id_str(GetStringFromValue(office_json, "id"s));
            Office::Id office_id(std::move(office_id_str));

            // Достаём координаты 
            Coord x(GetIntFromValue(office_json, "x"s));
            Coord y(GetIntFromValue(office_json, "y"s));
            // Достаём размеры 
            Dimension offsetX(GetIntFromValue(office_json, "offsetX"s));
            Dimension offsetY(GetIntFromValue(office_json, "offsetY"s));

            // Добавляем офис на карту
            Office model_office{office_id,{x,y},{offsetX,offsetY}};
            model_map.AddOffice(model_office);
        }

        // Добавляем заполненную карту в игру
        game.AddMap(model_map);
    }
   
    return game;
}

}  // namespace json_loader
