# ── SQLite3 (prefer system) ───────────────────────────────────────────────────
find_package(SQLite3 QUIET)

if(SQLite3_FOUND)
    add_library(sqlite3 INTERFACE)
    target_link_libraries(sqlite3 INTERFACE SQLite::SQLite3)
else()
    message(FATAL_ERROR "SQLite3 not found. Please install SQLite3 development package (e.g., libsqlite3-dev).")
endif()