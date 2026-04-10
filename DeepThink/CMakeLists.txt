cmake_minimum_required(VERSION 3.20)
project(DeepThink CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# nlohmann/json — header-only JSON library
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# cpp-httplib — header-only HTTP server/client (HTTP only, no OpenSSL)
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.15.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(httplib)

# ---------------------------------------------------------------------------
# qr2_rune_engine  (static library)
# ---------------------------------------------------------------------------
add_library(qr2_rune_engine STATIC
    engine/qr2_rune_engine.cpp
)
target_include_directories(qr2_rune_engine PUBLIC engine)
target_compile_features(qr2_rune_engine PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# deepthink_service  (HTTP server: ingest + query endpoints)
# ---------------------------------------------------------------------------
add_executable(deepthink_service
    service/deepthink_service.cpp
)
target_include_directories(deepthink_service PRIVATE service)
target_link_libraries(deepthink_service PRIVATE
    qr2_rune_engine
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# ---------------------------------------------------------------------------
# deepthink_bench  (engine-only benchmark, no network I/O)
# ---------------------------------------------------------------------------
add_executable(deepthink_bench
    engine/benchmark.cpp
)
target_link_libraries(deepthink_bench PRIVATE qr2_rune_engine)

# ---------------------------------------------------------------------------
# Install targets
# ---------------------------------------------------------------------------
install(TARGETS deepthink_service deepthink_bench
        RUNTIME DESTINATION bin)
