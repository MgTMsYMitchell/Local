-- Migration 001: Initial schema
-- Applied automatically on first run

-- See schema.sql for full reference.
-- This file duplicates the DDL for migration tracking.

CREATE TABLE IF NOT EXISTS nodes (
    id             TEXT PRIMARY KEY,
    alignment      BLOB,
    compressed_state BLOB,
    status         TEXT    NOT NULL DEFAULT 'active',
    trust          REAL    NOT NULL DEFAULT 1.0,
    last_seen      TEXT    DEFAULT (datetime('now')),
    created_at     TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS tasks (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id     TEXT    REFERENCES nodes(id),
    payload     BLOB,
    status      TEXT    NOT NULL DEFAULT 'pending',
    result      BLOB,
    created_at  TEXT    DEFAULT (datetime('now')),
    completed_at TEXT
);

CREATE TABLE IF NOT EXISTS alignment_snapshots (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id     TEXT    REFERENCES nodes(id),
    field_data  BLOB,
    drift       BLOB,
    sampled_at  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS routes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    from_node   TEXT    NOT NULL,
    to_node     TEXT    NOT NULL,
    via_node    TEXT,
    hops        INTEGER NOT NULL DEFAULT 1,
    latency_ms  REAL,
    updated_at  TEXT    DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS events (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    source      TEXT    NOT NULL,
    event_type  TEXT    NOT NULL,
    payload     TEXT    NOT NULL DEFAULT '{}',
    created_at  TEXT    DEFAULT (datetime('now'))
);

-- Migration metadata
CREATE TABLE IF NOT EXISTS schema_migrations (
    version     INTEGER PRIMARY KEY,
    applied_at  TEXT    DEFAULT (datetime('now'))
);

INSERT OR IGNORE INTO schema_migrations (version) VALUES (1);
