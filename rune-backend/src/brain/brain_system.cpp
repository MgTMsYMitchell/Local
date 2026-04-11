/*
  PROJECT: QRrune / Cognitive Node Brain
  FILE: brain_system.cpp

  INTENT FOR GITHUB COPILOT CHAT / AI:
  - This file is the canonical reference for:
      * DB schema (SQLite, WAL): nodes, agents, runes, strategies, fusion_log,
        artifacts, audit, node_events, trust_ledger, size_assessment
      * Seed data: 24 Elder/Extended Futhark runes, 7 strategy runes
      * Core agents: Heart (node_status), Worker (chat + wallet jobs),
        GC (TTL, quarantine, pruning), RuneTrustManager (trust/TTL updates),
        RuneFusionEngine (simple pairwise fusion)
      * Stub agents: ChatAgent, WalletAgent, NodeAgent, TorrentAgent, EdgeAgent
      * EventBus (SSE-style async hook shared by all agents)
      * size_assessment: table-growth snapshots for compression benchmarks
  - When extending:
      * Keep everything idempotent and deterministic.
      * Prefer compiled SQL with sqlite3_bind_*; no ad-hoc string concatenation.
      * Preserve trust / TTL / fusion / quarantine semantics.
      * Add new agents by subclassing AgentBase and overriding tick().
      * Enqueue async work via BrainDb::enqueue_event(); WorkerAgent picks it up.
      * Emit user-visible events via AgentBase::emit() — EventBus routes them.
      * Treat runes as a mycelial brain: pathways evolve, are pruned (GC),
        and fused (RuneFusionEngine) based on trust and usage signals.
*/

#include "brain_system.hpp"

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <thread>
#include <chrono>

// ─────────────────────────────────────────────────────────────────────────────
// Utilities
// ─────────────────────────────────────────────────────────────────────────────

std::string utc_now()
{
    auto t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    return buf;
}

// ─────────────────────────────────────────────────────────────────────────────
// EventBus
// ─────────────────────────────────────────────────────────────────────────────

void EventBus::post(BrainEvent ev)
{
    if (ev.timestamp.empty()) ev.timestamp = utc_now();

    // Collect handlers while holding the lock, then call them without it to
    // prevent deadlocks when a handler calls unsubscribe().
    std::vector<Handler> snapshot;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        ring_.push_back(ev);
        while (static_cast<int>(ring_.size()) > kMaxRing)
            ring_.pop_front();
        for (auto& [id, h] : subs_)
            snapshot.push_back(h);
    }
    for (auto& h : snapshot) h(ev);
}

int EventBus::subscribe(Handler h)
{
    std::lock_guard<std::mutex> lk(mtx_);
    int id = next_id_++;
    subs_[id] = std::move(h);
    return id;
}

void EventBus::unsubscribe(int id)
{
    std::lock_guard<std::mutex> lk(mtx_);
    subs_.erase(id);
}

nlohmann::json EventBus::recent(int n) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    auto arr = nlohmann::json::array();
    int start = static_cast<int>(ring_.size()) - n;
    if (start < 0) start = 0;
    for (int i = start; i < static_cast<int>(ring_.size()); ++i) {
        const auto& ev = ring_[i];
        arr.push_back({
            {"source",    ev.source},
            {"type",      ev.type},
            {"payload",   ev.payload},
            {"timestamp", ev.timestamp}
        });
    }
    return arr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — constructor / schema
// ─────────────────────────────────────────────────────────────────────────────

BrainDb::BrainDb(const std::string& path)
{
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK)
        throw std::runtime_error(std::string("brain db open: ") + sqlite3_errmsg(db_));

    // Performance pragmas (same tuning as the main rune DB).
    raw_exec("PRAGMA journal_mode=WAL;");
    raw_exec("PRAGMA synchronous=NORMAL;");
    raw_exec("PRAGMA cache_size=-40000;");   // 40 MB page cache
    raw_exec("PRAGMA temp_store=MEMORY;");
    raw_exec("PRAGMA foreign_keys=ON;");

    ensure_schema();
    seed_data();
}

BrainDb::~BrainDb()
{
    if (db_) sqlite3_close(db_);
}

void BrainDb::raw_exec(const char* sql)
{
    char* err = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("brain sql: " + msg);
    }
}

void BrainDb::exec(const char* sql)
{
    std::lock_guard<std::mutex> lk(mtx_);
    raw_exec(sql);
}

