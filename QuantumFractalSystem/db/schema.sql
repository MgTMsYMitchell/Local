-- QuantumFractalSystem — SQLite Schema
-- Apply with:  sqlite3 qfs.db < schema.sql

PRAGMA journal_mode = WAL;
PRAGMA synchronous  = NORMAL;
PRAGMA cache_size   = -40000;   -- 40 MB page cache
PRAGMA temp_store   = MEMORY;

-- Mesh nodes participating in the distributed compute network
CREATE TABLE IF NOT EXISTS nodes (
    id             TEXT PRIMARY KEY,
    alignment      BLOB,              -- serialised Vec14 (56 bytes)
    compressed_state BLOB,            -- QState snapshot
    status         TEXT    NOT NULL DEFAULT 'active',
    trust          REAL    NOT NULL DEFAULT 1.0,
    last_seen      TEXT    DEFAULT (datetime('now')),
    created_at     TEXT    DEFAULT (datetime('now'))
);

-- Compute tasks dispatched across the mesh
CREATE TABLE IF NOT EXISTS tasks (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id     TEXT    REFERENCES nodes(id),
    payload     BLOB,
    status      TEXT    NOT NULL DEFAULT 'pending',
    result      BLOB,
    created_at  TEXT    DEFAULT (datetime('now')),
    completed_at TEXT
);

-- Alignment field snapshots for self-healing diagnostics
CREATE TABLE IF NOT EXISTS alignment_snapshots (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id     TEXT    REFERENCES nodes(id),
    field_data  BLOB,              -- serialised Tensor14 (784 bytes)
    drift       BLOB,              -- serialised Vec14 (56 bytes)
    sampled_at  TEXT    DEFAULT (datetime('now'))
);

-- Routing table cache
CREATE TABLE IF NOT EXISTS routes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    from_node   TEXT    NOT NULL,
    to_node     TEXT    NOT NULL,
    via_node    TEXT,
    hops        INTEGER NOT NULL DEFAULT 1,
    latency_ms  REAL,
    updated_at  TEXT    DEFAULT (datetime('now'))
);

-- Event log (append-only audit trail)
CREATE TABLE IF NOT EXISTS events (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    source      TEXT    NOT NULL,
    event_type  TEXT    NOT NULL,
    payload     TEXT    NOT NULL DEFAULT '{}',
    created_at  TEXT    DEFAULT (datetime('now'))
);
