
cmake_minimum_required(VERSION 3.16)
project(arena_tracker)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# ── Find system libraries ─────────────────────────────────────────────────────
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
 
# ── Crow (header-only, path set via -DCROW_DIR or defaults to /crow/include) ──
set(CROW_DIR "/crow/include" CACHE PATH "Path to Crow headers")
 
# ── Executable ────────────────────────────────────────────────────────────────
add_executable(arena_tracker
    src/main.cpp
    src/player_handler.cpp
    src/riot_client.cpp
)
 
target_include_directories(arena_tracker PRIVATE
    include
    ${CROW_DIR}
    ${CURL_INCLUDE_DIRS}
)
 
target_link_libraries(arena_tracker PRIVATE
    CURL::libcurl
    Threads::Threads
)