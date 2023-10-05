#pragma once

#include <string_view>

namespace api_strings {
    using namespace std::literals;
    constexpr static std::string_view MAIN_PATH = "api"sv;
    constexpr static int              MAIN_POS  = 0;
    constexpr static std::string_view VERSION_PATH = "v1"sv;
    constexpr static int              VERSION_POS  = 1;
    constexpr static std::string_view MAPS_PATH = "maps"sv;
    constexpr static int              MAPS_POS  = 2;
    constexpr static int              MAP_ID_POS = 3;
}