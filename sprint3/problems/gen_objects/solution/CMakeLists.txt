cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

# обратите внимание на аргумент TARGETS у команды conan_basic_setup
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(game_server
	src/app/app.cpp
	src/app/app.h
	src/app/join_use_case.cpp
	src/app/join_use_case.h
	src/app/maps_use_case.cpp
	src/app/maps_use_case.h
	src/app/player_use_case.cpp
	src/app/player_use_case.h
	src/app/players_use_case.cpp
	src/app/players_use_case.h
	src/app/players.cpp
	src/app/players.h
	src/app/state_use_case.cpp
	src/app/state_use_case.h
	src/app/tick_use_case.cpp
	src/app/tick_use_case.h
	src/http/api_handler.cpp
	src/http/api_handler.h
	src/http/file_handler.cpp
	src/http/file_handler.h
	src/http/http_handler_defs.h
	src/http/http_handler_types.cpp
	src/http/http_handler_types.h
	src/http/http_server.cpp
	src/http/http_server.h
	src/http/request_handler_logging.h
	src/http/request_handler.cpp
	src/http/request_handler.h
	src/model/model.cpp
	src/model/model.h
	src/utils/boost_json.cpp
	src/utils/json_fields.h
	src/utils/json_loader.cpp
	src/utils/json_loader.h
	src/utils/logger.cpp
	src/utils/logger.h
	src/utils/loot_generator.cpp
	src/utils/loot_generator.h
	src/utils/tagged.h
	src/utils/utils.cpp
	src/utils/utils.h
	src/utils/write_adapter.h
	src/main.cpp
	src/sdk.h
	src/server_params.h
)

target_include_directories(game_server PRIVATE src src/app src/http src/model src/utils)

# используем "импортированную" цель CONAN_PKG::boost
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server Threads::Threads CONAN_PKG::boost)

add_executable(tests
	tests/loot_generator_tests.cpp
	src/utils/loot_generator.cpp
	src/utils/loot_generator.h
)

target_include_directories(game_server PRIVATE src/utils)

target_include_directories(tests PRIVATE CONAN_PKG::catch2)
target_link_libraries(tests PRIVATE CONAN_PKG::catch2)

