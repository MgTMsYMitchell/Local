#include "database.hpp"

#include <stdexcept>
#include <string>

// ── migration descriptors ─────────────────────────────────────────────────────
struct Migration {
    int         version;
    const char* name;
    const char* sql;
};

static const Migration MIGRATIONS[] = {
    {
        1, "initial_schema",
        R"sql(
CREATE TABLE IF NOT EXISTS schema_migrations (
    version    INTEGER PRIMARY KEY,
    name       TEXT    NOT NULL,
    applied_at TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS runes (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    name       TEXT    NOT NULL UNIQUE,
    glyph      TEXT,
    meaning    TEXT,
    created_at TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS macro_runes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    sequence    TEXT,       -- comma-separated rune ids
    description TEXT,
    created_at  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS stories (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    title      TEXT    NOT NULL,
    body       TEXT,
    rune_id    INTEGER REFERENCES runes(id) ON DELETE SET NULL,
    created_at TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS mythic_moments (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    event      TEXT    NOT NULL,
    rune_id    INTEGER REFERENCES runes(id) ON DELETE SET NULL,
    story_id   INTEGER REFERENCES stories(id) ON DELETE SET NULL,
    timestamp  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS wallet (
    id         INTEGER PRIMARY KEY CHECK (id = 1),
    public_key TEXT    NOT NULL,
    secret_key TEXT    NOT NULL,
    created_at TEXT    DEFAULT (datetime('now'))
);
        )sql"
    },
    {
        2, "add_agent_log",
        R"sql(
CREATE TABLE IF NOT EXISTS agent_log (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    agent      TEXT    NOT NULL,
    level      TEXT    NOT NULL DEFAULT 'info',
    message    TEXT    NOT NULL,
    created_at TEXT    DEFAULT (datetime('now'))
);
        )sql"
    },
    {
        3, "perf_indexes",
        R"sql(
-- Indexes on FK columns eliminate full-table scans on JOIN / WHERE lookups.
CREATE INDEX IF NOT EXISTS idx_stories_rune_id  ON stories(rune_id);
CREATE INDEX IF NOT EXISTS idx_mythic_rune_id   ON mythic_moments(rune_id);
CREATE INDEX IF NOT EXISTS idx_mythic_story_id  ON mythic_moments(story_id);
-- agent_log: fast filtering by agent name and time-range queries.
CREATE INDEX IF NOT EXISTS idx_agentlog_agent   ON agent_log(agent);
CREATE INDEX IF NOT EXISTS idx_agentlog_ts      ON agent_log(created_at);
        )sql"
    },
};

// ── Database implementation ───────────────────────────────────────────────────

Database::Database(const std::string& path)
{
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error(std::string("sqlite open: ") + sqlite3_errmsg(db));

    // WAL mode: allows concurrent readers while a writer is active.
    exec("PRAGMA journal_mode=WAL;");

    // With WAL, NORMAL is safe and avoids the extra fsync of FULL.
    exec("PRAGMA synchronous=NORMAL;");

    // 40 MB page cache (negative value = kibibytes).
    exec("PRAGMA cache_size=-40000;");

    // Keep temp tables / indexes in RAM rather than a temp file.
    exec("PRAGMA temp_store=MEMORY;");

    exec("PRAGMA foreign_keys=ON;");

    run_migrations();
}

Database::~Database()
{
    if (db) sqlite3_close(db);
}

void Database::exec(const char* sql)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("SQL exec: " + msg);
    }
}

void Database::exec_locked(const char* sql)
{
    std::lock_guard<std::mutex> lk(mtx);
    exec(sql);
}

// Returns all four row-counts in a single SQLite round-trip.
// Using scalar subqueries is cheaper than four separate statements because
// SQLite processes them in one B-tree pass over the internal stat pages.
HealthCounts Database::health_counts()
{
    std::lock_guard<std::mutex> lk(mtx);

    static const char* sql =
        "SELECT"
        "  (SELECT COUNT(*) FROM runes)          AS rune_count,"
        "  (SELECT COUNT(*) FROM macro_runes)    AS macro_count,"
        "  (SELECT COUNT(*) FROM stories)        AS story_count,"
        "  (SELECT COUNT(*) FROM mythic_moments) AS mythic_count;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    HealthCounts hc{};
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        hc.runes          = sqlite3_column_int(stmt, 0);
        hc.macro_runes    = sqlite3_column_int(stmt, 1);
        hc.stories        = sqlite3_column_int(stmt, 2);
        hc.mythic_moments = sqlite3_column_int(stmt, 3);
    }
    sqlite3_finalize(stmt);
    return hc;
}

int Database::count(const char* table)
{
    std::lock_guard<std::mutex> lk(mtx);
    // NOTE: table is always a string literal from internal callers, never
    // user-supplied, so string concatenation here is safe.
    std::string sql = std::string("SELECT COUNT(*) FROM ") + table + ";";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    int n = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return n;
}

nlohmann::json Database::all_runes(int limit, int offset)
{
    std::lock_guard<std::mutex> lk(mtx);

    // LIMIT/OFFSET with bound parameters prevents unbounded scans and
    // protects against (theoretical) injection via numeric args.
    static const char* sql =
        "SELECT id, name, glyph, meaning, created_at "
        "FROM runes ORDER BY id "
        "LIMIT ? OFFSET ?;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    // limit <= 0 means "return everything" — use SQLite's max signed int.
    sqlite3_bind_int(stmt, 1, (limit > 0) ? limit : 0x7fffffff);
    sqlite3_bind_int(stmt, 2, offset);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    nlohmann::json arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        arr.push_back({
            {"id",         sqlite3_column_int(stmt, 0)},
            {"name",       text(1)},
            {"glyph",      text(2)},
            {"meaning",    text(3)},
            {"created_at", text(4)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

void Database::run_migrations()
{
    // Ensure the tracking table exists before we query it.
    exec(R"sql(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version    INTEGER PRIMARY KEY,
            name       TEXT    NOT NULL,
            applied_at TEXT    DEFAULT (datetime('now'))
        );
    )sql");

    // Prepare reusable statements once and bind per-migration.
    // Using sqlite3_bind_int avoids string concatenation and is more efficient
    // when the migration count grows.
    sqlite3_stmt* check_stmt = nullptr;
    sqlite3_prepare_v2(db,
        "SELECT COUNT(*) FROM schema_migrations WHERE version = ?;",
        -1, &check_stmt, nullptr);

    sqlite3_stmt* ins_stmt = nullptr;
    sqlite3_prepare_v2(db,
        "INSERT INTO schema_migrations (version, name) VALUES (?, ?);",
        -1, &ins_stmt, nullptr);

    for (const auto& m : MIGRATIONS) {
        // Check whether this version is already applied.
        sqlite3_reset(check_stmt);
        sqlite3_bind_int(check_stmt, 1, m.version);
        int already = 0;
        if (sqlite3_step(check_stmt) == SQLITE_ROW)
            already = sqlite3_column_int(check_stmt, 0);

        if (already) continue;

        exec(m.sql);

        // Record the applied migration.
        sqlite3_reset(ins_stmt);
        sqlite3_bind_int (ins_stmt, 1, m.version);
        sqlite3_bind_text(ins_stmt, 2, m.name, -1, SQLITE_STATIC);
        sqlite3_step(ins_stmt);
    }

    sqlite3_finalize(check_stmt);
    sqlite3_finalize(ins_stmt);
}

