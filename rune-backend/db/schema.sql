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
