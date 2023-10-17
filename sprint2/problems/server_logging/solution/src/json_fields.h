#pragma once

namespace json_field {
    // Game
    constexpr static char GAME_MAPS[] = "maps";
    // Map
    constexpr static char MAP_ID[] = "id";
    constexpr static char MAP_NAME[] = "name";
    constexpr static char MAP_ROADS[] = "roads";
    constexpr static char MAP_BUILDINGS[] = "buildings";
    constexpr static char MAP_OFFICES[] = "offices";
    // Road
    constexpr static char ROAD_START_X[] = "x0";
    constexpr static char ROAD_START_Y[] = "y0";
    constexpr static char ROAD_END_X[] = "x1";
    constexpr static char ROAD_END_Y[] = "y1";
    // Building
    constexpr static char BUILDING_POS_X[] = "x";
    constexpr static char BUILDING_POS_Y[] = "y";
    constexpr static char BUILDING_SIZE_W[] = "w";
    constexpr static char BUILDING_SIZE_H[] = "h";
    // Office
    constexpr static char OFFICE_ID[] = "id";
    constexpr static char OFFICE_POS_X[] = "x";
    constexpr static char OFFICE_POS_Y[] = "y";
    constexpr static char OFFICE_OFFSET_DX[] = "offsetX";
    constexpr static char OFFICE_OFFSET_DY[] = "offsetY";
    // Errors
    constexpr static char ERROR_CODE[] = "code";
    constexpr static char ERROR_MESSAGE[] = "message";
    // Logger
    constexpr static char LOGGER_TIMESTAMP[] = "timestamp";
    constexpr static char LOGGER_DATA[] = "data";

    // При появлении новых данных в модели внутрь json_field нужно добавить их названия в JSON
}
