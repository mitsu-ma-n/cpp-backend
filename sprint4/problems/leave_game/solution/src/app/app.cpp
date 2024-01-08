#include "app.h"

namespace app {

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

GetStateResult Application::GetState(std::string_view token) {
    return game_state_.GetState(app::Token(std::string(token)));
}

PlayerActionResult Application::ExecutePlayerAction(std::string_view token, PlayerAction action) {
    return player_action_.ExecutePlayerAction(app::Token(std::string(token)), action);
}

TickResult Application::ExecuteTick(Tick tick) {
    auto tick_res = tick_.ExecuteTick(tick);
    
    // Уведомляем подписчиков сигнала tick
    tick_signal_(tick.GetTimeDelta());

    return tick_res;
}

}  // namespace app
