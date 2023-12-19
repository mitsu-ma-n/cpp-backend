#pragma once

#include "model.h"
#include "players.h"
#include "join_use_case.h"
#include "players_use_case.h"
#include "state_use_case.h"
#include "maps_use_case.h"
#include "player_use_case.h"
#include "tick_use_case.h"
#include <string>

namespace app {

class Application {
public:
    Application(model::Game& game)
        : join_game_{game, tokens_, players_}
        , list_players_{tokens_, players_}
        , game_state_{tokens_, players_}
        , player_action_{tokens_}
        , tick_{game}
        , list_maps_{game}
        , get_map_{game} {
    }

    // Выдаёт список доступных карт 
    model::Game::Maps ListMaps();
    // Выдаёт ссылку на карту по её ID
    const model::Map& FindMap(std::string map_id);
    // Подключает игрока к указанной карте
    JoinGameResult JoinGame(std::string user_name, std::string map_id);
    // Для игрока с заданным токеном получает список играющих с ним игроков
    ListPlayersResult GetPlayers(std::string_view token);
    // Получает игровое состояние для игрока с заданным токеном
    GetStateResult GetState(std::string_view token);
    // Выполняет действие для игрока с заданным токеном
    PlayerActionResult ExecutePlayerAction(std::string_view token, PlayerAction action);
    // Выполняет один шаг по времени в игре
    TickResult ExecuteTick(Tick tick);

private:
    Players players_;
    PlayerTokens tokens_;

    JoinGameUseCase join_game_;
    ListPlayersUseCase list_players_;
    GetStateUseCase game_state_;
    PlayerActionUseCase player_action_;
    ListMapsUseCase list_maps_;
    GetMapUseCase get_map_;
    TickUseCase tick_;
};

}  // namespace app
