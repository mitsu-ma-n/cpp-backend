#include "app_use_cases.h"
#include "app.h"
#include <stdexcept>

namespace app {

///  ---  JoinGameUseCase  ---  ///

JoinGameUseCase::JoinGameUseCase(model::Game& game, PlayerTokens& player_tokens, Players& players) 
    : game_{&game}
    , player_tokens_{&player_tokens}
    , players_{&players} {
}

// Проверка имени на правильность
bool isValidName(Player::Name name) {
    // На данный момент проверяем только пустоту строки
    return !(*name).empty();
}

// Получение произвольной точки на дорогах карты
model::Point GetRandomPointOnMap() {
    return model::Point{0,0};
}

// Подключает игрока с указанным именем (пса) к указанной карте
JoinGameResult JoinGameUseCase::JoinGame(model::Map::Id map_id, Player::Name name) {
    model::Dog::Name name_str(*name);
    if ( !isValidName(name) ) {
        throw JoinGameError{JoinGameErrorReason::InvalidName};
    }

    if ( !game_->FindMap(map_id) ) {
        throw JoinGameError{JoinGameErrorReason::InvalidMap};
    }
    
    if ( auto* session = game_->FindSession(model::Map::Id{map_id}) ) {
        auto spawn_point = GetRandomPointOnMap();
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

// Подключает игрока с указанным именем (пса) к указанной карте
ListPlayersResult ListPlayersUseCase::GetPlayers(Token token) {
    ListPlayersResult res;
    // Получаем игрока с заданным токеном
    if ( auto self_player = player_tokens_->FindPlayerByToken(token) ) {
        // Получаем сессию, к которой подключен игрок и список собак в сессии
        auto session = self_player->GetSession();
        auto dogs = session->GetDogs();;

        // Для каждой собаки находим игрока и складываем в результат
        for ( const auto& dog : dogs ) {
            auto dog_name = dog.GetName();
            auto player = players_->FinByDog(dog);
            res.push_back({player->GetId(), player->GetName()});
        }
    } else {
        throw ListPlayersError{ListPlayersErrorReason::InvalidToken};
    }

    return res;
}

///  ---  Application  ---  ///

model::Game::Maps Application::ListMaps() {
    return list_maps_.GetMaps();
}

const model::Map& Application::FindMap(std::string map_id) {
    return get_map_.GetMap(model::Map::Id(map_id));
}

JoinGameResult Application::JoinGame(std::string user_name, std::string map_id) {
    return join_game_.JoinGame(model::Map::Id{map_id}, Player::Name{user_name});
}

ListPlayersResult Application::GetPlayers(std::string_view token) {
    return list_players_.GetPlayers(app::Token(std::string(token)));
}


}  // namespace app
