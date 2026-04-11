cmake_minimum_required(VERSION 3.16)
project(QuantumFractalSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Options ──────────────────────────────────────────────────────────────────
option(QFS_BUILD_TESTS "Build the test suite" ON)
option(QFS_ENABLE_GPU  "Enable CUDA GPU kernels (requires CUDA toolkit)" OFF)

# ── Dependencies ─────────────────────────────────────────────────────────────
include(FetchContent)

# SQLite3 — prefer system, fall back to bundled amalgamation
find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    add_library(qfs_sqlite3 INTERFACE)
    target_link_libraries(qfs_sqlite3 INTERFACE SQLite::SQLite3)
    message(STATUS "QFS: Using system SQLite3 ${SQLite3_VERSION}")
else()
    message(FATAL_ERROR "SQLite3 not found. Install libsqlite3-dev.")
endif()

# nlohmann/json (header-only)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# ── Source library ───────────────────────────────────────────────────────────
file(GLOB_RECURSE QFS_SOURCES
    src/app/*.cpp
    src/compression/*.cpp
    src/network/*.cpp
    src/engine/*.cpp
    src/cpu/*.cpp
    src/storage/*.cpp
)

add_library(qfs_core STATIC ${QFS_SOURCES})
target_include_directories(qfs_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(qfs_core PUBLIC qfs_sqlite3 nlohmann_json::nlohmann_json)

if(UNIX)
    target_link_libraries(qfs_core PUBLIC pthread)
endif()

# ── Main executable ──────────────────────────────────────────────────────────
add_executable(QuantumFractalSystem src/main.cpp)
target_link_libraries(QuantumFractalSystem PRIVATE qfs_core)

# ── GPU (optional) ───────────────────────────────────────────────────────────
if(QFS_ENABLE_GPU)
    enable_language(CUDA)
    add_library(qfs_gpu STATIC src/gpu/ComputeKernels.cu)
    target_include_directories(qfs_gpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(QuantumFractalSystem PRIVATE qfs_gpu)
    target_compile_definitions(QuantumFractalSystem PRIVATE QFS_HAS_GPU=1)
endif()

# ── Tests ────────────────────────────────────────────────────────────────────
if(QFS_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(googletest)

    set(QFS_TEST_SOURCES
        tests/test_compression.cpp
        tests/test_math.cpp
        tests/test_network.cpp
        tests/test_engine.cpp
        tests/test_storage.cpp
    )

    add_executable(qfs_tests ${QFS_TEST_SOURCES})
    target_link_libraries(qfs_tests PRIVATE qfs_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(qfs_tests)
endif()
