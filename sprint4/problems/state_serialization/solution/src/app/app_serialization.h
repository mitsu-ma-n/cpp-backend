#pragma once

#include <boost/serialization/vector.hpp>
#include <vector>

#include "players.h"
#include "game.h"

namespace serialization {

// PlayersRepr (PlayersRepresentation) - сериализованное представление класса Players
class PlayersRepr {
public:
    PlayersRepr() = default;

    explicit PlayersRepr(const app::Players& players) {
        for (const auto& player : players.GetPlayers()) {
            players_.push_back(*player);
        }
    }

private:
    std::vector<app::Player> players_;
    std::vector<model::Dog::Name> names_;
    std::vector<size_t> indexes_;
};

class StateSerializer {
public:
    StateSerializer(model::Game& game, app::Players& players, app::PlayerTokens& tokens)
    : game_(&game)
    , players_(&players)
    , tokens_(&tokens) 
    {}

public:
    model::Game* game_;
    app::Players* players_;
    app::PlayerTokens* tokens_;
};

}  // namespace serialization
