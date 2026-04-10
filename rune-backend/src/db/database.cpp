#include "database.hpp"

#include <stdexcept>
#include <string>

// Schema applied on first open (idempotent via IF NOT EXISTS).
static const char* SCHEMA_SQL = R"sql(
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
)sql";

// ── migration descriptors ─────────────────────────────────────────────────────
struct Migration {
    int         version;
    const char* name;
    const char* sql;
};

static const Migration MIGRATIONS[] = {
    {
        1, "initial_schema",
        SCHEMA_SQL
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
};

// ── Database implementation ───────────────────────────────────────────────────

Database::Database(const std::string& path)
{
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error(std::string("sqlite open: ") + sqlite3_errmsg(db));

    // Enable WAL for better concurrent read performance.
    exec("PRAGMA journal_mode=WAL;");
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

int Database::count(const char* table)
{
    std::lock_guard<std::mutex> lk(mtx);
    std::string sql = std::string("SELECT COUNT(*) FROM ") + table + ";";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    int n = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return n;
}

nlohmann::json Database::all_runes()
{
    std::lock_guard<std::mutex> lk(mtx);
    const char* sql =
        "SELECT id, name, glyph, meaning, created_at FROM runes ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

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
    // Ensure migration tracking table exists before anything else.
    exec(R"sql(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version    INTEGER PRIMARY KEY,
            name       TEXT    NOT NULL,
            applied_at TEXT    DEFAULT (datetime('now'))
        );
    )sql");

    for (const auto& m : MIGRATIONS) {
        // Check if this version is already applied.
        std::string check =
            "SELECT COUNT(*) FROM schema_migrations WHERE version = " +
            std::to_string(m.version) + ";";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, check.c_str(), -1, &stmt, nullptr);
        int already = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            already = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (already) continue;

        exec(m.sql);

        std::string record =
            std::string("INSERT INTO schema_migrations (version, name) VALUES (") +
            std::to_string(m.version) + ", '" + m.name + "');";
        exec(record.c_str());
    }
}
