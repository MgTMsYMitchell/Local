-- Migration 002 — agent log
-- Tracks messages from rune-agents for audit / debugging.

CREATE TABLE IF NOT EXISTS agent_log (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    agent      TEXT    NOT NULL,        -- 'chat', 'node', 'wallet'
    level      TEXT    NOT NULL DEFAULT 'info',  -- info | warn | error
    message    TEXT    NOT NULL,
    created_at TEXT    DEFAULT (datetime('now'))
);
