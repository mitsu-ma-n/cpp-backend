#pragma once

#include "game_session.h"
#include "loot_generator.h"

namespace model {

class Game {
public:
    using Maps = std::vector<Map>;
    using GameSessions = std::vector<GameSession*>;

    Game(loot_gen::LootGeneratorInfo info, DynamicDimension dogSpeed = 1.0, int bag_capacity = 3) noexcept
        : loot_generator_(info.GetPeriodInMilliseconds(), info.GetProbability())
        , defuault_dog_speed_{dogSpeed}
        , default_bag_capacity_{bag_capacity} {
    }

    ~Game() {
        for (auto session : sessions_) {
            delete session;
        }
    }

    void AddMap(const Map& map);
    GameSession* CreateSession(const Map::Id& map_id);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const GameSessions& GetSessions() const noexcept {
        return sessions_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept;
    GameSession* FindSession(const Map::Id& id);

    const DynamicDimension GetDefaultDogSpeed() const noexcept {
        return defuault_dog_speed_;
    }

    void SetDefaultDogSpeed(DynamicDimension dog_speed) noexcept {
        defuault_dog_speed_ = dog_speed;
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    DynamicDimension defuault_dog_speed_;
    int default_bag_capacity_;

    Maps maps_;
    MapIdToIndex map_id_to_map_index_;

    GameSessions sessions_;
    MapIdToIndex map_id_to_session_index_;

    loot_gen::LootGenerator loot_generator_;
};

}  // namespace model
