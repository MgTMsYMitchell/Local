-- Migration 001 — initial schema
-- Part of the versioned migration chain run by Database::run_migrations().
-- Applied automatically on first open; safe to run manually too.

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
