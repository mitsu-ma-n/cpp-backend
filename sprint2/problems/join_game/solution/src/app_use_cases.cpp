#include "app_use_cases.h"

namespace app {

///  ---  JoinGameUseCase  ---  ///

JoinGameUseCase::JoinGameUseCase(model::Game& game, PlayerTokens& player_tokens, Players& players) 
    : game_{&game}
    , player_tokens_{&player_tokens}
    , players_{&players} {
}

// Проверка имени на правильность. Пока никаких требований к имени нет
bool isValidName(Player::Name name) {
    return true;
}

// Получение произвольной точки на дорогах карты
model::Point GetRandomPointOnMap() {
    return model::Point{0,0};
}

// Подключает игрока с указанным именем (пса) к указанной карте
JoinGameResult JoinGameUseCase::JoinGame(model::Map::Id map_id, Player::Name name) {
    std::string name_str(name);
    if ( !isValidName(name) ) {
        throw JoinGameError{JoinGameErrorReason::InvalidName};
    }
    
    if ( auto* session = game_->FindSession(model::Map::Id{map_id}) ) {
        auto spawn_point = GetRandomPointOnMap();
        auto dog = session->AddDog(spawn_point, std::move(name_str));
        auto& player = players_->Add(dog, *session);
        auto token = player_tokens_->AddPlayer(player);
        return {token, player.GetId()};
    }
    throw JoinGameError{JoinGameErrorReason::InvalidMap};
}

///  ---  Application  ---  ///

model::Game::Maps Application::ListMaps() {
    return list_maps_.GetMaps();
}

const model::Map& Application::FindMap(std::string map_id) {
    return get_map_.GetMap(model::Map::Id(map_id));
}

JoinGameResult Application::JoinGame(std::string user_name, std::string map_id) {
    return join_game_.JoinGame(model::Map::Id(map_id), user_name);
}


}  // namespace app
