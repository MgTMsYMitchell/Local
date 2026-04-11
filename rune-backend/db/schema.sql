-- Rune System SQLite schema
-- Apply with:  sqlite3 rune.db < schema.sql

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

-- ══════════════════════════════════════════════════════════════════════════════
-- Brain schema (brain.db) — cognitive node tables
-- ══════════════════════════════════════════════════════════════════════════════

-- nodes: cognitive node registry (local + peer nodes)
CREATE TABLE IF NOT EXISTS nodes (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id    TEXT    NOT NULL UNIQUE,
    label      TEXT    NOT NULL DEFAULT '',
    status     TEXT    NOT NULL DEFAULT 'active',
    last_seen  TEXT    DEFAULT (datetime('now')),
    trust      REAL    NOT NULL DEFAULT 1.0,
    metadata   TEXT    NOT NULL DEFAULT '{}',
    created_at TEXT    DEFAULT (datetime('now'))
);

-- agents: registered agent instances and their runtime state
CREATE TABLE IF NOT EXISTS agents (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    type        TEXT    NOT NULL,
    status      TEXT    NOT NULL DEFAULT 'idle',
    last_tick   TEXT,
    tick_count  INTEGER NOT NULL DEFAULT 0,
    error_count INTEGER NOT NULL DEFAULT 0,
    metadata    TEXT    NOT NULL DEFAULT '{}',
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- runes (brain): cognitive units with trust, TTL, level
CREATE TABLE IF NOT EXISTS runes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    glyph       TEXT    NOT NULL DEFAULT '',
    category    TEXT    NOT NULL DEFAULT 'general',
    level       INTEGER NOT NULL DEFAULT 1,
    trust       REAL    NOT NULL DEFAULT 1.0,
    usage_count INTEGER NOT NULL DEFAULT 0,
    error_count INTEGER NOT NULL DEFAULT 0,
    ttl_seconds INTEGER NOT NULL DEFAULT 0,
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
    rune_ids    TEXT    NOT NULL DEFAULT '[]',
    priority    INTEGER NOT NULL DEFAULT 0,
    active      INTEGER NOT NULL DEFAULT 1,
    usage_count INTEGER NOT NULL DEFAULT 0,
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- fusion_log: record of every pairwise rune fusion attempt
CREATE TABLE IF NOT EXISTS fusion_log (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    input_a     TEXT    NOT NULL,
    input_b     TEXT    NOT NULL,
    output_name TEXT    NOT NULL,
    score       REAL    NOT NULL DEFAULT 0.0,
    accepted    INTEGER NOT NULL DEFAULT 1,
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- artifacts: byproducts of fusion, GC, and agent activity (TTL-bound)
CREATE TABLE IF NOT EXISTS artifacts (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    type        TEXT    NOT NULL,
    source      TEXT    NOT NULL,
    payload     TEXT    NOT NULL DEFAULT '{}',
    ttl_seconds INTEGER NOT NULL DEFAULT 86400,
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- audit: immutable append-only event log
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

-- trust_ledger: append-only trust delta log
CREATE TABLE IF NOT EXISTS trust_ledger (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    subject      TEXT    NOT NULL,
    subject_type TEXT    NOT NULL DEFAULT 'rune',
    trust_delta  REAL    NOT NULL,
    reason       TEXT    NOT NULL DEFAULT '',
    created_at   TEXT    DEFAULT (datetime('now'))
);

-- size_assessment: table-growth snapshots
CREATE TABLE IF NOT EXISTS size_assessment (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    table_name TEXT    NOT NULL,
    row_count  INTEGER NOT NULL,
    sampled_at TEXT    DEFAULT (datetime('now'))
);
