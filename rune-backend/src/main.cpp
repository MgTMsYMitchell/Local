#include <atomic>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <sqlite3.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ── helpers ───────────────────────────────────────────────────────────────────

static std::mutex g_log_mutex;

template <typename... Args>
void log(const char* fmt, Args&&... args)
{
    std::lock_guard<std::mutex> lk(g_log_mutex);
    std::printf(fmt, std::forward<Args>(args)...);
    std::fflush(stdout);
}

// ── database ──────────────────────────────────────────────────────────────────

static const char* SCHEMA_SQL = R"sql(
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
    sequence    TEXT,
    description TEXT,
    created_at  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS stories (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    title      TEXT    NOT NULL,
    body       TEXT,
    rune_id    INTEGER REFERENCES runes(id),
    created_at TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS mythic_moments (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    event      TEXT    NOT NULL,
    rune_id    INTEGER REFERENCES runes(id),
    story_id   INTEGER REFERENCES stories(id),
    timestamp  TEXT    DEFAULT (datetime('now'))
);
)sql";

struct Database {
    sqlite3* db = nullptr;
    std::mutex mtx;

    explicit Database(const std::string& path)
    {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
            throw std::runtime_error(std::string("open db: ") + sqlite3_errmsg(db));
        exec(SCHEMA_SQL);
        log("[db] opened %s\n", path.c_str());
    }

    ~Database() { if (db) sqlite3_close(db); }

    // Not copyable/movable because of the mutex and raw pointer.
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void exec(const char* sql)
    {
        char* err = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
            std::string msg = err ? err : "unknown";
            sqlite3_free(err);
            throw std::runtime_error("SQL: " + msg);
        }
    }

    int count(const char* table)
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

    // Returns all rows from `runes` as a JSON array.
    json all_runes()
    {
        std::lock_guard<std::mutex> lk(mtx);
        const char* sql =
            "SELECT id, name, glyph, meaning, created_at FROM runes ORDER BY id;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        json arr = json::array();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto text = [&](int col) -> std::string {
                const unsigned char* p = sqlite3_column_text(stmt, col);
                return p ? reinterpret_cast<const char*>(p) : "";
            };
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
};

// ── worker nodes ──────────────────────────────────────────────────────────────

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

// ── HTTP server ───────────────────────────────────────────────────────────────

static void setup_routes(httplib::Server& srv, Database& db,
                         std::atomic<bool>& shutdown)
{
    // CORS helper
    auto cors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    };

    // GET /health
    srv.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
        json body = {
            {"status", "ok"},
            {"runes",  db.count("runes")},
            {"stories", db.count("stories")},
            {"mythic_moments", db.count("mythic_moments")}
        };
        cors(res);
        res.set_content(body.dump(2), "application/json");
    });

    // GET /runes
    srv.Get("/runes", [&](const httplib::Request&, httplib::Response& res) {
        cors(res);
        res.set_content(db.all_runes().dump(2), "application/json");
    });

    // POST /shutdown  (for clean remote stop)
    srv.Post("/shutdown", [&](const httplib::Request&, httplib::Response& res) {
        cors(res);
        res.set_content("{\"status\":\"shutting down\"}", "application/json");
        shutdown.store(true);
        srv.stop();
    });
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    const std::string db_path  = (argc > 1) ? argv[1] : "rune.db";
    const int         http_port = (argc > 2) ? std::stoi(argv[2]) : 7070;

    log("[rune] starting — db: %s  http: :%d\n", db_path.c_str(), http_port);

    Database db(db_path);

    if (db.count("runes") == 0) {
        db.exec(R"sql(
            INSERT INTO runes (name, glyph, meaning) VALUES
                ('Fehu',     'ᚠ', 'Cattle, wealth, abundance'),
                ('Uruz',     'ᚢ', 'Aurochs, strength, endurance'),
                ('Thurisaz', 'ᚦ', 'Giant, thorn, chaos');
        )sql");
        log("[db] seeded initial runes\n");
    }

    // Worker nodes
    const int NODE_COUNT = 2;
    std::atomic<bool> shutdown{false};
    std::vector<std::thread> threads;
    threads.reserve(NODE_COUNT + 1);

    for (int i = 0; i < NODE_COUNT; ++i) {
        RuneNode node{i + 1, shutdown, db};
        threads.emplace_back([n = std::move(node)]() mutable { n.run(); });
    }

    // HTTP server on its own thread
    httplib::Server srv;
    setup_routes(srv, db, shutdown);

    threads.emplace_back([&srv, http_port]() {
        log("[http] listening on :%d\n", http_port);
        srv.listen("0.0.0.0", http_port);
        log("[http] server stopped\n");
    });

    log("[rune] %d node(s) + HTTP server running.\n"
        "       GET  http://localhost:%d/health\n"
        "       GET  http://localhost:%d/runes\n"
        "       POST http://localhost:%d/shutdown\n"
        "       Press ENTER to stop.\n",
        NODE_COUNT, http_port, http_port, http_port);

    std::cin.get();

    shutdown.store(true);
    srv.stop();
    for (auto& t : threads) t.join();

    log("[rune] shutdown complete.\n");
    return 0;
}

