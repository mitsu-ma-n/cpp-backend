#pragma once

#include <filesystem>

#include "model.h"
#include "http_handler_types.h"
#include "join_use_case.h"
#include "players_use_case.h"
#include "state_use_case.h"
#include "player_use_case.h"
#include "tick_use_case.h"

namespace model {
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Office const& office);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Building const& building);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Road const& road);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Position const& pos);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Speed const& speed);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Dog const& dog);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Map const& map);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, std::vector<Map> const& maps);
} // namespace model

namespace app {
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::JoinGameResult const& join_result);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::PlayerInfo const& player);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::ListPlayersResult const& players_result);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::GetStateResult const& state_result);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::PlayerActionResult const& action_result);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::TickResult const& action_result);
} // namespace app

namespace json_loader {
    model::Game LoadGame(const std::filesystem::path& json_path);
    bool ReadJoinParamsFromString(http_handler::JoinParams& params, std::string str);
    bool ReadPlayerActionParamsFromString(http_handler::PlayerActionParams& params, std::string str);
    bool ReadTickParamsFromString(http_handler::TickParams& params, std::string str);
}  // namespace json_loader
