# ᚠ Rune System

A local-first AI coding environment with a threaded C++ backend node, minimal
web status UI, Ed25519 wallet, SQLite persistence, and Node.js agents.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          Rune System Architecture                           │
│                                                                             │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                      rune-backend  (C++20)                           │   │
│  │                                                                      │   │
│  │  main.cpp ──► RuneNode ×2 (threads) ──► Database (SQLite WAL)       │   │
│  │           ──► httplib Server (:7070)                                  │   │
│  │               GET  /health  /runes  /runes/encode?text=              │   │
│  │               GET  /wallet/pubkey                                    │   │
│  │               POST /wallet/sign   /wallet/verify   /shutdown         │   │
│  │                                                                      │   │
│  │  src/db/database.hpp/.cpp    — SQLite + migrations                   │   │
│  │  src/encoding/rune_codec.hpp — Elder Futhark 24-rune table + encode  │   │
│  │  src/node/rune_node.hpp      — threaded heartbeat node               │   │
│  │  src/wallet/wallet.hpp/.cpp  — Ed25519 via Monocypher                │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│           ▲                          ▲                                       │
│  HTTP :7070                    HTTP :7070                                    │
│           │                          │                                       │
│  ┌────────┴──────────┐   ┌──────────┴──────────────────────────────────┐    │
│  │   rune-web        │   │         rune-agents                         │    │
│  │   (Node.js :3000) │   │                                             │    │
│  │                   │   │  chat-agent.js   — REPL + LLM bridge        │    │
│  │  /health proxy    │   │  node-agent.js   — health monitor/alerter   │    │
│  │  /runes  proxy    │   │  wallet-agent.js — Ed25519 key CLI          │    │
│  │  static UI        │   └─────────────────────────────────────────────┘    │
│  └───────────────────┘                                                       │
│                                                                             │
│  rune.db (SQLite)                                                           │
│    runes · macro_runes · stories · mythic_moments · wallet · agent_log     │
│    schema_migrations (versioned, idempotent)                               │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Repository layout

```
Local/
├── rune-backend/           C++20 backend node
│   ├── CMakeLists.txt      FetchContent: sqlite3, monocypher, httplib, nlohmann/json
│   ├── src/
│   │   ├── main.cpp        entry point — wires all modules
│   │   ├── db/             database.hpp / database.cpp
│   │   ├── encoding/       rune_codec.hpp (header-only, Elder Futhark)
│   │   ├── node/           rune_node.hpp  (header-only, threaded worker)
│   │   └── wallet/         wallet.hpp / wallet.cpp (Ed25519)
│   └── db/
│       ├── schema.sql      full schema (reference)
│       └── migrations/     001_initial.sql  002_agent_log.sql
│
├── rune-web/               Node.js Express UI
│   ├── index.js            /health + /runes proxy, static /public
│   ├── public/index.html   dark-theme status page
│   └── package.json
│
├── rune-agents/            Node.js agents (zero npm deps)
│   ├── chat-agent.js       interactive REPL + optional LLM
│   ├── node-agent.js       health-poll monitor
│   ├── wallet-agent.js     Ed25519 key CLI
│   └── package.json
│
└── scripts/
    ├── check-deps.ps1      Windows dependency check
    ├── check-deps.sh       Linux / Termux dependency check
    └── bootstrap.ps1       Windows winget installer
```

---

## C++ dependency sources (all cloned at CMake configure time)

| Library | Repo | Tag | Role |
|---------|------|-----|------|
| [sqlite/sqlite](https://github.com/sqlite/sqlite) | amalgamation | `version-3.45.2` | SQLite database |
| [LoupVaillant/Monocypher](https://github.com/LoupVaillant/Monocypher) | C | `3.1.3` | Ed25519 sign/verify |
| [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib) | header-only | `v0.15.3` | embedded HTTP server |
| [nlohmann/json](https://github.com/nlohmann/json) | header-only | `v3.11.3` | JSON serialization |

---

## Windows — full build & run sequence

### 1. Install toolchain (one-time)

```powershell
# Elevated PowerShell
powershell -ExecutionPolicy Bypass -File scripts\bootstrap.ps1
# Restart terminal after to pick up PATH
```

Or install manually:
```powershell
winget install --id Git.Git                                -e
winget install --id Kitware.CMake                          -e
winget install --id Ninja-build.Ninja                      -e
winget install --id OpenJS.NodeJS.LTS                      -e
winget install --id Microsoft.VisualStudio.2022.BuildTools -e
```

### 2. Verify dependencies

```powershell
powershell -ExecutionPolicy Bypass -File scripts\check-deps.ps1
```

### 3. Clone (if not already)

```powershell
git clone https://github.com/MgTMsYMitchell/Local.git
cd Local
```

### 4. Build backend (C++20 + all deps fetched automatically)

```powershell
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd ..
```

> First configure will clone sqlite3, monocypher, cpp-httplib, nlohmann/json
> into `rune-backend/build/_deps/`.

### 5. Run backend

```powershell
# Terminal 1
.\rune-backend\build\rune.exe          # rune.db on :7070 (defaults)
.\rune-backend\build\rune.exe mydb.db wallet.json 8080  # custom
```

### 6. Run web UI

```powershell
# Terminal 2
cd rune-web
npm install
npm start
# Open http://localhost:3000
```

### 7. Run agents

```powershell
# Terminal 3 — chat agent
cd rune-agents
node chat-agent.js

# Terminal 4 — node monitor
node node-agent.js

# Terminal 5 — wallet CLI
node wallet-agent.js
```

---

## Termux + proot Ubuntu (secondary node)

```bash
# Install deps
pkg install git cmake ninja nodejs sqlite zstd curl clang

# Verify
bash scripts/check-deps.sh

# Build backend
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run
./build/rune &

# Web UI
cd ../rune-web && npm install && node index.js &

# Agents
cd ../rune-agents
node chat-agent.js
```

---

## API reference

| Method | Path | Description |
|--------|------|-------------|
| GET | `/health` | Status + row counts + public key prefix |
| GET | `/runes` | All runes (JSON array) |
| GET | `/runes/encode?text=hello` | Latin → Elder Futhark glyphs |
| GET | `/wallet/pubkey` | Ed25519 public key (hex) |
| POST | `/wallet/sign` | `{"message":"..."}` → `{"signature":"<hex>"}` |
| POST | `/wallet/verify` | `{"message":"...","signature":"<hex>"}` → `{"valid":bool}` |
| POST | `/shutdown` | Graceful stop |

---

## LM Studio integration (optional)

Start LM Studio, load any GGUF model, enable the local server, then:

```powershell
set RUNE_LLM_URL=http://localhost:1234/v1/chat/completions
set RUNE_LLM_MODEL=your-model-name
node rune-agents\chat-agent.js
```

The chat agent will send your messages to LM Studio and display the replies.
