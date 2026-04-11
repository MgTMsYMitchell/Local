cmake_minimum_required(VERSION 3.20)
project(rune-backend CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include(FetchContent)

# ── SQLite3 (system) ──────────────────────────────────────────────────────────
find_package(SQLite3 REQUIRED)

# ── Monocypher 3.1.3 ─────────────────────────────────────────────────────────
FetchContent_Declare(monocypher
    URL https://github.com/LoupVaillant/Monocypher/archive/3.1.3.tar.gz
)
FetchContent_MakeAvailable(monocypher)
add_library(monocypher_lib STATIC
    ${monocypher_SOURCE_DIR}/src/monocypher.c
    ${monocypher_SOURCE_DIR}/src/optional/monocypher-ed25519.c
)
target_include_directories(monocypher_lib PUBLIC
    ${monocypher_SOURCE_DIR}/src
    ${monocypher_SOURCE_DIR}/src/optional
)

# ── cpp-httplib v0.15.3 (header-only) ────────────────────────────────────────
FetchContent_Declare(httplib
    URL https://github.com/yhirose/cpp-httplib/archive/v0.15.3.tar.gz
)
FetchContent_MakeAvailable(httplib)
add_library(httplib_iface INTERFACE)
target_include_directories(httplib_iface INTERFACE ${httplib_SOURCE_DIR})

# ── nlohmann/json v3.11.3 ─────────────────────────────────────────────────────
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/archive/v3.11.3.tar.gz
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install   OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

# ── Common include paths ──────────────────────────────────────────────────────
set(COMMON_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
)

# ── rune (:7070) ──────────────────────────────────────────────────────────────
add_executable(rune
    src/main.cpp
    src/db/database.cpp
    src/wallet/wallet.cpp
)
target_include_directories(rune PRIVATE ${COMMON_INCLUDES})
target_link_libraries(rune PRIVATE
    SQLite::SQLite3
    monocypher_lib
    httplib_iface
    nlohmann_json::nlohmann_json
)

# ── rune_brain (:7071) ────────────────────────────────────────────────────────
add_executable(rune_brain
    src/brain_main.cpp
    src/brain/brain_system.cpp
    src/wallet/wallet.cpp
)
target_include_directories(rune_brain PRIVATE ${COMMON_INCLUDES})
target_link_libraries(rune_brain PRIVATE
    SQLite::SQLite3
    monocypher_lib
    httplib_iface
    nlohmann_json::nlohmann_json
)

message(STATUS "rune-backend: rune(:7070) + rune_brain(:7071)")
