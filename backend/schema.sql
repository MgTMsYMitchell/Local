PRAGMA journal_mode = WAL;
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS skills (
  skill_idx      INTEGER PRIMARY KEY,
  name           TEXT NOT NULL,
  tags           TEXT,
  source         TEXT NOT NULL DEFAULT 'local',
  version        INTEGER NOT NULL DEFAULT 1,
  created_at     TEXT NOT NULL,
  updated_at     TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS patterns (
  signature_hash TEXT PRIMARY KEY,
  opcode_seq     BLOB NOT NULL,
  role_seq       BLOB NOT NULL,
  skill_seq      BLOB NOT NULL,
  skill_idx      INTEGER NOT NULL,
  dewey_code     TEXT,
  count_local    INTEGER NOT NULL DEFAULT 0,
  count_community INTEGER NOT NULL DEFAULT 0,
  is_native      INTEGER NOT NULL DEFAULT 0,
  native_handler_id TEXT,
  created_at     TEXT NOT NULL,
  updated_at     TEXT NOT NULL,
  FOREIGN KEY (skill_idx) REFERENCES skills(skill_idx)
);

CREATE TABLE IF NOT EXISTS adjacency (
  skill_a    INTEGER NOT NULL,
  skill_b    INTEGER NOT NULL,
  weight     REAL NOT NULL,
  updated_at TEXT NOT NULL,
  PRIMARY KEY (skill_a, skill_b),
  FOREIGN KEY (skill_a) REFERENCES skills(skill_idx),
  FOREIGN KEY (skill_b) REFERENCES skills(skill_idx)
);

CREATE TABLE IF NOT EXISTS handlers (
  handler_id       TEXT PRIMARY KEY,
  signature_hash   TEXT NOT NULL,
  language         TEXT NOT NULL,
  path             TEXT NOT NULL,
  created_at       TEXT NOT NULL,
  FOREIGN KEY (signature_hash) REFERENCES patterns(signature_hash)
);

CREATE TABLE IF NOT EXISTS community_updates (
  update_id   TEXT PRIMARY KEY,
  payload     TEXT NOT NULL,
  applied     INTEGER NOT NULL DEFAULT 0,
  timestamp   TEXT NOT NULL
);
