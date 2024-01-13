#pragma once

#include "game.h"
#include "players.h"
#include "join_use_case.h"
#include "players_use_case.h"
#include "state_use_case.h"
#include "maps_use_case.h"
#include "player_use_case.h"
#include "tick_use_case.h"

#include "app_serialization.h"

#include <boost/signals2.hpp>
#include <chrono>
#include <filesystem>

namespace app {

namespace sig = boost::signals2;
using milliseconds = std::chrono::milliseconds;
using namespace std::literals;

class Application {
public:
    using TickSignal = sig::signal<void(milliseconds delta)>;

    Application(model::Game& game)
        : join_game_{game, tokens_, players_}
        , list_players_{tokens_, players_}
        , game_state_{tokens_, players_}
        , player_action_{tokens_}
        , tick_{game}
        , list_maps_{game}
        , get_map_{game}
        , state_serializer_{game, players_, tokens_}
        {
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

    // Сохранение состояния игры в файл
    void SerializeApp(const std::filesystem::path& path);

    // Добавляем обработчик сигнала tick и возвращаем объект connection для управления,
    // при помощи которого можно отписаться от сигнала
    [[nodiscard]] sig::connection DoOnTick(const TickSignal::slot_type& handler) {
        return tick_signal_.connect(handler);
    }

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

    TickSignal tick_signal_;

    serialization::StateSerializer state_serializer_;
};

}  // namespace app
