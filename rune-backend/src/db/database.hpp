#pragma once
#include <mutex>
#include <stdexcept>
#include <string>

#include <sqlite3.h>
#include <nlohmann/json.hpp>

// ── Database ─────────────────────────────────────────────────────────────────
// Thread-safe wrapper around a single SQLite connection.
// All public methods (except exec()) acquire the internal mutex before
// touching the DB handle.  exec() is intentionally unlocked so it can be
// called safely from the constructor / run_migrations() chain; use
// exec_locked() from any post-construction caller.

// Aggregated row-counts returned by a single SQL round-trip.
struct HealthCounts {
    int runes          = 0;
    int macro_runes    = 0;
    int stories        = 0;
    int mythic_moments = 0;
};

struct Database {
    sqlite3*   db = nullptr;
    std::mutex mtx;

    explicit Database(const std::string& path);
    ~Database();

    // Non-copyable / non-movable (mutex + raw pointer).
    Database(const Database&)            = delete;
    Database& operator=(const Database&) = delete;

    // Execute one or more semicolon-separated SQL statements (no results).
    // NOT mutex-protected — safe only from the constructor chain.
    void exec(const char* sql);

    // Thread-safe variant of exec() — acquires mtx before executing.
    // Use this for any post-construction write (e.g. seeding in main).
    void exec_locked(const char* sql);

    // Return all four row-counts in a single SQLite round-trip.
    // Replaces four separate count() calls on the hot /health path.
    HealthCounts health_counts();

    // COUNT(*) of a single table — kept for convenience/one-off use.
    int count(const char* table);

    // Return rows in `runes` as a JSON array.
    // limit  — max rows to return (default 500, 0 = unlimited)
    // offset — skip this many rows (for pagination)
    nlohmann::json all_runes(int limit = 500, int offset = 0);

    // Apply versioned migrations from the schema_migrations table.
    void run_migrations();
};

