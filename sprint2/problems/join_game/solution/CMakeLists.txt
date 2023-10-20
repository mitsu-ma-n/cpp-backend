cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

# обратите внимание на аргумент TARGETS у команды conan_basic_setup
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(game_server
	src/api_handler.cpp
	src/api_handler.h
	src/app.cpp
	src/app.h
	src/boost_json.cpp
	src/file_handler.cpp
	src/file_handler.h
	src/http_handler_defs.h
	src/http_handler_types.cpp
	src/http_handler_types.h
	src/http_server.cpp
	src/http_server.h
	src/json_fields.h
	src/json_loader.cpp
	src/json_loader.h
	src/logger.cpp
	src/logger.h
	src/logging_request_handler.h
	src/main.cpp
	src/model.cpp
	src/model.h
	src/request_handler.cpp
	src/request_handler.h
	src/sdk.h
	src/server_params.h
	src/tagged.h
	src/write_adapter.h
)

# используем "импортированную" цель CONAN_PKG::boost
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server Threads::Threads CONAN_PKG::boost)
