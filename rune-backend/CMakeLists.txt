cmake_minimum_required(VERSION 3.20)
project(rune VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Threads
find_package(Threads REQUIRED)

# SQLite3 (system or vendored)
find_package(SQLite3 QUIET)
if(NOT SQLite3_FOUND)
    message(STATUS "SQLite3 not found via find_package, using vendored sqlite3.c")
    add_library(sqlite3_vendored STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/sqlite3.c
    )
    target_include_directories(sqlite3_vendored PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    )
    set(SQLITE3_TARGET sqlite3_vendored)
else()
    set(SQLITE3_TARGET SQLite::SQLite3)
endif()

add_executable(rune
    src/main.cpp
)

target_link_libraries(rune PRIVATE
    Threads::Threads
    ${SQLITE3_TARGET}
)

target_include_directories(rune PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
