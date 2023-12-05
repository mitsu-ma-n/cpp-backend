#include "join_use_case.h"
#include <cstddef>
#include <stdexcept>

namespace app {

// Проверка имени на правильность
bool isValidName(Player::Name name) {
    // На данный момент проверяем только пустоту строки
    return !name->empty();
}

// Получение произвольной точки на дорогах карты
model::Position GetRandomPointOnMap(const model::Map& map) {
    auto roads = map.GetRoads();
    // Выбираем случайную дорогу
//    size_t road_index = utils::my_random::GetRandomIndex(0, roads.size()-1);
// Временно для тестрования берём начальную точку первой дороги
    size_t road_index = 0;
    model::Road& road = roads[road_index];

    // Выбираем случайную точку на дороге
    model::Point start = road.GetStart();
    model::Point end = road.GetEnd();

    model::Position res;
    res.x = start.x;
    res.y = start.y;
/*
    if ( road.IsHorizontal() ) {
        res.y = start.y;
        res.x = utils::my_random::GetRandomDouble(start.x, end.x);
    } else {
        res.x = start.x;
        res.y = utils::my_random::GetRandomDouble(start.y, end.y);
    }
*/
    return res;
}

JoinGameUseCase::JoinGameUseCase(model::Game& game, PlayerTokens& player_tokens, Players& players) 
    : game_{&game}
    , player_tokens_{&player_tokens}
    , players_{&players} {
}

// Подключает игрока с указанным именем (пса) к указанной карте
JoinGameResult JoinGameUseCase::JoinGame(model::Map::Id map_id, Player::Name name) {
    model::Dog::Name name_str(*name);
    if ( !isValidName(name) ) {
        throw JoinGameError{JoinGameErrorReason::InvalidName};
    }

    auto map = game_->FindMap(map_id);
    if ( !map ) {
        throw JoinGameError{JoinGameErrorReason::InvalidMap};
    }
    
    if ( auto session = game_->FindSession(model::Map::Id{map_id}) ) {
        auto spawn_point = GetRandomPointOnMap(*map);
        try {
            auto dog = session->AddDog(spawn_point, std::move(name_str));
            auto& player = players_->Add(dog, *session);
            auto token = player_tokens_->AddPlayer(player);
            return {token, player.GetId()};
        }
        catch ( std::invalid_argument err ) {
            throw JoinGameError{JoinGameErrorReason::InvalidName};
        }
    }
    throw JoinGameError{JoinGameErrorReason::InvalidMap};
}


}  // namespace app
