#pragma once

#include <filesystem>

#include "model.h"
#include "http_handler_types.h"
#include "app_use_cases.h"

namespace model {
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Office const& office);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Building const& building);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Road const& road);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Map const& map);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, std::vector<Map> const& maps);
} // namespace model

namespace app {
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::JoinGameResult const& join_result);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::PlayerInfo const& player);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, app::ListPlayersResult const& players_result);
} // namespace app

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);
bool ReadJoinParamsFromString(http_handler::JoinParams& params, std::string str);

}  // namespace json_loader
