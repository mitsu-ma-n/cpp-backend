#pragma once

#include "model.h"

#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <string>
#include <unordered_map>

namespace extra_data {

class LootGeneratorConfig {
public:
    LootGeneratorConfig(double period, double probability)
        : period_{period}
        , probability_{probability} {
    }

    double GetPeriod() const {
        return period_;
    }

    double GetProbability() const {
        return probability_;
    }

private:
    double period_;
    double probability_;
};

/*
class LootType {
public:
    LootType(std::string name, std::string file, std::string type, int rotation, std::string color, double scale)
        : name_{std::move(name)}
        , file_{std::move(file)}
        , type_{std::move(type)}
        , rotation_{rotation}
        , color_{std::move(color)}
        , scale_{scale} {
    }

    // void SetMap(model::Map& map) {
    //     this->map_ = &map;
    // }

    // const model::Map& GetMap() const {
    //     return *map_;
    // }


public:
    std::string name_;
    std::string file_;
    std::string type_;
    int rotation_;
    std::string color_;
    double scale_;

private:
    // Ссылка на информацию о карте, к которой относится информация
    model::Map* map_ = nullptr;
};
*/

class ExtendedMap {
public:
    //using LootType = boost::json::value;
    //using LootTypes = std::vector<LootType>;
    using LootTypes = boost::json::array;
    ExtendedMap(model::Map map, LootTypes loot_types)
        : map_{std::move(map)}
        , loot_types_(std::move(loot_types)) {
    }

    const model::Map& GetMap() const {
        return map_;
    }

    model::Map* GetMapPointer() {
        return &map_;
    }

    const LootTypes& GetLootTypes() const {
        return loot_types_;
    }

    // const std::vector<LootType>& GetLootTypes() const {
    //     return loot_types_;
    // }

    // void AddLootType(LootType loot_type) {
    //     try {
    //         loot_types_.push_back(std::move(loot_type));
    //     } catch (const std::exception& e) {
    //         throw std::runtime_error("Failed to add loot type: " + std::string(e.what()));
    //     }
    // }

private:
    model::Map map_;
    LootTypes loot_types_;
};

using MapIdHasher = util::TaggedHasher<model::Map::Id>;
using MapsLootTypes = std::unordered_map<model::Map::Id, ExtendedMap::LootTypes, MapIdHasher>;

}  // namespace extra_data
