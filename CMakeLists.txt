cmake_minimum_required(VERSION 3.25)

project(web_server
    VERSION 1.0
    DESCRIPTION "A basic HTTP server"
    LANGUAGES CXX)

# TODO: Determine if we really need to include headers
set(SOURCES
    src/main.cpp src/util.h src/server.h src/server.cpp src/client.h src/client.cpp
    src/http_request.h src/http_request.cpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # We need this for clangd to get a compile_commands.json

add_executable(web ${SOURCES})
target_compile_options(web PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wno-error=unused-parameter -Wno-error=unused-variable
    -Wno-error=unused-but-set-variable)
