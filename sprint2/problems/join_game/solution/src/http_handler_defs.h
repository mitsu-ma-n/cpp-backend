#pragma once

#include <string_view>

namespace api_strings {
    using namespace std::literals;
    constexpr static std::string_view MAIN_PATH = "/api/"sv;
    constexpr static int              MAIN_POS  = 0;
    constexpr static std::string_view VERSION_PATH = "v1"sv;
    constexpr static int              VERSION_POS  = 1;
    constexpr static std::string_view MAPS_PATH = "maps"sv;
    constexpr static int              MAPS_POS  = 2;
    constexpr static int              MAP_ID_POS = 3;
}

namespace ContentType {
    using namespace std::literals;
    constexpr static std::string_view APP_JSON          = "application/json"sv;
    constexpr static std::string_view APP_OCT_STREAM    = "application/octet-stream"sv;
    constexpr static std::string_view APP_XML           = "application/xml"sv;
    constexpr static std::string_view AUDIO_MPEG        = "audio/mpeg"sv;
    constexpr static std::string_view IMG_BMP           = "image/bmp"sv;
    constexpr static std::string_view IMG_GIF           = "image/gif"sv;
    constexpr static std::string_view IMG_ICON          = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMG_JPEG          = "image/jpeg"sv;
    constexpr static std::string_view IMG_PNG           = "image/png"sv;
    constexpr static std::string_view IMG_SVG           = "image/svg+xml"sv;
    constexpr static std::string_view IMG_TIFF          = "image/tiff"sv;
    constexpr static std::string_view TEXT_CSS          = "text/css"sv;
    constexpr static std::string_view TEXT_HTML         = "text/html"sv;
    constexpr static std::string_view TEXT_JS           = "text/javascript"sv;
    constexpr static std::string_view TEXT_PLAIN        = "text/plain"sv;
};
