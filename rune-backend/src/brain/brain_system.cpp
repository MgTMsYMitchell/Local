/*
  PROJECT: QRrune / Cognitive Node Brain
  FILE: brain_system.cpp

  INTENT FOR GITHUB COPILOT CHAT / AI:
  - This file is the canonical reference for:
      * DB schema (SQLite, WAL): nodes, agents, runes, strategies, fusion_log,
        artifacts, audit, node_events, trust_ledger, size_assessment
      * Seed data: 24 Elder/Extended Futhark runes, 7 strategy runes
      * Core agents: Heart (node_status), Worker (strategy + trust jobs),
        GC (TTL, quarantine, pruning), RuneTrustManager (trust/TTL updates),
        RuneFusionEngine (simple pairwise fusion)
      * Specialized agents: ChatAgent (LLM), WalletAgent (Ed25519),
        NodeAgent (peer health), TorrentAgent (piece tracking),
        EdgeAgent (hashing/edge compute), HousekeepingAgent, StrategyAgent,
        EpicRuneAgent, OverwatchAgent, LibrarianAgent, LoadSimulatorAgent
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

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <chrono>

#include <httplib.h>
#include "../wallet/wallet.hpp"

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

-- symbols: 14-dimensional QFS symbol objects
CREATE TABLE IF NOT EXISTS symbols (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    radical        TEXT    NOT NULL DEFAULT '',
    layer          TEXT    NOT NULL DEFAULT 'SYNTHETIC',
    sem_x          REAL    NOT NULL DEFAULT 0.0,
    sem_y          REAL    NOT NULL DEFAULT 0.0,
    sem_z          REAL    NOT NULL DEFAULT 0.0,
    color_h        REAL    NOT NULL DEFAULT 0.0,
    color_s        REAL    NOT NULL DEFAULT 0.0,
    color_b        REAL    NOT NULL DEFAULT 0.0,
    temporal_phase REAL    NOT NULL DEFAULT 0.0,
    affinity_mask  INTEGER NOT NULL DEFAULT 0,
    mutation_index INTEGER NOT NULL DEFAULT 0,
    stroke_count   INTEGER NOT NULL DEFAULT 0,
    fractal_depth  INTEGER NOT NULL DEFAULT 1,
    compression_q  TEXT    NOT NULL DEFAULT 'LOSSLESS',
    payload        TEXT    NOT NULL DEFAULT '{}',
    checksum       TEXT    NOT NULL DEFAULT '',
    version        INTEGER NOT NULL DEFAULT 1,
    created_at     TEXT    DEFAULT (datetime('now')),
    updated_at     TEXT    DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS idx_symbols_radical   ON symbols(radical);
CREATE INDEX IF NOT EXISTS idx_symbols_layer     ON symbols(layer);
CREATE INDEX IF NOT EXISTS idx_symbols_temporal  ON symbols(temporal_phase);
CREATE INDEX IF NOT EXISTS idx_symbols_affinity  ON symbols(affinity_mask);
CREATE INDEX IF NOT EXISTS idx_symbols_mutation  ON symbols(mutation_index);

-- knowledge: MemorySubstrate — radical-indexed knowledge entries
CREATE TABLE IF NOT EXISTS knowledge (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    radical    TEXT    NOT NULL,
    layer      TEXT    NOT NULL DEFAULT 'SYNTHETIC',
    entry      TEXT    NOT NULL DEFAULT '{}',
    sem_x      REAL    NOT NULL DEFAULT 0.0,
    sem_y      REAL    NOT NULL DEFAULT 0.0,
    sem_z      REAL    NOT NULL DEFAULT 0.0,
    created_at TEXT    DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS idx_knowledge_radical ON knowledge(radical);

-- agent_checkpoints: recovery snapshots per agent
CREATE TABLE IF NOT EXISTS agent_checkpoints (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    agent_name  TEXT    NOT NULL,
    state       TEXT    NOT NULL DEFAULT '{}',
    created_at  TEXT    DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS idx_checkpoints_agent ON agent_checkpoints(agent_name);

-- radicals: CR-01 through CR-60 atomic component registry (RQ^R2 §5.1)
CREATE TABLE IF NOT EXISTS radicals (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    code        TEXT    NOT NULL UNIQUE,  -- "CR-01" … "CR-60"
    name        TEXT    NOT NULL,
    tier        INTEGER NOT NULL,         -- 1-6
    domain      TEXT    NOT NULL DEFAULT '',
    sem_x       REAL    NOT NULL DEFAULT 0.0,
    sem_y       REAL    NOT NULL DEFAULT 0.0,
    sem_z       REAL    NOT NULL DEFAULT 0.0,
    color_hue   REAL    NOT NULL DEFAULT 0.0,
    ac_ratio    REAL    NOT NULL DEFAULT 0.5,
    description TEXT    NOT NULL DEFAULT ''
);
CREATE INDEX IF NOT EXISTS idx_radicals_code ON radicals(code);
CREATE INDEX IF NOT EXISTS idx_radicals_tier ON radicals(tier);

-- dreams: unverified/hallucinated fragments (RQ^R2 §2.3, §3)
CREATE TABLE IF NOT EXISTS dreams (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    raw_input           TEXT    NOT NULL,
    fragment_data       TEXT,
    radicals_guess      TEXT    NOT NULL DEFAULT '[]',
    sem_x               REAL,
    sem_y               REAL,
    sem_z               REAL,
    confidence          REAL    NOT NULL DEFAULT 0.0,
    source_agent        TEXT    NOT NULL DEFAULT '',
    source_context      TEXT,
    dream_type          TEXT    NOT NULL DEFAULT 'fragment',
    consolidation_count INTEGER DEFAULT 0,
    last_consolidated   TEXT,
    promoted_to         INTEGER,
    rejected_at         TEXT,
    rejection_reason    TEXT,
    created_at          TEXT    DEFAULT (datetime('now')),
    expires_at          TEXT
);
CREATE INDEX IF NOT EXISTS idx_dreams_type       ON dreams(dream_type);
CREATE INDEX IF NOT EXISTS idx_dreams_confidence ON dreams(confidence);
CREATE INDEX IF NOT EXISTS idx_dreams_expires    ON dreams(expires_at);
CREATE INDEX IF NOT EXISTS idx_dreams_promoted   ON dreams(promoted_to);

-- theories: promoted dreams under active evaluation (RQ^R2 §2.4, §7)
CREATE TABLE IF NOT EXISTS theories (
    id                     INTEGER PRIMARY KEY AUTOINCREMENT,
    dream_origin           INTEGER NOT NULL,
    glyph_data             TEXT    NOT NULL DEFAULT '{}',
    radicals               TEXT    NOT NULL DEFAULT '[]',
    sem_x                  REAL    NOT NULL DEFAULT 0.0,
    sem_y                  REAL    NOT NULL DEFAULT 0.0,
    sem_z                  REAL    NOT NULL DEFAULT 0.0,
    fractal_depth          INTEGER NOT NULL DEFAULT 1,
    compression_q          REAL    NOT NULL DEFAULT 0.5,
    hypothesis             TEXT    NOT NULL,
    supporting_evidence    TEXT,
    contradicting_evidence TEXT,
    test_count             INTEGER DEFAULT 0,
    pass_count             INTEGER DEFAULT 0,
    fail_count             INTEGER DEFAULT 0,
    trust_score            REAL    NOT NULL DEFAULT 0.3,
    promotion_threshold    REAL    DEFAULT 0.75,
    status                 TEXT    NOT NULL DEFAULT 'active',
    created_at             TEXT    DEFAULT (datetime('now')),
    last_tested            TEXT,
    promoted_at            TEXT
);
CREATE INDEX IF NOT EXISTS idx_theories_status ON theories(status);
CREATE INDEX IF NOT EXISTS idx_theories_trust  ON theories(trust_score);

-- tendrils: mycelium connections between symbols (RQ^R2 §14)
CREATE TABLE IF NOT EXISTS tendrils (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    source_symbol  INTEGER NOT NULL,
    target_symbol  INTEGER NOT NULL,
    weight         REAL    NOT NULL DEFAULT 0.5,
    traffic_count  INTEGER DEFAULT 0,
    last_traversed TEXT,
    created_at     TEXT    DEFAULT (datetime('now')),
    tendril_type   TEXT    NOT NULL DEFAULT 'association'
);
CREATE INDEX IF NOT EXISTS idx_tendrils_source ON tendrils(source_symbol);
CREATE INDEX IF NOT EXISTS idx_tendrils_target ON tendrils(target_symbol);
CREATE INDEX IF NOT EXISTS idx_tendrils_weight ON tendrils(weight);

    )sql");

    // ── RQ^R2 symbol migrations — add new columns to existing symbols table.
    // Each ALTER TABLE is attempted individually; "duplicate column name" is
    // expected on databases that already have the column and is silently ignored.
    auto try_alter = [this](const char* sql) {
        char* err = nullptr;
        sqlite3_exec(db_, sql, nullptr, nullptr, &err);
        if (err) sqlite3_free(err);  // ignore SQLITE_ERROR for existing columns
    };
    try_alter("ALTER TABLE symbols ADD COLUMN trust_state    TEXT    NOT NULL DEFAULT 'dream'");
    try_alter("ALTER TABLE symbols ADD COLUMN trust_score    REAL    NOT NULL DEFAULT 0.0");
    try_alter("ALTER TABLE symbols ADD COLUMN ac_ratio       REAL    NOT NULL DEFAULT 0.5");
    try_alter("ALTER TABLE symbols ADD COLUMN domain         TEXT");
    try_alter("ALTER TABLE symbols ADD COLUMN halo_json      TEXT    NOT NULL DEFAULT '{}'");
    try_alter("ALTER TABLE symbols ADD COLUMN lineage_parent INTEGER");
    try_alter("ALTER TABLE symbols ADD COLUMN dream_source   INTEGER");
    try_alter("ALTER TABLE symbols ADD COLUMN promoted_at    TEXT");
    try_alter("ALTER TABLE symbols ADD COLUMN accessed_at    TEXT");
    try_alter("ALTER TABLE symbols ADD COLUMN access_count   INTEGER DEFAULT 0");
    try_alter("ALTER TABLE symbols ADD COLUMN decay_score    REAL    DEFAULT 1.0");
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

    // ── 60 core radicals (CR-01 → CR-60) — seed only when table is empty ─────
    {
        sqlite3_stmt* chk = nullptr;
        sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM radicals;", -1, &chk, nullptr);
        int nr = 0;
        if (sqlite3_step(chk) == SQLITE_ROW) nr = sqlite3_column_int(chk, 0);
        sqlite3_finalize(chk);
        if (nr == 0) {
            // code, name, tier, domain, sem_x, sem_y, sem_z, color_hue, ac_ratio, description
            raw_exec(R"sql(
BEGIN;
INSERT OR IGNORE INTO radicals
    (code,    name,       tier, domain,  sem_x,  sem_y,  sem_z, color_hue, ac_ratio, description)
VALUES
  -- TIER 1 — ELEMENTAL (World-Entity Ontology, ac_ratio near 0.5, sem_z=0.5 concrete)
  ('CR-01','Tree',     1,'elemental', 0.2, 0.1, 0.5,  120.0, 0.55, 'File systems, ASTs, hierarchies'),
  ('CR-02','Bug',      1,'elemental', 0.3, 0.4, 0.6,   45.0, 0.50, 'Microservices, worker threads, swarm'),
  ('CR-03','Bird',     1,'elemental',-0.2, 0.3, 0.7,  200.0, 0.35, 'Message brokers, event dispatchers'),
  ('CR-04','Fungi',    1,'elemental',-0.1,-0.2, 0.4,  280.0, 0.25, 'Distributed caches, gossip protocols'),
  ('CR-05','Vine',     1,'elemental', 0.0, 0.5, 0.5,  150.0, 0.30, 'Dependency injection, middleware'),
  ('CR-06','Flower',   1,'elemental', 0.1, 0.6, 0.6,  330.0, 0.25, 'UI components, API surfaces'),
  ('CR-07','Soil',     1,'elemental', 0.4,-0.1, 0.8,   30.0, 0.70, 'Databases, persistent storage'),
  ('CR-08','Water',    1,'elemental',-0.3, 0.0, 0.3,  210.0, 0.15, 'Streams, pipelines, ETL'),
  ('CR-09','Fire',     1,'elemental', 0.0,-0.3, 0.5,   10.0, 0.65, 'Compilers, optimizers, GC'),
  ('CR-10','Wind',     1,'elemental',-0.5, 0.2, 0.4,  180.0, 0.10, 'Network I/O, broadcast, pub/sub'),
  -- TIER 2 — STRUCTURAL (Angular Domain, ac_ratio near 0.85)
  ('CR-11','Wall',     2,'structural', 0.8, 0.0, 0.9,   30.0, 0.90, 'Boundary, containment, isolation'),
  ('CR-12','Frame',    2,'structural', 0.7,-0.1, 0.8,   40.0, 0.88, 'Container, scaffold, skeleton'),
  ('CR-13','Path',     2,'structural', 0.5, 0.0, 0.5,   45.0, 0.80, 'Route, sequence, traversal'),
  ('CR-14','Bridge',   2,'structural', 0.6, 0.3, 0.6,   50.0, 0.75, 'Interface, adapter, connection'),
  ('CR-15','Gate',     2,'structural', 0.7,-0.2, 0.8,   40.0, 0.85, 'Auth, access control, filter'),
  ('CR-16','Tower',    2,'structural', 0.9,-0.1, 0.9,   35.0, 0.92, 'Service, daemon, long-running process'),
  ('CR-17','Root',     2,'structural', 0.6,-0.4, 0.9,   30.0, 0.80, 'Foundation, origin, anchor'),
  ('CR-18','Branch',   2,'structural', 0.4, 0.1, 0.7,   90.0, 0.70, 'Fork, subtree, divergence'),
  ('CR-19','Grid',     2,'structural', 0.9, 0.0, 0.9,   45.0, 0.95, 'Matrix, index, regular structure'),
  ('CR-20','Knot',     2,'structural', 0.7, 0.2, 0.7,   60.0, 0.82, 'Binding, coupling, entanglement'),
  -- TIER 3 — PROCESSUAL (Curved Domain, ac_ratio near 0.15)
  ('CR-21','Flow',     3,'processual',-0.4, 0.1, 0.2,  210.0, 0.10, 'Pipeline, stream, continuous process'),
  ('CR-22','Spiral',   3,'processual',-0.3,-0.1, 0.1,  260.0, 0.05, 'Recursion, iteration, self-similarity'),
  ('CR-23','Wave',     3,'processual',-0.5, 0.2, 0.2,  200.0, 0.08, 'Oscillation, signal, periodic'),
  ('CR-24','Bloom',    3,'processual',-0.2, 0.4, 0.3,  330.0, 0.12, 'Expansion, growth, emergence'),
  ('CR-25','Wilt',     3,'processual',-0.1,-0.3, 0.2,   30.0, 0.10, 'Decay, degradation, entropy'),
  ('CR-26','Merge',    3,'processual',-0.3, 0.3, 0.3,  280.0, 0.15, 'Join, consolidate, fuse'),
  ('CR-27','Split',    3,'processual', 0.2, 0.0, 0.4,   60.0, 0.40, 'Fork, divide, partition'),
  ('CR-28','Twist',    3,'processual',-0.2, 0.1, 0.3,  240.0, 0.20, 'Transform, distort, rotate'),
  ('CR-29','Echo',     3,'processual',-0.4, 0.0, 0.1,  180.0, 0.05, 'Reflection, resonance, repetition'),
  ('CR-30','Drift',    3,'processual',-0.6, 0.1, 0.0,  200.0, 0.08, 'Gradual shift, divergence over time'),
  -- TIER 4 — COGNITIVE (Brain Model, sem_z near -0.5 abstract)
  ('CR-31','Perceive', 4,'cognitive',-0.2, 0.0,-0.3, 200.0, 0.30, 'Sensory input, detection, observation'),
  ('CR-32','Remember', 4,'cognitive', 0.1,-0.1,-0.5, 120.0, 0.45, 'Recall, retrieval, persistence'),
  ('CR-33','Decide',   4,'cognitive', 0.5, 0.0,-0.4, 60.0,  0.75, 'Branch selection, policy, judgment'),
  ('CR-34','Create',   4,'cognitive',-0.1, 0.2,-0.6, 300.0, 0.25, 'Generation, synthesis, novelty'),
  ('CR-35','Compare',  4,'cognitive', 0.6, 0.0,-0.3, 45.0,  0.80, 'Diff, evaluation, metric'),
  ('CR-36','Abstract', 4,'cognitive',-0.3,-0.1,-0.8, 270.0, 0.20, 'Generalise, distil, conceptualise'),
  ('CR-37','Embody',   4,'cognitive', 0.4, 0.1,-0.2, 30.0,  0.65, 'Instantiate, materialise, ground'),
  ('CR-38','Dream',    4,'cognitive',-0.5, 0.3,-0.9, 260.0, 0.10, 'Hypothesise, hallucinate, speculate'),
  ('CR-39','Focus',    4,'cognitive', 0.7,-0.2,-0.4, 50.0,  0.85, 'Attention, filter, priority'),
  ('CR-40','Release',  4,'cognitive',-0.2, 0.0,-0.3, 180.0, 0.15, 'Emit, publish, let go'),
  -- TIER 5 — RELATIONAL (Movement/Relationship, sem_y near 0.5)
  ('CR-41','Parent',   5,'relational', 0.3, 0.7, 0.2, 30.0,  0.60, 'Owner, creator, authority'),
  ('CR-42','Child',    5,'relational', 0.2, 0.8, 0.3, 120.0, 0.45, 'Derived, subordinate, spawned'),
  ('CR-43','Sibling',  5,'relational', 0.0, 0.9, 0.2, 180.0, 0.50, 'Peer, co-equal, parallel'),
  ('CR-44','Bond',     5,'relational', 0.4, 0.6, 0.4, 240.0, 0.55, 'Link, coupling, dependency'),
  ('CR-45','Guard',    5,'relational', 0.8, 0.5, 0.6, 50.0,  0.88, 'Protect, authenticate, validate'),
  ('CR-46','Feed',     5,'relational',-0.1, 0.6, 0.3, 150.0, 0.25, 'Supply, provide, nourish'),
  ('CR-47','Compete',  5,'relational', 0.5, 0.4, 0.5, 10.0,  0.70, 'Contend, race, conflict'),
  ('CR-48','Observe',  5,'relational',-0.3, 0.5, 0.1, 200.0, 0.20, 'Monitor, watch, measure'),
  ('CR-49','Carry',    5,'relational', 0.3, 0.7, 0.5, 80.0,  0.60, 'Transport, propagate, relay'),
  ('CR-50','Anchor',   5,'relational', 0.7, 0.3, 0.8, 30.0,  0.90, 'Pin, stabilise, caern-node'),
  -- TIER 6 — META (System-level, sem_z near -1.0 most abstract)
  ('CR-51','Agent',    6,'meta',  0.2, 0.0,-0.8, 45.0,  0.55, 'Autonomous actor, cognitive agent'),
  ('CR-52','Message',  6,'meta',-0.1, 0.3,-0.7, 180.0, 0.20, 'Event, signal, datum'),
  ('CR-53','Rule',     6,'meta',  0.9,-0.1,-0.8, 50.0,  0.95, 'Constraint, law, policy'),
  ('CR-54','Error',    6,'meta',  0.0,-0.5,-0.9, 10.0,  0.50, 'Fault, exception, failure'),
  ('CR-55','Time',     6,'meta',-0.2, 0.0,-1.0, 200.0, 0.30, 'Temporal marker, timestamp, phase'),
  ('CR-56','Space',    6,'meta', 0.1, 0.1,-0.9, 180.0, 0.50, 'Coordinate, region, extent'),
  ('CR-57','Truth',    6,'meta', 0.8, 0.0,-1.0, 60.0,  0.80, 'Axiom, ground-truth, verified fact'),
  ('CR-58','Unknown',  6,'meta',-0.8, 0.0,-1.0, 270.0, 0.10, 'Unknown, gap, unresolved'),
  ('CR-59','Null',     6,'meta', 0.0, 0.0,-1.0,   0.0, 0.50, 'Empty, void, zero'),
  ('CR-60','Infinity', 6,'meta',-0.5, 0.0,-1.0, 260.0, 0.05, 'Unbounded, recursive, self-referential');
COMMIT;
            )sql");
        }
    }
}
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
// BrainDb — extended queries and new substrate methods
// ─────────────────────────────────────────────────────────────────────────────

nlohmann::json BrainDb::nodes_list()
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,node_id,label,status,trust,last_seen FROM nodes"
        " WHERE node_id != 'local' ORDER BY id ASC;",
        -1, &stmt, nullptr);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        arr.push_back({
            {"id",        sqlite3_column_int(stmt, 0)},
            {"node_id",   txt(1)},
            {"label",     txt(2)},
            {"status",    txt(3)},
            {"trust",     sqlite3_column_double(stmt, 4)},
            {"last_seen", txt(5)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

nlohmann::json BrainDb::pending_events_of_type(const std::string& type, int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,source,event_type,payload,priority FROM node_events"
        " WHERE processed=0 AND event_type=?"
        " ORDER BY priority DESC, id ASC LIMIT ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, limit);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        nlohmann::json pl;
        try { pl = nlohmann::json::parse(txt(3)); }
        catch (...) { pl = nlohmann::json::object(); }
        arr.push_back({
            {"id",         sqlite3_column_int(stmt, 0)},
            {"source",     txt(1)},
            {"event_type", txt(2)},
            {"payload",    pl},
            {"priority",   sqlite3_column_int(stmt, 4)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

nlohmann::json BrainDb::all_agents()
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,name,type,status,tick_count,error_count,last_tick"
        " FROM agents ORDER BY id ASC;",
        -1, &stmt, nullptr);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        arr.push_back({
            {"id",          sqlite3_column_int(stmt, 0)},
            {"name",        txt(1)},
            {"type",        txt(2)},
            {"status",      txt(3)},
            {"tick_count",  sqlite3_column_int(stmt, 4)},
            {"error_count", sqlite3_column_int(stmt, 5)},
            {"last_tick",   txt(6)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

int BrainDb::archive_old_audit(int days_old)
{
    struct Row { int id; std::string agent, action, subject, detail, created_at; };
    std::vector<Row> rows;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        sqlite3_stmt* stmt = nullptr;
        // Use a negative offset bound as text, e.g. "-30 days"
        std::string offset = "-" + std::to_string(days_old) + " days";
        sqlite3_prepare_v2(db_,
            "SELECT id,agent,action,subject,detail,created_at FROM audit"
            " WHERE created_at < datetime('now', ?) LIMIT 200;",
            -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, offset.c_str(), -1, SQLITE_TRANSIENT);
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        while (sqlite3_step(stmt) == SQLITE_ROW)
            rows.push_back({sqlite3_column_int(stmt,0),txt(1),txt(2),txt(3),txt(4),txt(5)});
        sqlite3_finalize(stmt);
    }
    for (const auto& r : rows) {
        insert_artifact("audit_archive", r.agent, {
            {"orig_id",    r.id},
            {"agent",      r.agent},
            {"action",     r.action},
            {"subject",    r.subject},
            {"detail",     r.detail},
            {"created_at", r.created_at}
        }, 0);
        {
            std::lock_guard<std::mutex> lk(mtx_);
            sqlite3_stmt* del = nullptr;
            sqlite3_prepare_v2(db_, "DELETE FROM audit WHERE id=?;", -1, &del, nullptr);
            sqlite3_bind_int(del, 1, r.id);
            sqlite3_step(del);
            sqlite3_finalize(del);
        }
    }
    return static_cast<int>(rows.size());
}

int BrainDb::insert_symbol(const nlohmann::json& doc)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO symbols (radical,layer,sem_x,sem_y,sem_z,color_h,color_s,color_b,"
        "temporal_phase,affinity_mask,mutation_index,stroke_count,fractal_depth,"
        "compression_q,payload,checksum,version,"
        "trust_state,trust_score,ac_ratio,domain,halo_json,"
        "lineage_parent,dream_source,decay_score)"
        " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);",
        -1, &stmt, nullptr);
    auto s = [&](const std::string& k, const std::string& def="") {
        return doc.value(k, def);
    };
    sqlite3_bind_text  (stmt,  1, s("radical").c_str(),            -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt,  2, s("layer","SYNTHETIC").c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,  3, doc.value("sem_x", 0.0));
    sqlite3_bind_double(stmt,  4, doc.value("sem_y", 0.0));
    sqlite3_bind_double(stmt,  5, doc.value("sem_z", 0.0));
    sqlite3_bind_double(stmt,  6, doc.value("color_h", 0.0));
    sqlite3_bind_double(stmt,  7, doc.value("color_s", 0.0));
    sqlite3_bind_double(stmt,  8, doc.value("color_b", 0.0));
    sqlite3_bind_double(stmt,  9, doc.value("temporal_phase", 0.0));
    sqlite3_bind_int   (stmt, 10, doc.value("affinity_mask", 0));
    sqlite3_bind_int   (stmt, 11, doc.value("mutation_index", 0));
    sqlite3_bind_int   (stmt, 12, doc.value("stroke_count", 0));
    sqlite3_bind_int   (stmt, 13, doc.value("fractal_depth", 1));
    sqlite3_bind_text  (stmt, 14, s("compression_q","LOSSLESS").c_str(), -1, SQLITE_TRANSIENT);
    std::string pl = doc.value("payload", nlohmann::json::object()).dump();
    sqlite3_bind_text  (stmt, 15, pl.c_str(),            -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 16, s("checksum").c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int   (stmt, 17, doc.value("version", 1));
    // RQ^R2 trust + halo columns
    sqlite3_bind_text  (stmt, 18, s("trust_state","dream").c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 19, doc.value("trust_score", 0.0));
    sqlite3_bind_double(stmt, 20, doc.value("ac_ratio", 0.5));
    if (doc.contains("domain") && !doc["domain"].is_null())
        sqlite3_bind_text(stmt, 21, doc["domain"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 21);
    std::string halo = doc.value("halo_json", nlohmann::json::object()).dump();
    sqlite3_bind_text  (stmt, 22, halo.c_str(), -1, SQLITE_TRANSIENT);
    if (doc.contains("lineage_parent") && doc["lineage_parent"].is_number_integer())
        sqlite3_bind_int(stmt, 23, doc["lineage_parent"].get<int>());
    else
        sqlite3_bind_null(stmt, 23);
    if (doc.contains("dream_source") && doc["dream_source"].is_number_integer())
        sqlite3_bind_int(stmt, 24, doc["dream_source"].get<int>());
    else
        sqlite3_bind_null(stmt, 24);
    sqlite3_bind_double(stmt, 25, doc.value("decay_score", 1.0));
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::symbol_by_id(int id)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,radical,layer,sem_x,sem_y,sem_z,color_h,color_s,color_b,"
        "temporal_phase,affinity_mask,mutation_index,stroke_count,fractal_depth,"
        "compression_q,payload,checksum,version,created_at,updated_at"
        " FROM symbols WHERE id=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return nullptr; }
    auto txt = [&](int c) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, c);
        return p ? reinterpret_cast<const char*>(p) : "";
    };
    nlohmann::json result = {
        {"id",             sqlite3_column_int(stmt, 0)},
        {"radical",        txt(1)},  {"layer",  txt(2)},
        {"sem_x",          sqlite3_column_double(stmt, 3)},
        {"sem_y",          sqlite3_column_double(stmt, 4)},
        {"sem_z",          sqlite3_column_double(stmt, 5)},
        {"color_h",        sqlite3_column_double(stmt, 6)},
        {"color_s",        sqlite3_column_double(stmt, 7)},
        {"color_b",        sqlite3_column_double(stmt, 8)},
        {"temporal_phase", sqlite3_column_double(stmt, 9)},
        {"affinity_mask",  sqlite3_column_int(stmt, 10)},
        {"mutation_index", sqlite3_column_int(stmt, 11)},
        {"stroke_count",   sqlite3_column_int(stmt, 12)},
        {"fractal_depth",  sqlite3_column_int(stmt, 13)},
        {"compression_q",  txt(14)}, {"payload", txt(15)},
        {"checksum",       txt(16)},
        {"version",        sqlite3_column_int(stmt, 17)},
        {"created_at",     txt(18)}, {"updated_at", txt(19)}
    };
    sqlite3_finalize(stmt);
    return result;
}

nlohmann::json BrainDb::symbols_query(const std::string& radical,
                                       const std::string& layer, int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    std::string sql =
        "SELECT id,radical,layer,sem_x,sem_y,sem_z,affinity_mask,mutation_index,"
        "checksum,created_at FROM symbols WHERE 1=1";
    if (!radical.empty()) sql += " AND radical=?1";
    if (!layer.empty())   sql += " AND layer=?2";
    sql += " ORDER BY id DESC LIMIT ?3;";
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (!radical.empty()) sqlite3_bind_text(stmt, 1, radical.c_str(), -1, SQLITE_TRANSIENT);
    if (!layer.empty())   sqlite3_bind_text(stmt, 2, layer.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, limit);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        arr.push_back({
            {"id",             sqlite3_column_int(stmt, 0)},
            {"radical",        txt(1)},  {"layer", txt(2)},
            {"sem_x",          sqlite3_column_double(stmt, 3)},
            {"sem_y",          sqlite3_column_double(stmt, 4)},
            {"sem_z",          sqlite3_column_double(stmt, 5)},
            {"affinity_mask",  sqlite3_column_int(stmt, 6)},
            {"mutation_index", sqlite3_column_int(stmt, 7)},
            {"checksum",       txt(8)},  {"created_at", txt(9)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — RQ^R2 Dream buffer
// ─────────────────────────────────────────────────────────────────────────────

int BrainDb::insert_dream(const nlohmann::json& doc)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO dreams"
        " (raw_input,fragment_data,radicals_guess,sem_x,sem_y,sem_z,"
        "  confidence,source_agent,source_context,dream_type,expires_at)"
        " VALUES (?,?,?,?,?,?,?,?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, doc.value("raw_input","").c_str(), -1, SQLITE_TRANSIENT);
    std::string frag = doc.contains("fragment_data") && !doc["fragment_data"].is_null()
                       ? doc["fragment_data"].dump() : "";
    if (!frag.empty()) sqlite3_bind_text(stmt, 2, frag.c_str(), -1, SQLITE_TRANSIENT);
    else               sqlite3_bind_null(stmt, 2);
    std::string rg = doc.value("radicals_guess", nlohmann::json::array()).dump();
    sqlite3_bind_text(stmt, 3, rg.c_str(), -1, SQLITE_TRANSIENT);
    if (doc.contains("sem_x") && !doc["sem_x"].is_null()) sqlite3_bind_double(stmt, 4, doc["sem_x"].get<double>());
    else sqlite3_bind_null(stmt, 4);
    if (doc.contains("sem_y") && !doc["sem_y"].is_null()) sqlite3_bind_double(stmt, 5, doc["sem_y"].get<double>());
    else sqlite3_bind_null(stmt, 5);
    if (doc.contains("sem_z") && !doc["sem_z"].is_null()) sqlite3_bind_double(stmt, 6, doc["sem_z"].get<double>());
    else sqlite3_bind_null(stmt, 6);
    sqlite3_bind_double(stmt, 7, doc.value("confidence", 0.0));
    sqlite3_bind_text  (stmt, 8, doc.value("source_agent","").c_str(), -1, SQLITE_TRANSIENT);
    if (doc.contains("source_context") && doc["source_context"].is_string())
        sqlite3_bind_text(stmt, 9, doc["source_context"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 9);
    sqlite3_bind_text(stmt, 10, doc.value("dream_type","fragment").c_str(), -1, SQLITE_TRANSIENT);
    if (doc.contains("expires_at") && doc["expires_at"].is_string())
        sqlite3_bind_text(stmt, 11, doc["expires_at"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 11);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::dreams_query(const std::string& dream_type,
                                      double min_confidence, int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string sql =
        "SELECT id,raw_input,radicals_guess,sem_x,sem_y,sem_z,confidence,"
        "source_agent,dream_type,consolidation_count,promoted_to,created_at,expires_at"
        " FROM dreams WHERE rejected_at IS NULL";
    if (!dream_type.empty()) sql += " AND dream_type=?1";
    if (min_confidence > 0.0) sql += " AND confidence>=?2";
    sql += " ORDER BY confidence DESC LIMIT ?3;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (!dream_type.empty()) sqlite3_bind_text(stmt, 1, dream_type.c_str(), -1, SQLITE_TRANSIENT);
    if (min_confidence > 0.0) sqlite3_bind_double(stmt, 2, min_confidence);
    sqlite3_bind_int(stmt, 3, limit);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        nlohmann::json rad = nlohmann::json::array();
        try { rad = nlohmann::json::parse(txt(2)); } catch (...) {}
        arr.push_back({
            {"id",                  sqlite3_column_int(stmt, 0)},
            {"raw_input",           txt(1)},
            {"radicals_guess",      rad},
            {"sem_x",               sqlite3_column_type(stmt,3)==SQLITE_NULL ? nlohmann::json(nullptr) : nlohmann::json(sqlite3_column_double(stmt,3))},
            {"sem_y",               sqlite3_column_type(stmt,4)==SQLITE_NULL ? nlohmann::json(nullptr) : nlohmann::json(sqlite3_column_double(stmt,4))},
            {"sem_z",               sqlite3_column_type(stmt,5)==SQLITE_NULL ? nlohmann::json(nullptr) : nlohmann::json(sqlite3_column_double(stmt,5))},
            {"confidence",          sqlite3_column_double(stmt, 6)},
            {"source_agent",        txt(7)},
            {"dream_type",          txt(8)},
            {"consolidation_count", sqlite3_column_int(stmt, 9)},
            {"promoted_to",         sqlite3_column_type(stmt,10)==SQLITE_NULL ? nlohmann::json(nullptr) : nlohmann::json(sqlite3_column_int(stmt,10))},
            {"created_at",          txt(11)},
            {"expires_at",          sqlite3_column_type(stmt,12)==SQLITE_NULL ? nlohmann::json(nullptr) : nlohmann::json(txt(12))}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — RQ^R2 Theory buffer
// ─────────────────────────────────────────────────────────────────────────────

int BrainDb::insert_theory(const nlohmann::json& doc)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO theories"
        " (dream_origin,glyph_data,radicals,sem_x,sem_y,sem_z,fractal_depth,"
        "  compression_q,hypothesis,supporting_evidence,contradicting_evidence,"
        "  trust_score,promotion_threshold,status)"
        " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_int   (stmt,  1, doc.value("dream_origin", 0));
    std::string gd = doc.value("glyph_data", nlohmann::json::object()).dump();
    sqlite3_bind_text  (stmt,  2, gd.c_str(), -1, SQLITE_TRANSIENT);
    std::string rads = doc.value("radicals", nlohmann::json::array()).dump();
    sqlite3_bind_text  (stmt,  3, rads.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,  4, doc.value("sem_x", 0.0));
    sqlite3_bind_double(stmt,  5, doc.value("sem_y", 0.0));
    sqlite3_bind_double(stmt,  6, doc.value("sem_z", 0.0));
    sqlite3_bind_int   (stmt,  7, doc.value("fractal_depth", 1));
    sqlite3_bind_double(stmt,  8, doc.value("compression_q", 0.5));
    sqlite3_bind_text  (stmt,  9, doc.value("hypothesis","").c_str(), -1, SQLITE_TRANSIENT);
    std::string sup = doc.value("supporting_evidence",   nlohmann::json::array()).dump();
    std::string con = doc.value("contradicting_evidence", nlohmann::json::array()).dump();
    sqlite3_bind_text  (stmt, 10, sup.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 11, con.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 12, doc.value("trust_score", 0.3));
    sqlite3_bind_double(stmt, 13, doc.value("promotion_threshold", 0.75));
    sqlite3_bind_text  (stmt, 14, doc.value("status","active").c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::theories_query(const std::string& status, int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string sql =
        "SELECT id,dream_origin,radicals,sem_x,sem_y,sem_z,fractal_depth,"
        "compression_q,hypothesis,test_count,pass_count,fail_count,"
        "trust_score,status,created_at,last_tested,promoted_at"
        " FROM theories WHERE 1=1";
    if (!status.empty()) sql += " AND status=?1";
    sql += " ORDER BY trust_score DESC LIMIT ?2;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (!status.empty()) sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, limit);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        nlohmann::json rads = nlohmann::json::array();
        try { rads = nlohmann::json::parse(txt(2)); } catch (...) {}
        arr.push_back({
            {"id",               sqlite3_column_int(stmt,  0)},
            {"dream_origin",     sqlite3_column_int(stmt,  1)},
            {"radicals",         rads},
            {"sem_x",            sqlite3_column_double(stmt, 3)},
            {"sem_y",            sqlite3_column_double(stmt, 4)},
            {"sem_z",            sqlite3_column_double(stmt, 5)},
            {"fractal_depth",    sqlite3_column_int(stmt,   6)},
            {"compression_q",    sqlite3_column_double(stmt, 7)},
            {"hypothesis",       txt(8)},
            {"test_count",       sqlite3_column_int(stmt,   9)},
            {"pass_count",       sqlite3_column_int(stmt,  10)},
            {"fail_count",       sqlite3_column_int(stmt,  11)},
            {"trust_score",      sqlite3_column_double(stmt,12)},
            {"status",           txt(13)},
            {"created_at",       txt(14)},
            {"last_tested",      sqlite3_column_type(stmt,15)==SQLITE_NULL ? nlohmann::json(nullptr) : nlohmann::json(txt(15))},
            {"promoted_at",      sqlite3_column_type(stmt,16)==SQLITE_NULL ? nlohmann::json(nullptr) : nlohmann::json(txt(16))}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — RQ^R2 Trust escalation
// ─────────────────────────────────────────────────────────────────────────────

bool BrainDb::trust_escalate(int symbol_id, const std::string& new_state)
{
    // Valid transitions: dream→theory→trusted→axiom (no downgrades)
    static const std::unordered_map<std::string,double> kScores = {
        {"dream",   0.3},
        {"theory",  0.6},
        {"trusted", 0.85},
        {"axiom",   1.0}
    };
    if (kScores.find(new_state) == kScores.end()) return false;
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "UPDATE symbols SET trust_state=?, trust_score=?,"
        " promoted_at=datetime('now'), decay_score=1.0"
        " WHERE id=? AND trust_state != 'axiom';",
        -1, &stmt, nullptr);
    sqlite3_bind_text  (stmt, 1, new_state.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, kScores.at(new_state));
    sqlite3_bind_int   (stmt, 3, symbol_id);
    sqlite3_step(stmt);
    int rows = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return rows > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — RQ^R2 Tendrils (mycelium routing)
// ─────────────────────────────────────────────────────────────────────────────

int BrainDb::insert_tendril(const nlohmann::json& doc)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO tendrils (source_symbol,target_symbol,weight,tendril_type)"
        " VALUES (?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_int   (stmt, 1, doc.value("source_symbol", 0));
    sqlite3_bind_int   (stmt, 2, doc.value("target_symbol", 0));
    sqlite3_bind_double(stmt, 3, doc.value("weight", 0.5));
    sqlite3_bind_text  (stmt, 4, doc.value("tendril_type","association").c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::tendrils_query(int source, int target,
                                        const std::string& tendril_type, int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string sql =
        "SELECT id,source_symbol,target_symbol,weight,traffic_count,"
        "last_traversed,tendril_type,created_at FROM tendrils WHERE 1=1";
    if (source > 0)           sql += " AND source_symbol=?1";
    if (target > 0)           sql += " AND target_symbol=?2";
    if (!tendril_type.empty()) sql += " AND tendril_type=?3";
    sql += " ORDER BY weight DESC LIMIT ?4;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (source > 0)           sqlite3_bind_int (stmt, 1, source);
    if (target > 0)           sqlite3_bind_int (stmt, 2, target);
    if (!tendril_type.empty()) sqlite3_bind_text(stmt, 3, tendril_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, limit);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        arr.push_back({
            {"id",              sqlite3_column_int   (stmt, 0)},
            {"source_symbol",   sqlite3_column_int   (stmt, 1)},
            {"target_symbol",   sqlite3_column_int   (stmt, 2)},
            {"weight",          sqlite3_column_double(stmt, 3)},
            {"traffic_count",   sqlite3_column_int   (stmt, 4)},
            {"last_traversed",  txt(5)},
            {"tendril_type",    txt(6)},
            {"created_at",      txt(7)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — RQ^R2 Radicals query
// ─────────────────────────────────────────────────────────────────────────────

nlohmann::json BrainDb::radicals_query(int tier, const std::string& domain)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string sql =
        "SELECT id,code,name,tier,domain,sem_x,sem_y,sem_z,color_hue,ac_ratio,description"
        " FROM radicals WHERE 1=1";
    if (tier > 0)         sql += " AND tier=?1";
    if (!domain.empty())  sql += " AND domain=?2";
    sql += " ORDER BY code;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (tier > 0)        sqlite3_bind_int (stmt, 1, tier);
    if (!domain.empty()) sqlite3_bind_text(stmt, 2, domain.c_str(), -1, SQLITE_TRANSIENT);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        arr.push_back({
            {"id",          sqlite3_column_int   (stmt, 0)},
            {"code",        txt(1)},
            {"name",        txt(2)},
            {"tier",        sqlite3_column_int   (stmt, 3)},
            {"domain",      txt(4)},
            {"sem_x",       sqlite3_column_double(stmt, 5)},
            {"sem_y",       sqlite3_column_double(stmt, 6)},
            {"sem_z",       sqlite3_column_double(stmt, 7)},
            {"color_hue",   sqlite3_column_double(stmt, 8)},
            {"ac_ratio",    sqlite3_column_double(stmt, 9)},
            {"description", txt(10)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — RQ^R2 Consolidation cycle ("sleep")
// ─────────────────────────────────────────────────────────────────────────────

nlohmann::json BrainDb::consolidation_tick()
{
    // Counts for the report
    int dreams_scanned = 0, candidates_flagged = 0;
    int promoted_to_theory = 0, merged = 0, rejected = 0;
    int dreams_expired = 0, symbols_decayed = 0, tendrils_pruned = 0;
    nlohmann::json new_theory_ids = nlohmann::json::array();

    // ── STAGE 1 — Light Sleep: find promotion candidates ─────────────────────
    {
        std::lock_guard<std::mutex> lk(mtx_);
        sqlite3_stmt* cnt = nullptr;
        sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM dreams WHERE rejected_at IS NULL;",
                           -1, &cnt, nullptr);
        if (sqlite3_step(cnt) == SQLITE_ROW) dreams_scanned = sqlite3_column_int(cnt, 0);
        sqlite3_finalize(cnt);
    }
    // Candidates: high confidence OR recurring
    nlohmann::json candidates = nlohmann::json::array();
    {
        std::lock_guard<std::mutex> lk(mtx_);
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_,
            "SELECT id,raw_input,radicals_guess,sem_x,sem_y,sem_z,confidence,"
            "source_agent,dream_type,consolidation_count FROM dreams"
            " WHERE rejected_at IS NULL AND promoted_to IS NULL"
            "   AND (confidence >= 0.6 OR consolidation_count >= 3)"
            " ORDER BY confidence DESC LIMIT 20;",
            -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto txt = [&](int c) -> std::string {
                const unsigned char* p = sqlite3_column_text(stmt, c);
                return p ? reinterpret_cast<const char*>(p) : "";
            };
            candidates.push_back({
                {"id",                  sqlite3_column_int   (stmt, 0)},
                {"raw_input",           txt(1)},
                {"radicals_guess",      txt(2)},
                {"sem_x",               sqlite3_column_type(stmt,3)==SQLITE_NULL ? 0.0 : sqlite3_column_double(stmt,3)},
                {"sem_y",               sqlite3_column_type(stmt,4)==SQLITE_NULL ? 0.0 : sqlite3_column_double(stmt,4)},
                {"sem_z",               sqlite3_column_type(stmt,5)==SQLITE_NULL ? 0.0 : sqlite3_column_double(stmt,5)},
                {"confidence",          sqlite3_column_double(stmt, 6)},
                {"source_agent",        txt(7)},
                {"dream_type",          txt(8)},
                {"consolidation_count", sqlite3_column_int   (stmt, 9)}
            });
        }
        sqlite3_finalize(stmt);
        candidates_flagged = static_cast<int>(candidates.size());
    }

    // ── STAGE 2 — Deep Sleep: promote candidates to theories ─────────────────
    for (const auto& cand : candidates) {
        int dream_id = cand["id"].get<int>();
        double conf  = cand["confidence"].get<double>();
        std::string hyp = "Theory from dream: " + cand.value("raw_input", "");

        nlohmann::json theory = {
            {"dream_origin",          dream_id},
            {"sem_x",                 cand.value("sem_x", 0.0)},
            {"sem_y",                 cand.value("sem_y", 0.0)},
            {"sem_z",                 cand.value("sem_z", 0.0)},
            {"fractal_depth",         1},
            {"compression_q",         0.5},
            {"hypothesis",            hyp},
            {"trust_score",           conf * 0.8},  // start slightly below dream confidence
            {"supporting_evidence",   nlohmann::json::array()},
            {"contradicting_evidence",nlohmann::json::array()},
            {"status",                "active"}
        };
        int tid = insert_theory(theory);  // NOTE: calls raw insert, no extra lock needed
        new_theory_ids.push_back(tid);
        promoted_to_theory++;

        // Mark dream as promoted
        {
            std::lock_guard<std::mutex> lk(mtx_);
            sqlite3_stmt* upd = nullptr;
            sqlite3_prepare_v2(db_,
                "UPDATE dreams SET promoted_to=?, last_consolidated=datetime('now')"
                " WHERE id=?;",
                -1, &upd, nullptr);
            sqlite3_bind_int(upd, 1, tid);
            sqlite3_bind_int(upd, 2, dream_id);
            sqlite3_step(upd);
            sqlite3_finalize(upd);
        }
    }

    // ── STAGE 3 — Check theories ready for trusted promotion ─────────────────
    {
        std::lock_guard<std::mutex> lk(mtx_);
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_,
            "SELECT id,dream_origin,sem_x,sem_y,sem_z,radicals,trust_score"
            " FROM theories"
            " WHERE status='active'"
            "   AND trust_score >= 0.75"
            "   AND pass_count >= 3"
            "   AND (fail_count * 1.0 / NULLIF(pass_count,0)) < 0.2"
            " LIMIT 10;",
            -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto txt = [&](int c) -> std::string {
                const unsigned char* p = sqlite3_column_text(stmt, c);
                return p ? reinterpret_cast<const char*>(p) : "";
            };
            int tid = sqlite3_column_int(stmt, 0);
            nlohmann::json sym = {
                {"sem_x",        sqlite3_column_double(stmt, 2)},
                {"sem_y",        sqlite3_column_double(stmt, 3)},
                {"sem_z",        sqlite3_column_double(stmt, 4)},
                {"trust_state",  "trusted"},
                {"trust_score",  sqlite3_column_double(stmt, 6)},
                {"dream_source", sqlite3_column_int(stmt, 1)},
                {"fractal_depth",1},
                {"compression_q","0.8"},
                {"decay_score",  1.0}
            };
            // promote to symbols (finalize inner stmt before re-entrant insert)
            sqlite3_finalize(stmt);
            stmt = nullptr;

            int sym_id = insert_symbol(sym);  // acquires its own lock

            // Mark theory as promoted
            {
                sqlite3_stmt* upd = nullptr;
                sqlite3_prepare_v2(db_,
                    "UPDATE theories SET status='promoted', promoted_at=datetime('now')"
                    " WHERE id=?;",
                    -1, &upd, nullptr);
                sqlite3_bind_int(upd, 1, tid);
                sqlite3_step(upd);
                sqlite3_finalize(upd);
            }
            (void)sym_id;
            break;  // restart from the outer loop since we re-opened stmt
        }
        if (stmt) sqlite3_finalize(stmt);
    }

    // ── STAGE 4 — Pruning: expire dreams past TTL, decay symbols ─────────────
    {
        std::lock_guard<std::mutex> lk(mtx_);
        // Expire dreams past TTL
        sqlite3_stmt* exp = nullptr;
        sqlite3_prepare_v2(db_,
            "UPDATE dreams SET rejected_at=datetime('now'),"
            " rejection_reason='ttl_expired'"
            " WHERE expires_at IS NOT NULL"
            "   AND expires_at < datetime('now')"
            "   AND rejected_at IS NULL;",
            -1, &exp, nullptr);
        sqlite3_step(exp);
        dreams_expired = sqlite3_changes(db_);
        sqlite3_finalize(exp);

        // Apply passive decay to symbols (not axioms)
        // decay_score *= 0.999 per consolidation tick (very slow for trusted)
        sqlite3_stmt* dec = nullptr;
        sqlite3_prepare_v2(db_,
            "UPDATE symbols SET decay_score = decay_score *"
            " CASE trust_state"
            "   WHEN 'dream'   THEN 0.90"
            "   WHEN 'theory'  THEN 0.99"
            "   WHEN 'trusted' THEN 0.999"
            "   ELSE 1.0 END"  // axiom: no decay
            " WHERE trust_state != 'axiom';",
            -1, &dec, nullptr);
        sqlite3_step(dec);
        symbols_decayed = sqlite3_changes(db_);
        sqlite3_finalize(dec);

        // Prune tendrils with near-zero weight
        sqlite3_stmt* prune = nullptr;
        sqlite3_prepare_v2(db_,
            "DELETE FROM tendrils WHERE weight < 0.01;",
            -1, &prune, nullptr);
        sqlite3_step(prune);
        tendrils_pruned = sqlite3_changes(db_);
        sqlite3_finalize(prune);
    }

    // ── Build health snapshot ─────────────────────────────────────────────────
    nlohmann::json health;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        auto count_table = [&](const char* tbl) -> int {
            sqlite3_stmt* s = nullptr;
            std::string q = "SELECT COUNT(*) FROM "; q += tbl; q += ";";
            sqlite3_prepare_v2(db_, q.c_str(), -1, &s, nullptr);
            int n = 0;
            if (sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
            sqlite3_finalize(s);
            return n;
        };
        health = {
            {"total_symbols",   count_table("symbols")},
            {"total_dreams",    count_table("dreams")},
            {"total_theories",  count_table("theories")},
            {"total_tendrils",  count_table("tendrils")}
        };
    }

    return {
        {"stages", {
            {"n1_light_sleep",  {{"dreams_scanned",    dreams_scanned},
                                  {"candidates_flagged", candidates_flagged}}},
            {"n3_deep_sleep",   {{"promoted_to_theory", promoted_to_theory},
                                  {"merged_with_existing", merged},
                                  {"rejected", rejected},
                                  {"new_theory_ids", new_theory_ids}}},
            {"pruning",         {{"dreams_expired",   dreams_expired},
                                  {"symbols_decayed",  symbols_decayed},
                                  {"tendrils_pruned",  tendrils_pruned}}}
        }},
        {"health_snapshot", health}
    };
}

int BrainDb::store_knowledge(const std::string& radical, const std::string& layer,
                              const nlohmann::json& entry)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string e = entry.dump();
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO knowledge (radical,layer,entry,sem_x,sem_y,sem_z)"
        " VALUES (?,?,?,?,?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text  (stmt, 1, radical.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, layer.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 3, e.c_str(),        -1, SQLITE_TRANSIENT);
    auto sem = entry.value("semantic", nlohmann::json::array({0.0, 0.0, 0.0}));
    sqlite3_bind_double(stmt, 4, sem.size() > 0 ? sem[0].get<double>() : 0.0);
    sqlite3_bind_double(stmt, 5, sem.size() > 1 ? sem[1].get<double>() : 0.0);
    sqlite3_bind_double(stmt, 6, sem.size() > 2 ? sem[2].get<double>() : 0.0);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::query_knowledge(const std::string& radical, int limit)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,radical,layer,entry,sem_x,sem_y,sem_z,created_at"
        " FROM knowledge WHERE radical=? ORDER BY id DESC LIMIT ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, radical.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, limit);
    auto arr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto txt = [&](int c) -> std::string {
            const unsigned char* p = sqlite3_column_text(stmt, c);
            return p ? reinterpret_cast<const char*>(p) : "";
        };
        nlohmann::json e;
        try { e = nlohmann::json::parse(txt(3)); } catch (...) { e = nlohmann::json::object(); }
        arr.push_back({
            {"id",         sqlite3_column_int(stmt, 0)},
            {"radical",    txt(1)},  {"layer", txt(2)},
            {"entry",      e},
            {"sem_x",      sqlite3_column_double(stmt, 4)},
            {"sem_y",      sqlite3_column_double(stmt, 5)},
            {"sem_z",      sqlite3_column_double(stmt, 6)},
            {"created_at", txt(7)}
        });
    }
    sqlite3_finalize(stmt);
    return arr;
}

int BrainDb::save_checkpoint(const std::string& agent_name, const nlohmann::json& state)
{
    std::lock_guard<std::mutex> lk(mtx_);
    std::string s = state.dump();
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "INSERT INTO agent_checkpoints (agent_name,state) VALUES (?,?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, agent_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, s.c_str(),          -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

nlohmann::json BrainDb::load_checkpoint(const std::string& agent_name, int checkpoint_id)
{
    std::lock_guard<std::mutex> lk(mtx_);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_,
        "SELECT id,state,created_at FROM agent_checkpoints"
        " WHERE agent_name=? AND id=?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, agent_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, checkpoint_id);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return nullptr; }
    auto txt = [&](int c) -> std::string {
        const unsigned char* p = sqlite3_column_text(stmt, c);
        return p ? reinterpret_cast<const char*>(p) : "";
    };
    nlohmann::json s;
    try { s = nlohmann::json::parse(txt(1)); } catch (...) { s = nlohmann::json::object(); }
    nlohmann::json result = {
        {"id",         sqlite3_column_int(stmt, 0)},
        {"state",      s},
        {"created_at", txt(2)}
    };
    sqlite3_finalize(stmt);
    return result;
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
    auto events = db_.pending_events(20);
    if (events.empty()) return 1000;   // idle back-off

    // Event types consumed directly by their dedicated specialized agents.
    // WorkerAgent must not mark these processed — doing so would kill the event
    // before the owning agent can read it from the queue.
    static const std::initializer_list<std::string_view> kSpecializedTypes = {
        "chat_request", "wallet_sign", "wallet_verify",
        "torrent_piece", "edge_task", "classify_symbol"
    };

    for (const auto& ev : events) {
        const std::string type = ev.value("event_type", "");

        // Skip events owned by specialized agents — they consume these directly.
        bool skip = false;
        for (auto t : kSpecializedTypes) {
            if (type == t) { skip = true; break; }
        }
        if (skip) continue;

        db_.increment_event_attempts(ev["id"].get<int>());
        try {
            handle(ev);
        } catch (const std::exception& e) {
            db_.audit_log(name_, "handle_error", type,
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
    // WorkerAgent handles: heartbeat, strategy_execute, rune_trust.
    // Specialized event types (chat_request, wallet_sign, wallet_verify,
    // torrent_piece, edge_task, classify_symbol) are filtered out in tick()
    // and consumed directly by their dedicated agents.
    // To add a new Worker-owned event type:
    //   1. Handle it here.
    //   2. Remove it from kSpecializedTypes if it was previously specialized.
    // ────────────────────────────────────────────────────────────────────────

    if (type == "heartbeat") {
        // HeartAgent owns its own state; Worker just acknowledges.
        emit("worker_ack", {{"event_id", id}, {"type", type}});

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
// ChatAgent — LLM integration via RUNE_LLM_URL
// ─────────────────────────────────────────────────────────────────────────────

int ChatAgent::tick()
{
    auto events = db_.pending_events_of_type("chat_request", 3);
    if (events.empty()) return 5000;

    const char* llm_url   = std::getenv("RUNE_LLM_URL");
    const char* llm_model = std::getenv("RUNE_LLM_MODEL");
    std::string model     = llm_model ? llm_model : "local-model";

    for (const auto& ev : events) {
        int         ev_id   = ev.value("id", -1);
        std::string message = ev["payload"].value("message", "");
        if (message.empty()) { db_.mark_processed(ev_id); continue; }

        std::string reply;
        if (llm_url) {
            std::string url(llm_url);
            std::string host; int port = 80; std::string path = "/";
            size_t pp = url.find("://");
            if (pp != std::string::npos) url = url.substr(pp + 3);
            size_t sl = url.find('/');
            if (sl != std::string::npos) { path = url.substr(sl); url = url.substr(0, sl); }
            size_t co = url.find(':');
            if (co != std::string::npos) {
                host = url.substr(0, co);
                try { port = std::stoi(url.substr(co + 1)); } catch (...) { port = 80; }
            } else { host = url; }

            try {
                httplib::Client cli(host, port);
                cli.set_connection_timeout(5);
                cli.set_read_timeout(30);
                nlohmann::json body = {
                    {"model",    model},
                    {"messages", {{{"role","user"},{"content",message}}}},
                    {"stream",   false}
                };
                auto res = cli.Post(path, body.dump(), "application/json");
                if (res && res->status == 200) {
                    auto j = nlohmann::json::parse(res->body);
                    if (j.contains("choices") && j["choices"].is_array()
                            && !j["choices"].empty()
                            && j["choices"][0].contains("message")
                            && j["choices"][0]["message"].contains("content")) {
                        reply = j["choices"][0]["message"]["content"].get<std::string>();
                    } else {
                        reply = "[llm] unexpected response shape";
                    }
                } else {
                    reply = "[llm error] status=" +
                            (res ? std::to_string(res->status) : "no_response");
                }
            } catch (const std::exception& e) {
                reply = std::string("[llm error] ") + e.what();
            }
        } else {
            reply = "[echo] " + message + "  (set RUNE_LLM_URL to enable LLM replies)";
        }

        emit("chat_response", {{"message", message}, {"reply", reply}});
        db_.audit_log(name_, "chat_reply", "llm",
                      {{"message", message}, {"reply", reply}});
        db_.mark_processed(ev_id);
        db_.update_agent_tick(name_);
        log("[Chat] replied to: %.60s\n", message.c_str());
    }
    return 2000;
}

// ─────────────────────────────────────────────────────────────────────────────
// WalletAgent — event-driven Ed25519 signing / verification
// ─────────────────────────────────────────────────────────────────────────────

int WalletAgent::tick()
{
    {
        auto events = db_.pending_events_of_type("wallet_sign", 5);
        for (const auto& ev : events) {
            int ev_id = ev.value("id", -1);
            try {
                std::string msg_hex  = ev["payload"].value("message_hex", "");
                std::string key_path = ev["payload"].value("key_path", "wallet.json");
                auto kp      = wallet::load(key_path);
                auto msg_raw = wallet::from_hex(msg_hex);
                if (msg_raw.empty()) throw std::runtime_error("empty message_hex");
                auto sig     = wallet::sign(kp, {msg_raw.data(), msg_raw.size()});
                std::string sig_hex = wallet::to_hex(sig);
                std::string pub_hex = wallet::to_hex(kp.public_key);
                emit("wallet_signature", {
                    {"public_key",  pub_hex},
                    {"signature",   sig_hex},
                    {"message_hex", msg_hex}
                });
                db_.audit_log(name_, "wallet_sign", pub_hex,
                    {{"message_hex", msg_hex}, {"sig_hex", sig_hex}});
                db_.record_trust(name_, "agent", 0.01, "successful_signing");
            } catch (const std::exception& e) {
                emit("wallet_error", {{"error", e.what()}, {"event_id", ev_id}});
                db_.increment_agent_errors(name_);
            }
            db_.mark_processed(ev_id);
        }
    }
    {
        auto events = db_.pending_events_of_type("wallet_verify", 5);
        for (const auto& ev : events) {
            int ev_id = ev.value("id", -1);
            try {
                std::string pub_hex = ev["payload"].value("public_key", "");
                std::string msg_hex = ev["payload"].value("message_hex", "");
                std::string sig_hex = ev["payload"].value("signature",   "");
                auto pub_raw = wallet::from_hex(pub_hex);
                auto msg_raw = wallet::from_hex(msg_hex);
                auto sig_raw = wallet::from_hex(sig_hex);
                if (pub_raw.size() != 32 || sig_raw.size() != 64)
                    throw std::runtime_error("invalid key/sig length");
                std::array<uint8_t,32> pk{};
                std::array<uint8_t,64> sv{};
                std::copy(pub_raw.begin(), pub_raw.end(), pk.begin());
                std::copy(sig_raw.begin(), sig_raw.end(), sv.begin());
                bool ok = wallet::verify(pk, {msg_raw.data(), msg_raw.size()}, sv);
                emit("wallet_verify_result", {{"valid", ok}, {"public_key", pub_hex}});
                db_.audit_log(name_, "wallet_verify", pub_hex, {{"valid", ok}});
            } catch (const std::exception& e) {
                emit("wallet_error", {{"error", e.what()}, {"event_id", ev_id}});
            }
            db_.mark_processed(ev_id);
        }
    }

    db_.update_agent_tick(name_);
    return 3000;
}

// ─────────────────────────────────────────────────────────────────────────────
// NodeAgent — peer health monitoring + trust updates
// ─────────────────────────────────────────────────────────────────────────────

int NodeAgent::tick()
{
    auto nodes = db_.nodes_list();
    if (nodes.empty()) return 10000;

    for (const auto& node : nodes) {
        std::string node_id = node.value("node_id", "");
        std::string host    = node.value("host", "");
        int         port    = node.value("port", 7071);
        if (host.empty()) continue;

        bool reachable = false;
        try {
            httplib::Client cli(host, port);
            cli.set_connection_timeout(3);
            cli.set_read_timeout(5);
            auto res = cli.Get("/brain/health");
            reachable = (res && res->status == 200);
        } catch (...) { reachable = false; }

        if (reachable) {
            db_.record_trust(node_id, "node", 0.01, "health_check_ok");
            emit("node_healthy", {{"node_id", node_id}, {"host", host}, {"port", port}});
        } else {
            db_.record_trust(node_id, "node", -0.05, "health_check_failed");
            db_.upsert_node(node_id, node.value("label", node_id), "unreachable");
            emit("node_unreachable", {{"node_id", node_id}, {"host", host}});
            log("[Node] peer %s unreachable\n", node_id.c_str());
        }
    }

    db_.update_agent_tick(name_);
    return 10000;
}

// ─────────────────────────────────────────────────────────────────────────────
// TorrentAgent — piece tracking and replication across the node mesh
// ─────────────────────────────────────────────────────────────────────────────

int TorrentAgent::tick()
{
    auto events = db_.pending_events_of_type("torrent_piece", 5);
    for (const auto& ev : events) {
        int ev_id = ev.value("id", -1);
        try {
            std::string piece_id  = ev["payload"].value("piece_id",  "");
            std::string data_hash = ev["payload"].value("data_hash", "");
            int         size_kb   = ev["payload"].value("size_kb",   0);
            std::string source    = ev["payload"].value("source",    "local");

            db_.insert_artifact("torrent_piece", name_, {
                {"piece_id",  piece_id},
                {"data_hash", data_hash},
                {"size_kb",   size_kb},
                {"source",    source},
                {"available", true}
            }, 86400 * 7);

            emit("torrent_piece_stored", {
                {"piece_id",  piece_id},
                {"data_hash", data_hash},
                {"size_kb",   size_kb}
            });
            db_.audit_log(name_, "piece_stored", piece_id,
                {{"data_hash", data_hash}, {"size_kb", size_kb}});
        } catch (const std::exception& e) {
            emit("torrent_error", {{"error", e.what()}, {"event_id", ev_id}});
            db_.increment_agent_errors(name_);
        }
        db_.mark_processed(ev_id);
    }

    if (tick_count_ % 6 == 0) {
        auto peers = db_.nodes_list();
        emit("torrent_status", {
            {"peer_count", (int)peers.size()},
            {"tick",       tick_count_}
        });
    }

    db_.update_agent_tick(name_);
    return 30000;
}

// ─────────────────────────────────────────────────────────────────────────────
// EdgeAgent — low-latency edge compute (hashing, QR detection events)
// ─────────────────────────────────────────────────────────────────────────────

int EdgeAgent::tick()
{
    auto events = db_.pending_events_of_type("edge_task", 10);
    for (const auto& ev : events) {
        int ev_id = ev.value("id", -1);
        try {
            std::string task_type = ev["payload"].value("task_type", "hash");
            std::string data_hex  = ev["payload"].value("data_hex",  "");

            nlohmann::json result;
            if (task_type == "hash") {
                uint64_t h = 14695981039346656037ULL;
                for (unsigned char c : data_hex) {
                    h ^= static_cast<uint64_t>(c);
                    h *= 1099511628211ULL;
                }
                std::ostringstream oss;
                oss << std::hex << std::setw(16) << std::setfill('0') << h;
                result = {{"task_type", "hash"}, {"hash", oss.str()}, {"algo", "fnv1a64"}};
            } else if (task_type == "qr_detect") {
                result = {
                    {"task_type", "qr_detect"},
                    {"detected",  false},
                    {"note",      "QR library not linked; set task_type=hash for hash ops"}
                };
            } else {
                result = {{"task_type", task_type}, {"error", "unknown_task_type"}};
            }

            emit("edge_result", {{"event_id", ev_id}, {"result", result}});
            db_.audit_log(name_, "edge_task", task_type, result);
        } catch (const std::exception& e) {
            emit("edge_error", {{"error", e.what()}, {"event_id", ev_id}});
            db_.increment_agent_errors(name_);
        }
        db_.mark_processed(ev_id);
    }

    db_.update_agent_tick(name_);
    return 10000;
}

// ─────────────────────────────────────────────────────────────────────────────
// HousekeepingAgent — DB maintenance and audit archival
// ─────────────────────────────────────────────────────────────────────────────

int HousekeepingAgent::tick()
{
    int archived = db_.archive_old_audit(30);
    if (archived > 0) {
        db_.audit_log(name_, "audit_archive", "audit", {{"archived", archived}});
        log("[Housekeeping] archived %d old audit rows\n", archived);
    }

    if (tick_count_ % 10 == 0) {
        db_.exec("PRAGMA optimize;");
        log("[Housekeeping] PRAGMA optimize complete\n");
    }

    db_.update_agent_tick(name_);
    emit("housekeeping_cycle", {{"archived_audit", archived}, {"tick", tick_count_}});
    return 30000;
}

// ─────────────────────────────────────────────────────────────────────────────
// StrategyAgent — infers co-activation patterns from audit log
// ─────────────────────────────────────────────────────────────────────────────

int StrategyAgent::tick()
{
    auto runes = db_.all_runes(200, 0);

    std::vector<std::pair<int,std::string>> ranked;
    for (const auto& r : runes) {
        if (r.value("active", 0) == 1)
            ranked.push_back({r.value("usage_count", 0), r.value("name", "")});
    }
    std::sort(ranked.begin(), ranked.end(), std::greater<>());

    if (ranked.size() >= 3 && ranked[0].first >= 3) {
        auto n0 = ranked[0].second;
        auto n1 = ranked[1].second;
        auto n2 = ranked[2].second;
        std::string strat_name = "inferred_"
            + n0.substr(0, std::min<size_t>(4, n0.size()))
            + n1.substr(0, std::min<size_t>(4, n1.size()))
            + n2.substr(0, std::min<size_t>(4, n2.size()));

        auto existing = db_.strategy_by_name(strat_name);
        if (existing.is_null()) {
            nlohmann::json rune_ids = {n0, n1, n2};
            db_.insert_strategy(strat_name,
                "Auto-inferred co-activation pattern", rune_ids, 1);
            emit("strategy_inferred", {{"name", strat_name}, {"runes", rune_ids}});
            db_.audit_log(name_, "strategy_infer", strat_name, {{"runes", rune_ids}});
            log("[Strategy] inferred: %s\n", strat_name.c_str());
        }
    }

    db_.update_agent_tick(name_);
    return 30000;
}

// ─────────────────────────────────────────────────────────────────────────────
// EpicRuneAgent — detects runes achieving epic status (trust≥3 & usage≥100)
// ─────────────────────────────────────────────────────────────────────────────

int EpicRuneAgent::tick()
{
    auto runes = db_.all_runes(200, 0);
    for (const auto& r : runes) {
        double      trust = r.value("trust", 0.0);
        int         usage = r.value("usage_count", 0);
        int         level = r.value("level", 0);
        std::string name  = r.value("name",  "");
        std::string glyph = r.value("glyph", "");

        if (trust >= 3.0 && usage >= 100 && level >= 3) {
            std::string narrative =
                "The rune " + name + " (" + glyph + ") has achieved epic status: "
                "trust=" + std::to_string(trust) +
                ", usage=" + std::to_string(usage) +
                ", level=" + std::to_string(level) + ".";

            db_.insert_artifact("epic_rune", name_, {
                {"rune",      name},
                {"glyph",     "\xe2\x9f\xa8" + glyph + "\xe2\x9f\xa9"},
                {"trust",     trust},
                {"usage",     usage},
                {"level",     level},
                {"narrative", narrative}
            }, 0);
            db_.audit_log(name_, "epic_rune_born", name,
                {{"trust", trust}, {"usage", usage}});
            emit("epic_rune_born", {
                {"rune",      name},
                {"glyph",     glyph},
                {"narrative", narrative},
                {"trust",     trust},
                {"usage",     usage}
            });
            log("[Epic] %s reached epic status (trust=%.2f usage=%d)\n",
                name.c_str(), trust, usage);
        }
    }

    db_.update_agent_tick(name_);
    return 60000;
}

// ─────────────────────────────────────────────────────────────────────────────
// OverwatchAgent — EventBus anomaly monitor + system health reports
// ─────────────────────────────────────────────────────────────────────────────

OverwatchAgent::OverwatchAgent(std::string name, std::string type,
                                BrainDb& db, EventBus& bus,
                                std::atomic<bool>& shutdown)
    : AgentBase(std::move(name), std::move(type), db, bus, shutdown)
{
    sub_id_ = bus_.subscribe([this](const BrainEvent& ev) {
        std::lock_guard<std::mutex> lk(win_mtx_);
        event_window_[ev.type]++;
        if (ev.type == "gc_complete") {
            auto it = ev.payload.find("quarantined");
            if (it != ev.payload.end() && it->is_number())
                gc_kills_ += it->get<int>();
        }
    });
}

OverwatchAgent::~OverwatchAgent()
{
    if (sub_id_ >= 0) bus_.unsubscribe(sub_id_);
}

int OverwatchAgent::tick()
{
    std::map<std::string,int> snapshot;
    int gc_kills;
    {
        std::lock_guard<std::mutex> lk(win_mtx_);
        snapshot  = event_window_;
        gc_kills  = gc_kills_;
        event_window_.clear();
        gc_kills_ = 0;
    }

    if (gc_kills > 5) {
        emit("overwatch_alert", {
            {"anomaly",  "gc_kill_rate"},
            {"gc_kills", gc_kills},
            {"action",   "consider slowing trust decay rate"}
        });
        db_.audit_log(name_, "anomaly", "gc_kill_rate", {{"gc_kills", gc_kills}});
        log("[Overwatch] ALERT: GC killed %d runes in last window\n", gc_kills);
    }

    int unknown = snapshot.count("worker_unknown") ? snapshot.at("worker_unknown") : 0;
    if (unknown > 20) {
        emit("overwatch_alert", {
            {"anomaly", "unknown_event_flood"},
            {"count",   unknown}
        });
    }

    if (tick_count_ % 4 == 0) {
        auto c      = db_.all_counts();
        auto agents = db_.all_agents();
        emit("overwatch_report", {
            {"runes",   c.runes},
            {"agents",  c.agents},
            {"pending", c.pending_events},
            {"gc_kills_last_window", gc_kills},
            {"event_counts", snapshot},
            {"agent_list", agents}
        });
    }

    db_.update_agent_tick(name_);
    return 15000;
}

// ─────────────────────────────────────────────────────────────────────────────
// LibrarianAgent — symbol classification and agent routing (§17.4)
// ─────────────────────────────────────────────────────────────────────────────

int LibrarianAgent::tick()
{
    auto events = db_.pending_events_of_type("classify_symbol", 5);
    for (const auto& ev : events) {
        int ev_id = ev.value("id", -1);
        try {
            const auto& pl = ev["payload"];
            std::string radical = pl.value("radical", "");
            std::string layer   = pl.value("layer",   "SYNTHETIC");

            std::string dewey_class;
            if (layer == "OLD_NORSE" || layer == "ICELANDIC")
                dewey_class = "800.RUNIC";
            else if (layer == "CJK")
                dewey_class = "400.KANGXI";
            else
                dewey_class = "000.SYNTHETIC";

            nlohmann::json route_to = nlohmann::json::array();
            if (!radical.empty())            route_to.push_back("LibrarianAgent");
            if (layer == "OLD_NORSE")        route_to.push_back("RuneFusionEngine");
            if (pl.contains("message"))      route_to.push_back("ChatAgent");
            if (pl.contains("data_hex"))     route_to.push_back("EdgeAgent");
            if (pl.contains("sign_request")) route_to.push_back("WalletAgent");

            double sem_x = pl.value("sem_x", 0.0);
            double sem_y = pl.value("sem_y", 0.0);
            double sem_z = pl.value("sem_z", 0.0);

            // ── RQ^R2 §3: All new encodings enter as dream-state first ─────────
            // Confidence is derived from how well the radical and coordinates
            // are specified.  Full coordinates + known radical → higher confidence.
            double confidence = 0.3;
            if (!radical.empty())         confidence += 0.2;
            if (sem_x != 0.0 || sem_y != 0.0 || sem_z != 0.0) confidence += 0.15;
            if (pl.contains("trust_score")) confidence = pl["trust_score"].get<double>();
            confidence = std::min(confidence, 1.0);

            // Determine dream_type (fragment if no radical, hallucination otherwise)
            std::string dream_type = radical.empty() ? "fragment" : "hallucination";
            if (pl.contains("dream_type")) dream_type = pl.value("dream_type", dream_type);

            // Build a best-guess radical list from the incoming radical field
            nlohmann::json radicals_guess = nlohmann::json::array();
            if (!radical.empty()) {
                radicals_guess.push_back({
                    {"radical_id",    radical},
                    {"confidence",    confidence},
                    {"match_reason",  "classify_symbol event"}
                });
            }

            // Build halo_json with all available 14D metadata
            nlohmann::json halo = {
                {"color",       {{"hue",0.0},{"saturation",0.2},{"lightness",0.3}}},
                {"intensity",   confidence},
                {"halo_glow",   0.1},
                {"trust",       confidence},
                {"activation",  0.5},
                {"connectivity",0},
                {"temporal",    pl.value("temporal_phase", 0.0)},
                {"spatial",     {{"x",sem_x},{"y",sem_y},{"z",sem_z}}},
                {"lineage_ids", nlohmann::json::array()}
            };

            int dream_id = db_.insert_dream({
                {"raw_input",      pl.value("raw_input", radical)},
                {"radicals_guess", radicals_guess},
                {"sem_x",          sem_x},
                {"sem_y",          sem_y},
                {"sem_z",          sem_z},
                {"confidence",     confidence},
                {"source_agent",   name_},
                {"source_context", dewey_class},
                {"dream_type",     dream_type}
            });

            // Also persist to knowledge substrate for radical indexing
            db_.store_knowledge(radical, layer, {
                {"classification", dewey_class},
                {"route_to",       route_to},
                {"source_event",   ev_id},
                {"dream_id",       dream_id},
                {"semantic",       nlohmann::json::array({sem_x, sem_y, sem_z})}
            });

            // If confidence is high enough, also insert into symbols as dream-state
            if (confidence >= 0.4 && !radical.empty()) {
                db_.insert_symbol({
                    {"radical",      radical},
                    {"layer",        layer},
                    {"sem_x",        sem_x},
                    {"sem_y",        sem_y},
                    {"sem_z",        sem_z},
                    {"color_h",      pl.value("color_h", 0.0)},
                    {"color_s",      pl.value("color_s", 0.0)},
                    {"color_b",      pl.value("color_b", 0.0)},
                    {"temporal_phase",pl.value("temporal_phase", 0.0)},
                    {"fractal_depth",pl.value("fractal_depth", 1)},
                    {"compression_q","LOSSY_LQ"},
                    {"payload",      pl},
                    {"trust_state",  "dream"},
                    {"trust_score",  confidence},
                    {"ac_ratio",     pl.value("ac_ratio", 0.5)},
                    {"halo_json",    halo},
                    {"dream_source", dream_id},
                    {"decay_score",  1.0}
                });
            }

            emit("symbol_classified", {
                {"event_id",    ev_id},
                {"dream_id",    dream_id},
                {"radical",     radical},
                {"layer",       layer},
                {"dewey_class", dewey_class},
                {"trust_state", "dream"},
                {"confidence",  confidence},
                {"route_to",    route_to}
            });

            for (const auto& target : route_to) {
                if (target == "EdgeAgent" && pl.contains("data_hex")) {
                    db_.enqueue_event(name_, "edge_task", {
                        {"task_type", "hash"},
                        {"data_hex",  pl["data_hex"]}
                    });
                }
                if (target == "ChatAgent" && pl.contains("message")) {
                    db_.enqueue_event(name_, "chat_request", {
                        {"message", pl["message"]}
                    }, 1);
                }
            }

            db_.audit_log(name_, "classify", radical,
                {{"dewey_class", dewey_class}, {"route_to", route_to},
                 {"dream_id", dream_id}, {"trust_state", "dream"}});
        } catch (const std::exception& e) {
            emit("librarian_error", {{"error", e.what()}, {"event_id", ev_id}});
            db_.increment_agent_errors(name_);
        }
        db_.mark_processed(ev_id);
    }

    // Periodically run a lightweight consolidation tick during idle
    if (tick_count_ % 60 == 0) {
        try {
            auto report = db_.consolidation_tick();
            emit("consolidation_complete", report);
        } catch (const std::exception& e) {
            emit("consolidation_error", {{"error", e.what()}});
        }
    }

    if (tick_count_ % 8 == 0) {
        auto runes = db_.all_runes(200, 0);
        for (const auto& r : runes) {
            std::string name  = r.value("name",  "");
            std::string glyph = r.value("glyph", "");
            std::string cat   = r.value("category", "elemental");
            if (name.empty()) continue;
            db_.store_knowledge(glyph, "OLD_NORSE", {
                {"rune_name",   name},
                {"category",    cat},
                {"trust",       r.value("trust", 1.0)},
                {"usage_count", r.value("usage_count", 0)}
            });
        }
        emit("catalog_updated", {{"rune_count", (int)runes.size()}});
    }

    db_.update_agent_tick(name_);
    return 20000;
}

// ─────────────────────────────────────────────────────────────────────────────
// LoadSimulatorAgent — synthetic event injection for stress testing
// ─────────────────────────────────────────────────────────────────────────────

int LoadSimulatorAgent::tick()
{
    if (tick_count_ > 5 && tick_count_ % 30 != 0) return 10000;

    static std::atomic<int> sim_counter{0};
    int cnt = ++sim_counter;

    static const std::array<const char*,5> targets = {
        "Fehu","Ansuz","Dagaz","Perthro","Mannaz"
    };
    std::string target = targets[cnt % targets.size()];

    db_.enqueue_event("LoadSim", "chat_request", {
        {"message", "Synthetic load test message #" + std::to_string(cnt)},
        {"sim", true}
    });
    db_.enqueue_event("LoadSim", "rune_trust", {
        {"subject", target}, {"delta", 0.01}, {"reason", "load_sim"}
    });
    db_.enqueue_event("LoadSim", "classify_symbol", {
        {"radical", target},
        {"layer",   "OLD_NORSE"},
        {"sem_x",   0.1 * (cnt % 10 - 5)},
        {"sem_y",   0.1 * (cnt % 7 - 3)},
        {"sem_z",   0.0}
    });

    emit("load_sim_tick", {{"sim_counter", cnt}, {"target", target}});
    db_.update_agent_tick(name_);
    log("[LoadSim] tick #%d — injected events for rune=%s\n",
        tick_count_, target.c_str());
    return 10000;
}
