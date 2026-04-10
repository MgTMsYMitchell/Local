cmake_minimum_required(VERSION 3.20)
project(rune VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ── Threads (system) ──────────────────────────────────────────────────────────
find_package(Threads REQUIRED)

# ── SQLite3 amalgamation ──────────────────────────────────────────────────────
# Fetches the official single-file amalgamation from the sqlite/sqlite mirror.
# Tag format: "version-X.Y.Z" on https://github.com/sqlite/sqlite
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

# The mirror includes Makefile.msc and the amalgamation sources.
# We build just the two amalgamation files.
add_library(sqlite3 STATIC
    ${sqlite3_src_SOURCE_DIR}/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC
    ${sqlite3_src_SOURCE_DIR}
)
# Silence warnings from third-party source
target_compile_options(sqlite3 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)
set_target_properties(sqlite3 PROPERTIES LANGUAGE C)

# ── cpp-httplib (header-only embedded HTTP server) ────────────────────────────
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.15.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(httplib)

# ── nlohmann/json (header-only JSON) ─────────────────────────────────────────
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
)

target_link_libraries(rune PRIVATE
    Threads::Threads
    sqlite3
    httplib::httplib
    nlohmann_json::nlohmann_json
)

target_include_directories(rune PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