void BrainDb::ensure_schema()
{
    // All CREATE TABLE / INDEX statements are idempotent via IF NOT EXISTS.
    // Schema is versioned implicitly: add new columns in new tables only so
    // existing databases remain forward-compatible.
    raw_exec(R"sql(

-- nodes: cognitive node registry (local + peer nodes)
CREATE TABLE IF NOT EXISTS nodes (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id    TEXT    NOT NULL UNIQUE,
    label      TEXT    NOT NULL DEFAULT '',
    status     TEXT    NOT NULL DEFAULT 'active', -- active|idle|quarantined|dead
    last_seen  TEXT    DEFAULT (datetime('now')),
    trust      REAL    NOT NULL DEFAULT 1.0,
    metadata   TEXT    NOT NULL DEFAULT '{}',
    created_at TEXT    DEFAULT (datetime('now'))
);

-- agents: registered agent instances and their runtime state
CREATE TABLE IF NOT EXISTS agents (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    type        TEXT    NOT NULL,  -- heart|worker|gc|trust|fusion|chat|wallet|node|torrent|edge
    status      TEXT    NOT NULL DEFAULT 'idle',  -- idle|running|paused|error
    last_tick   TEXT,
    tick_count  INTEGER NOT NULL DEFAULT 0,
    error_count INTEGER NOT NULL DEFAULT 0,
    metadata    TEXT    NOT NULL DEFAULT '{}',
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- runes: atomic cognitive units — the mycelial nodes of the brain
CREATE TABLE IF NOT EXISTS runes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    glyph       TEXT    NOT NULL DEFAULT '',
    category    TEXT    NOT NULL DEFAULT 'general',
    level       INTEGER NOT NULL DEFAULT 1,
    trust       REAL    NOT NULL DEFAULT 1.0,
    usage_count INTEGER NOT NULL DEFAULT 0,
    error_count INTEGER NOT NULL DEFAULT 0,
    ttl_seconds INTEGER NOT NULL DEFAULT 0,   -- 0 = immortal
    active      INTEGER NOT NULL DEFAULT 1,
    metadata    TEXT    NOT NULL DEFAULT '{}',
    created_at  TEXT    DEFAULT (datetime('now')),
    updated_at  TEXT    DEFAULT (datetime('now'))
);

-- strategies: named rune pathways (cognitive programs)
CREATE TABLE IF NOT EXISTS strategies (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    description TEXT    NOT NULL DEFAULT '',
    rune_ids    TEXT    NOT NULL DEFAULT '[]',  -- JSON array of rune names
    priority    INTEGER NOT NULL DEFAULT 0,
    active      INTEGER NOT NULL DEFAULT 1,
    usage_count INTEGER NOT NULL DEFAULT 0,
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- fusion_log: record of every pairwise rune fusion attempt
CREATE TABLE IF NOT EXISTS fusion_log (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    input_a     TEXT    NOT NULL,   -- rune name
    input_b     TEXT    NOT NULL,   -- rune name
    output_name TEXT    NOT NULL,   -- synthesised rune name
    score       REAL    NOT NULL DEFAULT 0.0,
    accepted    INTEGER NOT NULL DEFAULT 1,
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- artifacts: byproducts of fusion, GC, and agent activity (TTL-bound)
CREATE TABLE IF NOT EXISTS artifacts (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    type        TEXT    NOT NULL,   -- fusion_residue|gc_pruning|trust_collapse|…
    source      TEXT    NOT NULL,
    payload     TEXT    NOT NULL DEFAULT '{}',
    ttl_seconds INTEGER NOT NULL DEFAULT 86400,
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- audit: immutable append-only event log (never UPDATE or DELETE rows here)
CREATE TABLE IF NOT EXISTS audit (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    agent      TEXT    NOT NULL,
    action     TEXT    NOT NULL,
    subject    TEXT    NOT NULL DEFAULT '',
    detail     TEXT    NOT NULL DEFAULT '{}',
    created_at TEXT    DEFAULT (datetime('now'))
);

-- node_events: async work queue consumed by WorkerAgent
CREATE TABLE IF NOT EXISTS node_events (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    source       TEXT    NOT NULL,
    event_type   TEXT    NOT NULL,
    payload      TEXT    NOT NULL DEFAULT '{}',
    priority     INTEGER NOT NULL DEFAULT 0,
    processed    INTEGER NOT NULL DEFAULT 0,
    processed_at TEXT,
    attempts     INTEGER NOT NULL DEFAULT 0,
    created_at   TEXT    DEFAULT (datetime('now'))
);

-- trust_ledger: append-only trust delta log; RuneTrustManager aggregates this
CREATE TABLE IF NOT EXISTS trust_ledger (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    subject      TEXT    NOT NULL,
    subject_type TEXT    NOT NULL DEFAULT 'rune',  -- rune|node|agent
    trust_delta  REAL    NOT NULL,
    reason       TEXT    NOT NULL DEFAULT '',
    created_at   TEXT    DEFAULT (datetime('now'))
);

-- size_assessment: table-growth snapshots for compression / heatmap analysis
CREATE TABLE IF NOT EXISTS size_assessment (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    table_name TEXT    NOT NULL,
    row_count  INTEGER NOT NULL,
    sampled_at TEXT    DEFAULT (datetime('now'))
);

-- ── Indexes ──────────────────────────────────────────────────────────────────
CREATE INDEX IF NOT EXISTS idx_runes_category     ON runes(category);
CREATE INDEX IF NOT EXISTS idx_runes_active       ON runes(active);
CREATE INDEX IF NOT EXISTS idx_runes_trust        ON runes(trust);
CREATE INDEX IF NOT EXISTS idx_strategies_active  ON strategies(active);
CREATE INDEX IF NOT EXISTS idx_fusion_inputs      ON fusion_log(input_a, input_b);
CREATE INDEX IF NOT EXISTS idx_fusion_output      ON fusion_log(output_name);
CREATE INDEX IF NOT EXISTS idx_artifacts_type     ON artifacts(type);
CREATE INDEX IF NOT EXISTS idx_audit_agent        ON audit(agent);
CREATE INDEX IF NOT EXISTS idx_audit_ts           ON audit(created_at);
CREATE INDEX IF NOT EXISTS idx_nevents_pending    ON node_events(processed, priority DESC, created_at);
CREATE INDEX IF NOT EXISTS idx_trust_subject      ON trust_ledger(subject, subject_type);
CREATE INDEX IF NOT EXISTS idx_sizeass_table      ON size_assessment(table_name, sampled_at);

    )sql");
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — seed data
// ─────────────────────────────────────────────────────────────────────────────

void BrainDb::seed_data()
{
    // Guard: only seed when runes table is empty.
    {
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM runes;", -1, &stmt, nullptr);
        int n = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (n > 0) return;
    }

    // ── 24 Elder/Extended Futhark runes with cognitive attributes ─────────────
    // Each rune is a node in the mycelial brain.  Categories map to agent roles:
    //   cognitive  → RuneTrustManager, WorkerAgent
    //   elemental  → RuneFusionEngine, HeartAgent
    //   gc         → GCAgent (disruption / pruning primitives)
    //   trust      → RuneTrustManager
    //   fusion     → RuneFusionEngine
    //   temporal   → HeartAgent (cycle/rhythm signals)
    //   physical   → base layer (raw signal processing)
    //   social     → NodeAgent, ChatAgent (peer exchange)
    raw_exec(R"sql(
BEGIN;

INSERT OR IGNORE INTO runes (name, glyph, category, level) VALUES
  -- Elder Futhark
  ('Fehu',    'ᚠ', 'elemental',  3),
  ('Uruz',    'ᚢ', 'physical',   4),
  ('Thurisaz','ᚦ', 'cognitive',  3),
  ('Ansuz',   'ᚨ', 'cognitive',  5),
  ('Raidho',  'ᚱ', 'temporal',   3),
  ('Kenaz',   'ᚲ', 'cognitive',  4),
  ('Gebo',    'ᚷ', 'social',     3),
  ('Wunjo',   'ᚹ', 'social',     3),
  ('Hagalaz', 'ᚺ', 'gc',         2),
  ('Nauthiz', 'ᚾ', 'gc',         2),
  ('Isa',     'ᛁ', 'gc',         1),
  ('Jera',    'ᛃ', 'temporal',   4),
  ('Eihwaz',  'ᛇ', 'trust',      4),
  ('Perthro', 'ᛈ', 'fusion',     3),
  ('Algiz',   'ᛉ', 'trust',      4),
  ('Sowilo',  'ᛊ', 'fusion',     5),
  ('Tiwaz',   'ᛏ', 'trust',      4),
  ('Berkano', 'ᛒ', 'elemental',  3),
  ('Ehwaz',   'ᛖ', 'social',     3),
  ('Mannaz',  'ᛗ', 'cognitive',  5),
  ('Laguz',   'ᛚ', 'elemental',  3),
  ('Ingwaz',  'ᛜ', 'fusion',     4),
  ('Dagaz',   'ᛞ', 'temporal',   5),
  ('Othalan', 'ᛟ', 'social',     3);

COMMIT;
    )sql");

    // ── 7 strategy runes (named cognitive pathways) ───────────────────────────
    // Guard: only seed when strategies table is empty.
    {
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM strategies;", -1, &stmt, nullptr);
        int n = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (n > 0) return;
    }

    struct StratSeed {
        const char* name;
        const char* description;
        const char* rune_ids;  // JSON array of rune names
        int         priority;
    };
    static constexpr StratSeed seeds[] = {
        {"seek_pattern",     "Cognitive pattern recognition pathway",
         R"(["Thurisaz","Kenaz","Mannaz"])",              3},
        {"transmit_wisdom",  "Knowledge and peer transmission pathway",
         R"(["Ansuz","Gebo","Ehwaz"])",                   2},
        {"root_binding",     "Energy anchoring and binding pathway",
         R"(["Fehu","Gebo","Eihwaz"])",                   2},
        {"temporal_scan",    "Cycle and time-based analysis pathway",
         R"(["Raidho","Jera","Dagaz"])",                  1},
        {"trust_weave",      "Trust reinforcement and protection pathway",
         R"(["Tiwaz","Algiz","Eihwaz"])",                 3},
        {"fusion_rite",      "Fusion catalyst — activates RuneFusionEngine",
         R"(["Perthro","Sowilo","Ingwaz"])",               4},
        {"gc_sweep",         "Cleanup protocol — activates GCAgent pruning",
         R"(["Hagalaz","Nauthiz","Isa"])",                 5},
    };

    sqlite3_stmt* ins = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT OR IGNORE INTO strategies (name, description, rune_ids, priority)"
        " VALUES (?,?,?,?);",
        -1, &ins, nullptr);

    raw_exec("BEGIN;");
    for (const auto& s : seeds) {
        sqlite3_reset(ins);
        sqlite3_bind_text(ins, 1, s.name,        -1, SQLITE_STATIC);
        sqlite3_bind_text(ins, 2, s.description, -1, SQLITE_STATIC);
        sqlite3_bind_text(ins, 3, s.rune_ids,    -1, SQLITE_STATIC);
        sqlite3_bind_int (ins, 4, s.priority);
        sqlite3_step(ins);
    }
    raw_exec("COMMIT;");
    sqlite3_finalize(ins);
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — nodes
// ─────────────────────────────────────────────────────────────────────────────

void BrainDb::upsert_node(const std::string& node_id,
                           const std::string& label,
                           const std::string& status)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO nodes (node_id, label, status)"
        " VALUES (?,?,?)"
        " ON CONFLICT(node_id) DO UPDATE SET"
        "   label=excluded.label, status=excluded.status,"
        "   last_seen=datetime('now');",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, node_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, label.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, status.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — agents
// ─────────────────────────────────────────────────────────────────────────────

void BrainDb::register_agent(const std::string& name, const std::string& type)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT OR IGNORE INTO agents (name, type) VALUES (?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::update_agent_tick(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE agents SET last_tick=datetime('now'), tick_count=tick_count+1,"
        " status='running' WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::update_agent_status(const std::string& name, const std::string& status)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE agents SET status=? WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, name.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::increment_agent_errors(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE agents SET error_count=error_count+1, status='error' WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — runes
// ─────────────────────────────────────────────────────────────────────────────

nlohmann::json BrainDb::all_runes(int limit, int offset)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,name,glyph,category,level,trust,usage_count,error_count,"
        "       ttl_seconds,active,created_at,updated_at"
        " FROM runes ORDER BY id LIMIT ? OFFSET ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, (limit > 0) ? limit : 0x7fffffff);
    sqlite3_bind_int(stmt, 2, offset);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        arr.push_back({
            {"id",          sqlite3_column_int(stmt,    0)},
            {"name",        text(1)},
            {"glyph",       text(2)},
            {"category",    text(3)},
            {"level",       sqlite3_column_int(stmt,    4)},
            {"trust",       sqlite3_column_double(stmt, 5)},
            {"usage_count", sqlite3_column_int(stmt,    6)},
            {"error_count", sqlite3_column_int(stmt,    7)},
            {"ttl_seconds", sqlite3_column_int(stmt,    8)},
            {"active",      sqlite3_column_int(stmt,    9)},
            {"created_at",  text(10)},
            {"updated_at",  text(11)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

nlohmann::json BrainDb::rune_by_name(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,name,glyph,category,level,trust,usage_count,error_count,"
        "       ttl_seconds,active,created_at,updated_at"
        " FROM runes WHERE name=? LIMIT 1;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    nlohmann::json obj = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        obj = {
            {"id",          sqlite3_column_int(stmt,    0)},
            {"name",        text(1)},
            {"glyph",       text(2)},
            {"category",    text(3)},
            {"level",       sqlite3_column_int(stmt,    4)},
            {"trust",       sqlite3_column_double(stmt, 5)},
            {"usage_count", sqlite3_column_int(stmt,    6)},
            {"error_count", sqlite3_column_int(stmt,    7)},
            {"ttl_seconds", sqlite3_column_int(stmt,    8)},
            {"active",      sqlite3_column_int(stmt,    9)},
            {"created_at",  text(10)},
            {"updated_at",  text(11)}
        };
    }
    sqlite3_finalize(stmt);
    return obj;
}

int BrainDb::insert_rune(const std::string& name, const std::string& glyph,
                          const std::string& category, int level, int ttl_seconds)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT OR IGNORE INTO runes (name,glyph,category,level,ttl_seconds)"
        " VALUES (?,?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, glyph.c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 4, level);
    sqlite3_bind_int (stmt, 5, ttl_seconds);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

void BrainDb::increment_rune_usage(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE runes SET usage_count=usage_count+1, updated_at=datetime('now')"
        " WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::increment_rune_errors(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE runes SET error_count=error_count+1, updated_at=datetime('now')"
        " WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::set_rune_trust(const std::string& name, double trust)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE runes SET trust=?, updated_at=datetime('now') WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, trust);
    sqlite3_bind_text  (stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::set_rune_active(const std::string& name, int active)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE runes SET active=?, updated_at=datetime('now') WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int (stmt, 1, active);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::set_rune_level(const std::string& name, int level)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE runes SET level=?, updated_at=datetime('now') WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int (stmt, 1, level);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — strategies
// ─────────────────────────────────────────────────────────────────────────────

nlohmann::json BrainDb::all_strategies(int limit, int offset)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,name,description,rune_ids,priority,active,usage_count,created_at"
        " FROM strategies ORDER BY priority DESC, id LIMIT ? OFFSET ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, (limit > 0) ? limit : 0x7fffffff);
    sqlite3_bind_int(stmt, 2, offset);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json rune_ids_parsed;
        try { rune_ids_parsed = nlohmann::json::parse(text(3)); }
        catch (...) { rune_ids_parsed = nlohmann::json::array(); }

        arr.push_back({
            {"id",          sqlite3_column_int(stmt, 0)},
            {"name",        text(1)},
            {"description", text(2)},
            {"rune_ids",    rune_ids_parsed},
            {"priority",    sqlite3_column_int(stmt, 4)},
            {"active",      sqlite3_column_int(stmt, 5)},
            {"usage_count", sqlite3_column_int(stmt, 6)},
            {"created_at",  text(7)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

nlohmann::json BrainDb::strategy_by_name(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,name,description,rune_ids,priority,active,usage_count,created_at"
        " FROM strategies WHERE name=? LIMIT 1;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    nlohmann::json obj = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json rune_ids_parsed;
        try { rune_ids_parsed = nlohmann::json::parse(text(3)); }
        catch (...) { rune_ids_parsed = nlohmann::json::array(); }
        obj = {
            {"id",          sqlite3_column_int(stmt, 0)},
            {"name",        text(1)},
            {"description", text(2)},
            {"rune_ids",    rune_ids_parsed},
            {"priority",    sqlite3_column_int(stmt, 4)},
            {"active",      sqlite3_column_int(stmt, 5)},
            {"usage_count", sqlite3_column_int(stmt, 6)},
            {"created_at",  text(7)}
        };
    }
    sqlite3_finalize(stmt);
    return obj;
}

int BrainDb::insert_strategy(const std::string& name, const std::string& description,
                               const nlohmann::json& rune_ids, int priority)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string rids = rune_ids.dump();
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT OR IGNORE INTO strategies (name,description,rune_ids,priority)"
        " VALUES (?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(),        -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, rids.c_str(),        -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 4, priority);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

void BrainDb::increment_strategy_usage(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE strategies SET usage_count=usage_count+1 WHERE name=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — node_events (async work queue)
// ─────────────────────────────────────────────────────────────────────────────

int BrainDb::enqueue_event(const std::string& source, const std::string& event_type,
                            const nlohmann::json& payload, int priority)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string p = payload.dump();
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO node_events (source,event_type,payload,priority)"
        " VALUES (?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, source.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, event_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, p.c_str(),          -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 4, priority);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::pending_events(int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    // Highest priority first, then oldest first (FIFO within same priority).
    sqlite3_prepare_v2(db_,
        "SELECT id,source,event_type,payload,priority,attempts,created_at"
        " FROM node_events WHERE processed=0"
        " ORDER BY priority DESC, created_at ASC LIMIT ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, limit);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json pl;
        try { pl = nlohmann::json::parse(text(3)); }
        catch (...) { pl = nlohmann::json::object(); }

        arr.push_back({
            {"id",         sqlite3_column_int(stmt, 0)},
            {"source",     text(1)},
            {"event_type", text(2)},
            {"payload",    pl},
            {"priority",   sqlite3_column_int(stmt, 4)},
            {"attempts",   sqlite3_column_int(stmt, 5)},
            {"created_at", text(6)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

void BrainDb::mark_processed(int event_id)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE node_events SET processed=1, processed_at=datetime('now')"
        " WHERE id=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, event_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void BrainDb::increment_event_attempts(int event_id)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE node_events SET attempts=attempts+1 WHERE id=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, event_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — trust ledger
// ─────────────────────────────────────────────────────────────────────────────

void BrainDb::record_trust(const std::string& subject, const std::string& subject_type,
                            double delta, const std::string& reason)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO trust_ledger (subject,subject_type,trust_delta,reason)"
        " VALUES (?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text  (stmt, 1, subject.c_str(),      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, subject_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, delta);
    sqlite3_bind_text  (stmt, 4, reason.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

nlohmann::json BrainDb::aggregate_trust(const std::string& subject_type)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT subject, SUM(trust_delta) AS total"
        " FROM trust_ledger WHERE subject_type=?"
        " GROUP BY subject ORDER BY total DESC;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, subject_type.c_str(), -1, SQLITE_TRANSIENT);

    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* s = sqlite3_column_text(stmt, 0);
        arr.push_back({
            {"subject", s ? reinterpret_cast<const char*>(s) : ""},
            {"total",   sqlite3_column_double(stmt, 1)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

int BrainDb::apply_trust_updates(const std::string& subject_type)
{
    // Get aggregated deltas then apply them as individual UPDATE statements so
    // we stay within prepared-statement discipline (no dynamic SQL).
    auto summary = aggregate_trust(subject_type);
    if (summary.empty()) return 0;

    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE runes SET trust=MAX(0.0, trust + ?),"
        " updated_at=datetime('now') WHERE name=?;",
        -1, &stmt, nullptr);

    int updated = 0;
    for (const auto& row : summary) {
        double delta = row["total"].get<double>();
        std::string subj = row["subject"].get<std::string>();
        sqlite3_reset(stmt);
        sqlite3_bind_double(stmt, 1, delta);
        sqlite3_bind_text  (stmt, 2, subj.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db_) > 0)
            ++updated;
    }
    sqlite3_finalize(stmt);
    return updated;
}

int BrainDb::quarantine_low_trust_runes(double threshold)
{
    // Collect names first, then deactivate — avoids dynamic WHERE clause.
    std::vector<std::string> targets;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        sqlite3_stmt* sel = nullptr;
        sqlite3_prepare_v2(db_,
            "SELECT name FROM runes WHERE trust < ? AND active=1;",
            -1, &sel, nullptr);
        sqlite3_bind_double(sel, 1, threshold);
        while (sqlite3_step(sel) == SQLITE_ROW) {
            const unsigned char* p = sqlite3_column_text(sel, 0);
            if (p) targets.emplace_back(reinterpret_cast<const char*>(p));
        }
        sqlite3_finalize(sel);
    }
    for (const auto& n : targets) set_rune_active(n, 0);
    return static_cast<int>(targets.size());
}

int BrainDb::level_up_high_trust_runes(double threshold)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    // Cap level at 10; consume 1.0 trust per level-up to balance the ledger.
    sqlite3_prepare_v2(db_,
        "UPDATE runes SET level=MIN(10,level+1), trust=trust-1.0,"
        " updated_at=datetime('now')"
        " WHERE trust >= ? AND active=1 AND level < 10;",
        -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, threshold);
    sqlite3_step(stmt);
    int n = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return n;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — fusion
// ─────────────────────────────────────────────────────────────────────────────

nlohmann::json BrainDb::fusion_candidates(double min_trust, int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    // Find active rune pairs not already fused, ranked by combined trust.
    sqlite3_prepare_v2(db_, R"sql(
        SELECT a.name, a.category, a.level, a.trust,
               b.name, b.category, b.level, b.trust
        FROM runes a
        JOIN runes b ON a.id < b.id
        WHERE a.active=1 AND b.active=1
          AND a.trust >= ? AND b.trust >= ?
          AND NOT EXISTS (
              SELECT 1 FROM fusion_log
              WHERE (input_a=a.name AND input_b=b.name)
                 OR (input_a=b.name AND input_b=a.name)
          )
        ORDER BY (a.trust + b.trust) DESC
        LIMIT ?;
    )sql", -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, min_trust);
    sqlite3_bind_double(stmt, 2, min_trust);
    sqlite3_bind_int   (stmt, 3, limit);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        arr.push_back({
            {"name_a",     text(0)},
            {"category_a", text(1)},
            {"level_a",    sqlite3_column_int(stmt,    2)},
            {"trust_a",    sqlite3_column_double(stmt, 3)},
            {"name_b",     text(4)},
            {"category_b", text(5)},
            {"level_b",    sqlite3_column_int(stmt,    6)},
            {"trust_b",    sqlite3_column_double(stmt, 7)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

bool BrainDb::already_fused(const std::string& a, const std::string& b)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT COUNT(*) FROM fusion_log"
        " WHERE (input_a=? AND input_b=?) OR (input_a=? AND input_b=?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, a.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, b.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, b.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, a.c_str(), -1, SQLITE_TRANSIENT);
    int n = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return n > 0;
}

int BrainDb::insert_fusion(const std::string& input_a, const std::string& input_b,
                            const std::string& output_name, double score)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO fusion_log (input_a,input_b,output_name,score) VALUES (?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text  (stmt, 1, input_a.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, input_b.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 3, output_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, score);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::fusion_log(int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,input_a,input_b,output_name,score,accepted,created_at"
        " FROM fusion_log ORDER BY id DESC LIMIT ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, limit);

    auto text = [&](int col) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, col);
        return p ? reinterpret_cast<const char*>(p) : "";
    };

    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        arr.push_back({
            {"id",          sqlite3_column_int(stmt,    0)},
            {"input_a",     text(1)},
            {"input_b",     text(2)},
            {"output_name", text(3)},
            {"score",       sqlite3_column_double(stmt, 4)},
            {"accepted",    sqlite3_column_int(stmt,    5)},
            {"created_at",  text(6)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — artifacts
// ─────────────────────────────────────────────────────────────────────────────

int BrainDb::insert_artifact(const std::string& type, const std::string& source,
                              const nlohmann::json& payload, int ttl_seconds)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string p = payload.dump();
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO artifacts (type,source,payload,ttl_seconds) VALUES (?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, source.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, p.c_str(),      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 4, ttl_seconds);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — audit
// ─────────────────────────────────────────────────────────────────────────────

void BrainDb::audit_log(const std::string& agent, const std::string& action,
                         const std::string& subject, const nlohmann::json& detail)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string d = detail.dump();
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO audit (agent,action,subject,detail) VALUES (?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, agent.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, action.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, subject.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, d.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — GC helpers
// ─────────────────────────────────────────────────────────────────────────────

int BrainDb::gc_expired_runes()
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    // Delete runes whose TTL has elapsed.  ttl_seconds=0 means immortal.
    sqlite3_prepare_v2(db_,
        "DELETE FROM runes"
        " WHERE ttl_seconds > 0"
        "   AND datetime(created_at, '+' || ttl_seconds || ' seconds')"
        "       < datetime('now');",
        -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int n = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return n;
}

int BrainDb::gc_expired_artifacts()
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "DELETE FROM artifacts"
        " WHERE datetime(created_at, '+' || ttl_seconds || ' seconds')"
        "       < datetime('now');",
        -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int n = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return n;
}

int BrainDb::gc_old_events(int days_old)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "DELETE FROM node_events WHERE processed=1"
        " AND datetime(created_at) < datetime('now', ? || ' days');",
        -1, &stmt, nullptr);
    // Bind as "-N days" (negative offset moves backward in time).
    std::string offset = "-" + std::to_string(days_old);
    sqlite3_bind_text(stmt, 1, offset.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int n = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return n;
}

int BrainDb::quarantine_high_error_runes(int error_threshold)
{
    std::vector<std::string> targets;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        sqlite3_stmt* sel = nullptr;
        sqlite3_prepare_v2(db_,
            "SELECT name FROM runes WHERE error_count >= ? AND active=1;",
            -1, &sel, nullptr);
        sqlite3_bind_int(sel, 1, error_threshold);
        while (sqlite3_step(sel) == SQLITE_ROW) {
            const unsigned char* p = sqlite3_column_text(sel, 0);
            if (p) targets.emplace_back(reinterpret_cast<const char*>(p));
        }
        sqlite3_finalize(sel);
    }
    for (const auto& n : targets) set_rune_active(n, 0);
    return static_cast<int>(targets.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — batched counts and size assessment
// ─────────────────────────────────────────────────────────────────────────────

AllCounts BrainDb::all_counts()
{
    std::lock_guard<std::mutex> lk(mtx_);
    // All nine counts in a single round-trip using scalar subqueries.
    static const char* sql =
        "SELECT"
        "  (SELECT COUNT(*) FROM nodes)                         AS n_nodes,"
        "  (SELECT COUNT(*) FROM agents)                        AS n_agents,"
        "  (SELECT COUNT(*) FROM runes)                         AS n_runes,"
        "  (SELECT COUNT(*) FROM strategies)                    AS n_strats,"
        "  (SELECT COUNT(*) FROM fusion_log)                    AS n_fusion,"
        "  (SELECT COUNT(*) FROM artifacts)                     AS n_artifacts,"
        "  (SELECT COUNT(*) FROM audit)                         AS n_audit,"
        "  (SELECT COUNT(*) FROM node_events WHERE processed=0) AS n_pending,"
        "  (SELECT COUNT(*) FROM trust_ledger)                  AS n_trust;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    AllCounts c{};
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        c.nodes          = sqlite3_column_int(stmt, 0);
        c.agents         = sqlite3_column_int(stmt, 1);
        c.runes          = sqlite3_column_int(stmt, 2);
        c.strategies     = sqlite3_column_int(stmt, 3);
        c.fusion_entries = sqlite3_column_int(stmt, 4);
        c.artifacts      = sqlite3_column_int(stmt, 5);
        c.audit_entries  = sqlite3_column_int(stmt, 6);
        c.pending_events = sqlite3_column_int(stmt, 7);
        c.trust_entries  = sqlite3_column_int(stmt, 8);
    }
    sqlite3_finalize(stmt);
    return c;
}

void BrainDb::record_size_assessment()
{
    // Fetch counts without holding the lock (all_counts acquires it internally).
    AllCounts c = all_counts();

    struct Entry { const char* name; int count; };
    const Entry entries[] = {
        {"nodes",          c.nodes},
        {"agents",         c.agents},
        {"runes",          c.runes},
        {"strategies",     c.strategies},
        {"fusion_log",     c.fusion_entries},
        {"artifacts",      c.artifacts},
        {"audit",          c.audit_entries},
        {"pending_events", c.pending_events},
        {"trust_ledger",   c.trust_entries},
    };

    // Insert all rows in a single transaction with one prepared statement.
    std::lock_guard<std::mutex> lk(mtx_);
    raw_exec("BEGIN;");
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO size_assessment (table_name, row_count) VALUES (?,?);",
        -1, &stmt, nullptr);
    for (const auto& e : entries) {
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, e.name,    -1, SQLITE_STATIC);
        sqlite3_bind_int (stmt, 2, e.count);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    raw_exec("COMMIT;");
}

// ─────────────────────────────────────────────────────────────────────────────
// AgentBase
// ─────────────────────────────────────────────────────────────────────────────

AgentBase::AgentBase(std::string name, std::string type,
                     BrainDb& db, EventBus& bus, std::atomic<bool>& shutdown)
    : name_(std::move(name)), type_(std::move(type)),
      db_(db), bus_(bus), shutdown_(shutdown)
{}

std::mutex& AgentBase::log_mutex()
{
    static std::mutex m;
    return m;
}

void AgentBase::log(const char* fmt, ...) const
{
    std::lock_guard<std::mutex> lk(log_mutex());
    std::va_list ap;
    va_start(ap, fmt);
    std::vprintf(fmt, ap);
    va_end(ap);
    std::fflush(stdout);
}

void AgentBase::emit(const std::string& type, nlohmann::json payload)
{
    bus_.post({name_, type, std::move(payload), utc_now()});
}

void AgentBase::run()
{
    db_.update_agent_status(name_, "running");
    log("[%s] started\n", name_.c_str());

    while (!shutdown_.load(std::memory_order_relaxed)) {
        int sleep_ms = 1000;
        try {
            sleep_ms = tick();
            ++tick_count_;
        } catch (const std::exception& e) {
            log("[%s] ERROR: %s\n", name_.c_str(), e.what());
            db_.increment_agent_errors(name_);
            emit("agent_error", {{"message", e.what()}});
        }

        // Sleep in 50 ms slices so shutdown is responsive.
        int elapsed = 0;
        while (elapsed < sleep_ms && !shutdown_.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            elapsed += 50;
        }
    }

    db_.update_agent_status(name_, "idle");
    log("[%s] stopped\n", name_.c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
// HeartAgent
// ─────────────────────────────────────────────────────────────────────────────

int HeartAgent::tick()
{
    auto c = db_.all_counts();

    // Update the local node record with current vitals.
    db_.upsert_node("local", "local_brain", "active");

    // Record a size-assessment snapshot every 12 heartbeats (~1 min).
    if (tick_count_ % 12 == 0)
        db_.record_size_assessment();

    db_.update_agent_tick(name_);
    emit("heartbeat", {
        {"nodes",          c.nodes},
        {"agents",         c.agents},
        {"runes",          c.runes},
        {"strategies",     c.strategies},
        {"fusion_entries", c.fusion_entries},
        {"artifacts",      c.artifacts},
        {"pending_events", c.pending_events},
        {"trust_entries",  c.trust_entries}
    });

    log("[Heart] tick #%d — runes=%d  pending=%d\n",
        tick_count_, c.runes, c.pending_events);
    return 5000;
}

// ─────────────────────────────────────────────────────────────────────────────
// WorkerAgent
// ─────────────────────────────────────────────────────────────────────────────

int WorkerAgent::tick()
{
    auto events = db_.pending_events(10);
    if (events.empty()) return 1000;   // idle back-off

    for (const auto& ev : events) {
        db_.increment_event_attempts(ev["id"].get<int>());
        try {
            handle(ev);
        } catch (const std::exception& e) {
            db_.audit_log(name_, "handle_error", ev.value("event_type", "?"),
                          {{"error", e.what()}, {"event", ev}});
        }
        db_.mark_processed(ev["id"].get<int>());
    }

    db_.update_agent_tick(name_);
    return 100;   // stay busy while there is work
}

void WorkerAgent::handle(const nlohmann::json& ev)
{
    const std::string type    = ev.value("event_type", "");
    const nlohmann::json& pl  = ev.contains("payload") ? ev["payload"]
                                                        : nlohmann::json::object();
    const int id              = ev.value("id", -1);

    // ── Dispatch table ──────────────────────────────────────────────────────
    // EXTEND: add new event_type branches here.
    // Pattern: process → emit result → audit_log.
    // ────────────────────────────────────────────────────────────────────────

    if (type == "heartbeat") {
        // HeartAgent owns its own state; Worker just acknowledges.
        emit("worker_ack", {{"event_id", id}, {"type", type}});

    } else if (type == "chat_request") {
        // EXTEND via ChatAgent: call LLM backend with pl["message"].
        // For now, stub-route to the ChatAgent queue.
        emit("worker_dispatch", {
            {"event_id", id}, {"type", type}, {"routed_to", "ChatAgent"}
        });
        db_.enqueue_event("Worker", "chat_route", pl, /*priority=*/1);
        db_.audit_log(name_, "dispatch", "ChatAgent", pl);

    } else if (type == "wallet_sign") {
        // EXTEND via WalletAgent: call wallet::sign() from wallet.hpp.
        emit("worker_dispatch", {
            {"event_id", id}, {"type", type}, {"routed_to", "WalletAgent"}
        });
        db_.audit_log(name_, "dispatch", "WalletAgent", pl);

    } else if (type == "strategy_execute") {
        std::string strat_name = pl.value("strategy", "");
        auto strat = db_.strategy_by_name(strat_name);
        if (!strat.is_null()) {
            db_.increment_strategy_usage(strat_name);
            // Mark every rune in the pathway as used.
            if (strat.contains("rune_ids") && strat["rune_ids"].is_array()) {
                for (const auto& r : strat["rune_ids"])
                    db_.increment_rune_usage(r.get<std::string>());
            }
            emit("strategy_activated", {{"strategy", strat}});
            db_.audit_log(name_, "strategy_execute", strat_name, strat);
        } else {
            emit("worker_error", {
                {"event_id", id}, {"error", "strategy_not_found"},
                {"strategy", strat_name}
            });
        }

    } else if (type == "rune_trust") {
        // Caller posts {subject, delta, reason} to nudge a rune's trust score.
        std::string subj   = pl.value("subject", "");
        double      delta  = pl.value("delta",   0.0);
        std::string reason = pl.value("reason",  "worker_event");
        if (!subj.empty()) {
            db_.record_trust(subj, "rune", delta, reason);
            emit("trust_recorded", {{"subject", subj}, {"delta", delta}});
        }

    } else {
        // Unknown event — log and emit a warning for observability.
        db_.audit_log(name_, "unknown_event", type, pl);
        emit("worker_unknown", {{"event_id", id}, {"type", type}});
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GCAgent
// ─────────────────────────────────────────────────────────────────────────────

int GCAgent::tick()
{
    int purged_runes     = db_.gc_expired_runes();
    int purged_artifacts = db_.gc_expired_artifacts();
    int purged_events    = db_.gc_old_events(1);           // older than 24h
    int quarantined      = db_.quarantine_high_error_runes(10);

    db_.record_size_assessment();
    db_.update_agent_tick(name_);

    if (purged_runes + purged_artifacts + purged_events + quarantined > 0) {
        db_.insert_artifact("gc_pruning", name_, {
            {"purged_runes",     purged_runes},
            {"purged_artifacts", purged_artifacts},
            {"purged_events",    purged_events},
            {"quarantined",      quarantined}
        });
        db_.audit_log(name_, "gc_sweep", "all", {
            {"purged_runes",     purged_runes},
            {"purged_artifacts", purged_artifacts},
            {"purged_events",    purged_events},
            {"quarantined",      quarantined}
        });
    }

    emit("gc_complete", {
        {"purged_runes",     purged_runes},
        {"purged_artifacts", purged_artifacts},
        {"purged_events",    purged_events},
        {"quarantined",      quarantined}
    });

    log("[GC] tick #%d — pruned=%d  artifacts=%d  events=%d  quarantined=%d\n",
        tick_count_, purged_runes, purged_artifacts, purged_events, quarantined);
    return 30000;
}

// ─────────────────────────────────────────────────────────────────────────────
// RuneTrustManager
// ─────────────────────────────────────────────────────────────────────────────

int RuneTrustManager::tick()
{
    // Apply accumulated ledger deltas to rune trust scores.
    int updated     = db_.apply_trust_updates("rune");
    // Quarantine runes whose trust has fallen below the floor.
    int quarantined = db_.quarantine_low_trust_runes(0.1);
    // Level-up runes that have accumulated surplus trust.
    int leveled     = db_.level_up_high_trust_runes(3.0);

    db_.update_agent_tick(name_);

    if (updated + quarantined + leveled > 0) {
        db_.audit_log(name_, "trust_cycle", "runes", {
            {"updated",     updated},
            {"quarantined", quarantined},
            {"leveled_up",  leveled}
        });
        emit("trust_cycle", {
            {"subjects_updated", updated},
            {"quarantined",      quarantined},
            {"leveled_up",       leveled}
        });
        log("[Trust] updated=%d  quarantined=%d  leveled=%d\n",
            updated, quarantined, leveled);
    }
    return 10000;
}

// ─────────────────────────────────────────────────────────────────────────────
// RuneFusionEngine
// ─────────────────────────────────────────────────────────────────────────────

std::string RuneFusionEngine::fused_category(const std::string& a, const std::string& b)
{
    // Category fusion rules — determines the cognitive domain of the offspring.
    // Extend this map to define new emergent category types.
    static const std::map<std::pair<std::string,std::string>, std::string> rules = {
        {{"cognitive",  "elemental"},  "arcane"},
        {{"elemental",  "cognitive"},  "arcane"},
        {{"temporal",   "cognitive"},  "prophetic"},
        {{"cognitive",  "temporal"},   "prophetic"},
        {{"trust",      "fusion"},     "sovereign"},
        {{"fusion",     "trust"},      "sovereign"},
        {{"social",     "trust"},      "covenant"},
        {{"trust",      "social"},     "covenant"},
        {{"physical",   "elemental"},  "primal"},
        {{"elemental",  "physical"},   "primal"},
        {{"gc",         "cognitive"},  "sentinel"},
        {{"cognitive",  "gc"},         "sentinel"},
        {{"temporal",   "elemental"},  "cyclical"},
        {{"elemental",  "temporal"},   "cyclical"},
        {{"fusion",     "social"},     "mythic"},
        {{"social",     "fusion"},     "mythic"},
    };
    auto it = rules.find({a, b});
    return (it != rules.end()) ? it->second : "composite";
}

std::string RuneFusionEngine::fused_name(const std::string& a, const std::string& b)
{
    // Use up to 4 chars from each parent name.
    auto prefix_a = a.substr(0, std::min<size_t>(4, a.size()));
    auto prefix_b = b.substr(0, std::min<size_t>(4, b.size()));
    // Lower-case for consistency.
    for (auto& c : prefix_a) c = static_cast<char>(std::tolower(c));
    for (auto& c : prefix_b) c = static_cast<char>(std::tolower(c));
    return prefix_a + prefix_b + "_f";
}

std::string RuneFusionEngine::fused_glyph(const std::string& a, const std::string& b)
{
    // Compose a synthetic glyph by concatenating the parents' glyphs.
    // The DB layer will look up the glyphs — we receive rune names here, so
    // brain_main.cpp uses rune_by_name() to fetch them before calling tick().
    // For the inline path, simply join the two name initials in angle brackets.
    std::string g = "<";
    g += (a.empty() ? '?' : static_cast<char>(std::toupper(a[0])));
    g += '+';
    g += (b.empty() ? '?' : static_cast<char>(std::toupper(b[0])));
    g += '>';
    return g;
}

int RuneFusionEngine::tick()
{
    auto candidates = db_.fusion_candidates(/*min_trust=*/1.0, /*limit=*/3);
    int fused = 0;

    for (const auto& pair : candidates) {
        std::string name_a     = pair["name_a"].get<std::string>();
        std::string name_b     = pair["name_b"].get<std::string>();
        std::string cat_a      = pair["category_a"].get<std::string>();
        std::string cat_b      = pair["category_b"].get<std::string>();
        double      trust_a    = pair["trust_a"].get<double>();
        double      trust_b    = pair["trust_b"].get<double>();
        int         level_a    = pair["level_a"].get<int>();
        int         level_b    = pair["level_b"].get<int>();

        // Double-check (candidates query guards this, but be defensive).
        if (db_.already_fused(name_a, name_b)) continue;

        std::string out_name  = fused_name(name_a, name_b);
        std::string out_glyph = fused_glyph(name_a, name_b);
        std::string out_cat   = fused_category(cat_a, cat_b);
        int         out_level = (level_a + level_b) / 2;
        double      score     = ((trust_a + trust_b) / 2.0) *
                                ((level_a + level_b) / 2.0);

        db_.insert_rune(out_name, out_glyph, out_cat, out_level);
        db_.insert_fusion(name_a, name_b, out_name, score);

        // Give the fused rune an initial trust boost seeded from its parents.
        db_.record_trust(out_name, "rune", score * 0.1, "fusion_birth");

        db_.insert_artifact("fusion_residue", name_, {
            {"input_a", name_a}, {"input_b", name_b},
            {"output",  out_name}, {"score", score}
        });
        db_.audit_log(name_, "fuse", out_name, {
            {"input_a", name_a}, {"input_b", name_b}, {"score", score}
        });
        ++fused;
    }

    if (fused > 0) {
        db_.update_agent_tick(name_);
        emit("fusion_cycle", {{"fusions", fused}});
        log("[Fusion] tick #%d — %d new rune(s) synthesised\n",
            tick_count_, fused);
    }
    return 20000;
}

// ─────────────────────────────────────────────────────────────────────────────
// Stub agents
// ─────────────────────────────────────────────────────────────────────────────

int ChatAgent::tick()
{
    // EXTEND: poll node_events WHERE event_type='chat_route' LIMIT 5,
    // call LLM backend, post reply as "chat_response" event, audit_log result.
    emit("agent_idle", {{"agent", name_}});
    return 5000;
}

int WalletAgent::tick()
{
    // EXTEND: poll node_events WHERE event_type='wallet_sign' LIMIT 5,
    // call wallet::sign() from src/wallet/wallet.hpp, emit "wallet_signature".
    emit("agent_idle", {{"agent", name_}});
    return 5000;
}

int NodeAgent::tick()
{
    // EXTEND: iterate nodes table WHERE status != 'local',
    // HTTP GET /brain/health on each peer, update last_seen + trust.
    emit("agent_idle", {{"agent", name_}});
    return 10000;
}

int TorrentAgent::tick()
{
    // EXTEND: manage torrent_piece artifacts, enqueue "torrent_piece" events,
    // track piece availability and replication across the node mesh.
    emit("agent_idle", {{"agent", name_}});
    return 30000;
}

int EdgeAgent::tick()
{
    // EXTEND: handle low-latency edge tasks (QR decode, image hash, etc.),
    // post results as "edge_result" events for real-time visualisation layers.
    emit("agent_idle", {{"agent", name_}});
    return 10000;
}
