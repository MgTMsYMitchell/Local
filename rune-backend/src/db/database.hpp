#pragma once
#include <mutex>
#include <stdexcept>
#include <string>

#include <sqlite3.h>
#include <nlohmann/json.hpp>

// ── Database ─────────────────────────────────────────────────────────────────
// Thread-safe wrapper around a single SQLite connection.
// All public methods acquire an internal mutex before touching the DB handle.

struct Database {
    sqlite3* db = nullptr;
    std::mutex mtx;

    explicit Database(const std::string& path);
    ~Database();

    // Non-copyable / non-movable (mutex + raw pointer).
    Database(const Database&)            = delete;
    Database& operator=(const Database&) = delete;

    // Execute one or more semicolon-separated SQL statements (no results).
    void exec(const char* sql);

    // COUNT(*) of a table — used for heartbeat / health reporting.
    int count(const char* table);

    // Return all rows in `runes` as a JSON array.
    nlohmann::json all_runes();

    // Apply versioned migrations from the schema_migrations table.
    void run_migrations();
};
