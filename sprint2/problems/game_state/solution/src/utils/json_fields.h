#pragma once

namespace json_field {
    // Game
    constexpr static char GAME_MAPS[] = "maps";
    // Map
    constexpr static char MAP_ID[]          = "id";
    constexpr static char MAP_NAME[]        = "name";
    constexpr static char MAP_ROADS[]       = "roads";
    constexpr static char MAP_BUILDINGS[]   = "buildings";
    constexpr static char MAP_OFFICES[]     = "offices";
    // Road
    constexpr static char ROAD_START_X[] = "x0";
    constexpr static char ROAD_START_Y[] = "y0";
    constexpr static char ROAD_END_X[]   = "x1";
    constexpr static char ROAD_END_Y[]   = "y1";
    // Building
    constexpr static char BUILDING_POS_X[]  = "x";
    constexpr static char BUILDING_POS_Y[]  = "y";
    constexpr static char BUILDING_SIZE_W[] = "w";
    constexpr static char BUILDING_SIZE_H[] = "h";
    // Office
    constexpr static char OFFICE_ID[]        = "id";
    constexpr static char OFFICE_POS_X[]     = "x";
    constexpr static char OFFICE_POS_Y[]     = "y";
    constexpr static char OFFICE_OFFSET_DX[] = "offsetX";
    constexpr static char OFFICE_OFFSET_DY[] = "offsetY";
    // Errors
    constexpr static char ERROR_CODE[]    = "code";
    constexpr static char ERROR_MESSAGE[] = "message";
    // Logger
    constexpr static char LOGGER_TIMESTAMP[] = "timestamp";
    constexpr static char LOGGER_DATA[]      = "data";
    constexpr static char LOGGER_MESSAGE[]   = "message";
    // Server
    constexpr static char SERVER_PORT[]           = "port";
    constexpr static char SERVER_ADDRESS[]        = "address";
    constexpr static char REQUEST_IP[]            = "ip";
    constexpr static char REQUEST_URI[]           = "URI";
    constexpr static char REQUEST_METHOD[]        = "method";
    constexpr static char RESPONSE_TIME[]         = "response_time";
    constexpr static char RESPONSE_CODE[]         = "code";
    constexpr static char RESPONSE_CONTENT_TYPE[] = "content_type";
    // API
    constexpr static char API_CODE_BAD_REQUEST[]      = "badRequest";
    constexpr static char API_CODE_INVALID_ARGUMENT[] = "invalidArgument";
    constexpr static char API_CODE_INVALID_METHOD[]   = "invalidMethod";
    constexpr static char API_CODE_INVALID_TOKEN[]    = "invalidToken";
    constexpr static char API_CODE_UNKNOWN_TOKEN[]    = "unknownToken";
    constexpr static char API_CODE_MAP_NOT_FOUND[]    = "mapNotFound";
    // JoinParams
    constexpr static char JOIN_NAME[]   = "userName";
    constexpr static char JOIN_MAP_ID[] = "mapId";
    // JoinResult
    constexpr static char JOIN_TOKEN[]  = "authToken";
    constexpr static char JOIN_PLAYER_ID[]  = "playerId";
    // ListPlayers
    constexpr static char LIST_PLAYERS_NAME[]  = "name";
}
