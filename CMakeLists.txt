cmake_minimum_required(VERSION 3.16)
project(arena_tracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(arena_tracker src/main.cpp src/player_handler.cpp src/riot_client.cpp)

target_include_directories(arena_tracker PRIVATE include)