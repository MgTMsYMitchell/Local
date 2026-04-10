cmake_minimum_required(VERSION 3.20)
project(rune VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ── Threads (system) ──────────────────────────────────────────────────────────
find_package(Threads REQUIRED)

# ── SQLite3 amalgamation ──────────────────────────────────────────────────────
FetchContent_Declare(
    sqlite3_src
    GIT_REPOSITORY https://github.com/sqlite/sqlite.git
    GIT_TAG        version-3.45.2
    GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(sqlite3_src)
if(NOT sqlite3_src_POPULATED)
    FetchContent_Populate(sqlite3_src)
endif()

add_library(sqlite3 STATIC
    ${sqlite3_src_SOURCE_DIR}/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC
    ${sqlite3_src_SOURCE_DIR}
)
target_compile_options(sqlite3 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)
set_target_properties(sqlite3 PROPERTIES LANGUAGE C)

# ── Monocypher 3.1.3 — Ed25519 (SHA-512) signing ─────────────────────────────
FetchContent_Declare(
    monocypher
    GIT_REPOSITORY https://github.com/LoupVaillant/Monocypher.git
    GIT_TAG        3.1.3
    GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(monocypher)
if(NOT monocypher_POPULATED)
    FetchContent_Populate(monocypher)
endif()

add_library(monocypher STATIC
    ${monocypher_SOURCE_DIR}/src/monocypher.c
    ${monocypher_SOURCE_DIR}/src/optional/monocypher-ed25519.c
)
target_include_directories(monocypher PUBLIC
    ${monocypher_SOURCE_DIR}/src
    ${monocypher_SOURCE_DIR}/src/optional
)
target_compile_options(monocypher PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)
set_target_properties(monocypher PROPERTIES LANGUAGE C)

# ── cpp-httplib v0.15.3 — header-only embedded HTTP server ───────────────────
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.15.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(httplib)

# ── nlohmann/json v3.11.3 — header-only JSON ─────────────────────────────────
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# ── rune executable ───────────────────────────────────────────────────────────
add_executable(rune
    src/main.cpp
    src/db/database.cpp
    src/wallet/wallet.cpp
)

target_link_libraries(rune PRIVATE
    Threads::Threads
    sqlite3
    monocypher
    httplib::httplib
    nlohmann_json::nlohmann_json
)

target_include_directories(rune PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Suppress noisy warnings from our own code on MSVC.
if(MSVC)
    target_compile_options(rune PRIVATE /W3 /utf-8)
endif()
