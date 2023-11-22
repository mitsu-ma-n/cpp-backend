#pragma once

#include "model.h"
#include "players.h"
#include "app_use_cases.h"
#include <string>

namespace app {

class Application {
public:
    Application(model::Game& game)
    : join_game_{game, tokens_, players_}
    , list_players_{tokens_, players_}
    , game_state_{tokens_, players_}
    , list_maps_{game}
    , get_map_{game} {

    }

    // Выдаёт список доступных карт 
    model::Game::Maps ListMaps();
    // Выдаёт ссылку на карту по её ID
    const model::Map& FindMap(std::string map_id);
    // Подключает игрока к указанной карте @todo: Какого игрока???
    JoinGameResult JoinGame(std::string user_name, std::string map_id);
    // Для игрока с заданным токеном получает список играющих с ним игроков
    ListPlayersResult GetPlayers(std::string_view token);
    // Получает игровое состояние для игрока с заданным токеном
    GetStateResult GetState(std::string_view token);

private:
    Players players_;
    PlayerTokens tokens_;

    JoinGameUseCase join_game_;
    ListPlayersUseCase list_players_;
    GetStateUseCase game_state_;
    ListMapsUseCase list_maps_;
    GetMapUseCase get_map_;
};

}  // namespace app
