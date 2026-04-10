#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <sqlite3.h>

// ── helpers ──────────────────────────────────────────────────────────────────

static std::mutex g_log_mutex;

template <typename... Args>
void log(const char* fmt, Args&&... args)
{
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::printf(fmt, std::forward<Args>(args)...);
    std::fflush(stdout);
}

// ── database ─────────────────────────────────────────────────────────────────

static const char* SCHEMA_SQL = R"sql(
CREATE TABLE IF NOT EXISTS runes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    glyph       TEXT,
    meaning     TEXT,
    created_at  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS macro_runes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    sequence    TEXT,       -- comma-separated rune ids
    description TEXT,
    created_at  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS stories (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    title       TEXT    NOT NULL,
    body        TEXT,
    rune_id     INTEGER REFERENCES runes(id),
    created_at  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS mythic_moments (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    event       TEXT    NOT NULL,
    rune_id     INTEGER REFERENCES runes(id),
    story_id    INTEGER REFERENCES stories(id),
    timestamp   TEXT    DEFAULT (datetime('now'))
);
)sql";

struct Database {
    sqlite3* db = nullptr;

    explicit Database(const std::string& path)
    {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            std::fprintf(stderr, "[db] open error: %s\n", sqlite3_errmsg(db));
            throw std::runtime_error("cannot open database");
        }
        exec(SCHEMA_SQL);
        log("[db] opened %s\n", path.c_str());
    }

    ~Database()
    {
        if (db) sqlite3_close(db);
    }

    void exec(const char* sql)
    {
        char* errmsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
            std::string msg = errmsg ? errmsg : "unknown error";
            sqlite3_free(errmsg);
            throw std::runtime_error("SQL error: " + msg);
        }
    }

    // Returns the number of rows in a table (for status reporting).
    int count(const char* table)
    {
        std::string sql = std::string("SELECT COUNT(*) FROM ") + table + ";";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        int n = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return n;
    }
};

// ── worker nodes ─────────────────────────────────────────────────────────────

struct RuneNode {
    int id;
    std::atomic<bool>& shutdown;
    Database& db;

    void run()
    {
        log("[node %d] started\n", id);
        while (!shutdown.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (shutdown.load()) break;
            log("[node %d] heartbeat — runes: %d\n", id, db.count("runes"));
        }
        log("[node %d] stopped\n", id);
    }
};

// ── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    const std::string db_path =
        (argc > 1) ? argv[1] : "rune.db";

    log("[rune] starting — db: %s\n", db_path.c_str());

    Database db(db_path);

    // Seed a default rune if the table is empty.
    if (db.count("runes") == 0) {
        db.exec(R"sql(
            INSERT INTO runes (name, glyph, meaning) VALUES
                ('Fehu',  'ᚠ', 'Cattle, wealth, abundance'),
                ('Uruz',  'ᚢ', 'Aurochs, strength, endurance'),
                ('Thurisaz', 'ᚦ', 'Giant, thorn, chaos');
        )sql");
        log("[db] seeded initial runes\n");
    }

    // Spin up worker nodes.
    const int NODE_COUNT = 2;
    std::atomic<bool> shutdown{false};
    std::vector<std::thread> threads;
    threads.reserve(NODE_COUNT);

    for (int i = 0; i < NODE_COUNT; ++i) {
        RuneNode node{i + 1, shutdown, db};
        threads.emplace_back([n = std::move(node)]() mutable { n.run(); });
    }

    log("[rune] %d node(s) running. Press ENTER to stop.\n", NODE_COUNT);
    std::cin.get();

    shutdown.store(true);
    for (auto& t : threads) t.join();

    log("[rune] shutdown complete.\n");
    return 0;
}
