# ᚠ QRrune Cognitive Node

> **A fully local-first cognitive engine built from runes, agents, nodes, and
> mycelial-style neural growth.**  
> The system behaves like a self-optimizing brain: it grows, prunes, fuses, and
> stabilizes over time.

This document is the **canonical reference** for the entire project — architecture,
schema, agents, cognitive model, build instructions, API, and Copilot Chat TODOs.

---

## Table of Contents

1. [High-Level Architecture](#1-high-level-architecture)
2. [Repository Layout](#2-repository-layout)
3. [Data Model (SQLite)](#3-data-model-sqlite)
4. [Agents — Organs of the System](#4-agents--organs-of-the-system)
5. [Cognitive Layer — Mycelium Model](#5-cognitive-layer--mycelium-model)
6. [System Flow](#6-system-flow)
7. [Heatmaps & Brain Visualization](#7-heatmaps--brain-visualization)
8. [Build & Run](#8-build--run)
9. [API Reference](#9-api-reference)
10. [LM Studio Integration](#10-lm-studio-integration)
11. [VS Code — Clone & Compile](#11-vs-code--clone--compile)
12. [C++ Dependency Sources](#12-c-dependency-sources)
13. [Copilot Chat TODO Board](#13-copilot-chat-todo-board)
14. [Project Philosophy](#14-project-philosophy)
15. [License](#15-license)

---

## 1. High-Level Architecture

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                          QRrune Cognitive Node                               │
│                                                                              │
│   Agents (organs)  ↔  Runes (neurons)  ↔  Skills (atoms)                   │
│                                                                              │
│   GC / Trust / Fusion  =  Neuroplasticity                                   │
│   Audit               =  Memory Traces                                       │
│   Overwatch           =  Meta-Cortex                                         │
│   SSE Dashboard       =  Sensory Cortex                                      │
│   SQLite (WAL)        =  Long-Term Memory                                    │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │               rune-backend  (C++20, two executables)                 │   │
│  │                                                                      │   │
│  │  rune        — original node · wallet · codec · HTTP :7070           │   │
│  │  rune_brain  — Cognitive Node · brain schema · all agents · HTTP :7071│  │
│  │                                                                      │   │
│  │  src/db/          database.hpp/.cpp     — SQLite WAL + migrations    │   │
│  │  src/brain/       brain_system.hpp/.cpp — canonical brain impl       │   │
│  │  src/encoding/    rune_codec.hpp        — Elder Futhark 24-rune      │   │
│  │  src/wallet/      wallet.hpp/.cpp       — Ed25519 via Monocypher     │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│           ▲                                    ▲                             │
│        HTTP :7070 / :7071               HTTP :7070 / :7071                  │
│           │                                    │                             │
│  ┌────────┴──────────┐          ┌──────────────┴──────────────────────┐     │
│  │   rune-web        │          │           rune-agents                │     │
│  │   (Node.js :3000) │          │                                      │     │
│  │  /health proxy    │          │  chat-agent.js   — REPL + LLM bridge │     │
│  │  /runes  proxy    │          │  node-agent.js   — health monitor    │     │
│  │  static UI        │          │  wallet-agent.js — Ed25519 key CLI   │     │
│  └───────────────────┘          └──────────────────────────────────────┘     │
│                                                                              │
│  rune.db / brain.db (SQLite WAL)                                            │
│    runes · agents · nodes · strategies · fusion_log · trust_ledger          │
│    node_events · artifacts · audit · size_assessment                        │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Repository Layout

```
Local/
├── rune-backend/
│   ├── CMakeLists.txt          FetchContent: sqlite3, monocypher, httplib, json
│   ├── src/
│   │   ├── main.cpp            rune entry-point (wallet, codec, RuneNode)
│   │   ├── brain_main.cpp      rune_brain entry-point (all cognitive agents)
│   │   ├── db/
│   │   │   ├── database.hpp    thread-safe SQLite wrapper + HealthCounts
│   │   │   └── database.cpp    WAL + PRAGMAs + versioned migrations
│   │   ├── brain/
│   │   │   ├── brain_system.hpp  ← CANONICAL REFERENCE (types + interfaces)
│   │   │   └── brain_system.cpp  ← CANONICAL IMPLEMENTATION (schema, agents)
│   │   ├── encoding/
│   │   │   └── rune_codec.hpp  header-only Elder Futhark encoder
│   │   ├── node/
│   │   │   └── rune_node.hpp   header-only threaded heartbeat node
│   │   └── wallet/
│   │       ├── wallet.hpp      Ed25519 keypair + sign/verify
│   │       └── wallet.cpp
│   └── db/
│       ├── schema.sql          reference schema
│       └── migrations/
│           ├── 001_initial.sql
│           ├── 002_agent_log.sql
│           └── 003_perf_indexes.sql
│
├── rune-web/                   Node.js Express status UI
│   ├── index.js                /health + /runes proxy + static /public
│   ├── public/index.html       dark-theme status page
│   └── package.json
│
├── rune-agents/                Node.js agents (zero npm deps beyond package.json)
│   ├── chat-agent.js           interactive REPL + LLM bridge
│   ├── node-agent.js           health-poll monitor / alerter
│   ├── wallet-agent.js         Ed25519 key CLI
│   └── package.json
│
├── scripts/
│   ├── bootstrap.ps1           Windows winget installer
│   ├── check-deps.ps1          Windows dependency check
│   └── check-deps.sh           Linux / Termux dependency check
│
└── .vscode/
    ├── settings.json           CMake source/build, C++20, format-on-save
    ├── tasks.json              Configure, Build, Clean, npm install/start
    ├── launch.json             Debug: rune (GDB/MSVC/LLDB), rune_brain, agents
    ├── extensions.json         Recommended: cpptools, cmake-tools, prettier, eslint
    └── c_cpp_properties.json   IntelliSense: Linux-GCC, Windows-MSVC, Windows-Clang
```

---

## 3. Data Model (SQLite)

Both `rune.db` (original node) and `brain.db` (cognitive node) use WAL mode with
performance pragmas: `synchronous=NORMAL`, `cache_size=-40000`, `temp_store=MEMORY`.

### Original node schema (`rune.db`)

| Table | Purpose |
|-------|---------|
| `runes` | Elder Futhark rune registry (name, glyph, meaning) |
| `macro_runes` | Composite rune sequences |
| `stories` | Narrative entries linked to runes |
| `mythic_moments` | Time-stamped mythic events |
| `wallet` | Single-row Ed25519 keypair store |
| `agent_log` | Structured log from Node.js agents |
| `schema_migrations` | Versioned migration tracker |

### Cognitive node schema (`brain.db`)

| Table | Purpose |
|-------|---------|
| `nodes` | Cognitive node registry — local + peer nodes |
| `agents` | Registered agent instances + runtime state |
| `runes` | Cognitive units with trust, TTL, level, usage/error counts |
| `strategies` | Named rune pathways (cognitive programs) |
| `fusion_log` | Record of every pairwise rune fusion attempt |
| `artifacts` | TTL-bound byproducts of fusion, GC, and agent activity |
| `audit` | Immutable append-only action log (never UPDATE/DELETE) |
| `node_events` | Async work queue consumed by WorkerAgent |
| `trust_ledger` | Append-only trust delta log aggregated by RuneTrustManager |
| `size_assessment` | Table-growth snapshots for compression benchmarks |

### Performance indexes (hot paths)

| Index | Covers |
|-------|--------|
| `idx_runes_trust` | `runes(trust)` — trust threshold scans |
| `idx_runes_active` | `runes(active)` — GC and fusion candidate queries |
| `idx_nevents_pending` | `node_events(processed, priority DESC, created_at)` — WorkerAgent polling |
| `idx_trust_subject` | `trust_ledger(subject, subject_type)` — RuneTrustManager aggregation |
| `idx_fusion_inputs` | `fusion_log(input_a, input_b)` — already-fused check |
| `idx_audit_ts` | `audit(created_at)` — time-range queries |
| `idx_sizeass_table` | `size_assessment(table_name, sampled_at)` — heatmap history |

---

## 4. Agents — Organs of the System

All agents subclass `AgentBase` and override `tick()`. The `run()` loop sleeps in
50 ms slices so shutdown is always responsive. Each agent self-registers in the
`agents` table on startup.

### ① HeartAgent *(Autonomic Heartbeat)*
- Updates `nodes` table with current vitals every **5 s**
- Emits `heartbeat` event with batched `AllCounts` (single SQL round-trip)
- Records `size_assessment` snapshot every 12 ticks (~1 min)
- **Status:** ✅ Implemented

### ② WorkerAgent *(Muscle Fibers)*
- Polls `node_events` queue (highest priority first, FIFO within priority)
- Handles: `heartbeat` · `chat_request` · `wallet_sign` · `strategy_execute` · `rune_trust`
- Marks events processed; records to `audit`
- Back-off: 100 ms (busy) / 1 000 ms (idle)
- **Status:** ✅ Implemented (stubs for chat + wallet dispatch)

### ③ GCAgent *(Microglia / Immune System)*
- Runs every **30 s**
- Prunes: expired runes (TTL), expired artifacts, old processed events (>24 h)
- Quarantines runes with `error_count ≥ 10`
- Snapshots `size_assessment`; logs everything to `audit`
- **Status:** ✅ Implemented

### ④ RuneTrustManager *(Neuroplasticity)*
- Runs every **10 s**
- Aggregates `trust_ledger` deltas and applies them to `runes.trust`
- Quarantines runes below trust floor (`< 0.1`)
- Promotes runes above trust ceiling (`≥ 3.0`) by +1 level, costs 1.0 trust
- **Status:** ✅ Implemented

### ⑤ RuneFusionEngine *(Cortical Folding)*
- Runs every **20 s**
- Finds active rune pairs with `trust ≥ 1.0` not already fused
- Fuses up to 3 pairs per tick → new rune + `fusion_log` entry + `fusion_residue` artifact
- Fusion score = `(trust_a + trust_b) / 2 × (level_a + level_b) / 2`
- Category fusion rules: `cognitive + elemental → arcane`, `trust + fusion → sovereign`, etc.
- **Status:** ✅ Implemented

### ⑥ ChatAgent *(Language / Speech Cortex)*
- EXTEND: handle `chat_request` events dispatched by WorkerAgent
- Integrate with LM Studio / Ollama via `RUNE_LLM_URL` (see `rune-agents/chat-agent.js`)
- **Status:** 🔲 Stub (idle loop)

### ⑦ WalletAgent *(Cryptographic Identity)*
- EXTEND: handle `wallet_sign` / `wallet_verify` events
- Reuse `wallet::sign()` / `wallet::verify()` from `src/wallet/wallet.hpp`
- **Status:** 🔲 Stub (idle loop)

### ⑧ NodeAgent *(Autonomic Nervous System)*
- EXTEND: poll peer nodes in `nodes` table, update `last_seen` + `status`
- Write trust deltas for unreachable peers via `BrainDb::record_trust()`
- **Status:** 🔲 Stub (idle loop)

### ⑨ TorrentAgent *(Swarm Behavior)*
- EXTEND: manage large artifact distribution across the node mesh
- Track pieces in `artifacts` table (`type = "torrent_piece"`)
- **Status:** 🔲 Stub (idle loop)

### ⑩ EdgeAgent *(Distributed Cognition)*
- EXTEND: handle low-latency edge tasks (QR decode, image hash, etc.)
- Post results as `edge_result` events for real-time visualization
- **Status:** 🔲 Stub (idle loop)

### ⑪ StrategyAgent *(Cross-Skill Intelligence)* — planned
- Extract reusable cognitive pathways from agent behavior patterns
- Store as `strategies` rows; suggest to WorkerAgent on matching events

### ⑫ EpicRuneAgent *(Mythic Layer)* — planned
- Detect high-trust, high-usage runes that qualify as "epic"
- Generate narrative descriptors + archive as `artifacts`

---

## 5. Cognitive Layer — Mycelium Model

```
Runes          =  Neurons / Mycelial nodes
Rune skills    =  Synapses / Hyphae connections
Strategies     =  Named pathways / Nutrient highways
Fusion         =  Cortical folding / Thickened mycelial cords
Trust          =  Synaptic strength
TTL state      =  Pruning / Rot cycle
GC             =  Microglia (removes dead or infected nodes)
Audit          =  Memory traces (immutable)
EventBus       =  Action potentials (SSE-style broadcast)
Overwatch      =  Meta-cortex (global reasoning — planned)
```

### Rune lifecycle

```
           insert_rune()
               ↓
           active (trust ≥ 0.1, ttl not elapsed)
               ↓  trust drops below 0.1
           quarantined (active = 0)
               ↓  GC sweep
           deleted
```

### Trust signals that feed RuneTrustManager

| Source | Delta | Reason |
|--------|-------|--------|
| WorkerAgent — strategy executed | `+0.2` per rune in pathway | `strategy_execute` |
| WorkerAgent — rune_trust event | caller-defined | `worker_event` |
| RuneFusionEngine — fusion birth | `score × 0.1` | `fusion_birth` |
| GCAgent — error threshold hit | implicit (quarantine, no ledger entry) | — |

### Category fusion table

| Input A | Input B | Output category |
|---------|---------|-----------------|
| cognitive | elemental | arcane |
| temporal | cognitive | prophetic |
| trust | fusion | sovereign |
| social | trust | covenant |
| physical | elemental | primal |
| gc | cognitive | sentinel |
| temporal | elemental | cyclical |
| fusion | social | mythic |
| anything else | anything else | composite |

---

## 6. System Flow

```
Startup
  ↓
BrainDb::ensure_schema()   — CREATE TABLE / INDEX IF NOT EXISTS (idempotent)
  ↓
BrainDb::seed_data()       — 24 runes + 7 strategies (skipped if populated)
  ↓
All agents register in `agents` table
  ↓
Threads start (one per agent)
  ↓
HeartAgent  ──► beats every 5 s ──► emits `heartbeat`, snapshots sizes
WorkerAgent ──► polls queue 100 ms–1 s ──► dispatches jobs ──► audits
RuneTrustManager ──► every 10 s ──► aggregates ledger ──► adjusts trust
RuneFusionEngine ──► every 20 s ──► fuses rune pairs ──► creates offspring
GCAgent     ──► every 30 s ──► prunes TTL / errors / stale events
  ↓
EventBus broadcasts to SSE /brain/events/stream
  ↓
Overwatch (planned) monitors all events and makes global adjustments
```

---

## 7. Heatmaps & Brain Visualization

### Visualization dimensions per rune

| Dimension | Visual encoding |
|-----------|----------------|
| `trust` | Brightness (brighter = higher trust) |
| `usage_count` (recent) | Heat colour (cool → warm → hot) |
| `error_count` (recent) | Red tint overlay |
| Fusion clusters | Thickened pathway lines between parent nodes |
| `active = 1` | Normal rendering |
| `active = 0` (quarantined) | Red halo / dimmed |
| TTL elapsed → deleted | Node fades out on GC sweep |

### Mycelium growth model

```
High trust + high usage  →  mycelial highway (thick, bright connection)
Fusion product           →  thick cord bridging parent nodes
Strategy pathway         →  nutrient hub (highlighted subgraph)
GC sweep                 →  nodes dissolve, connections sever
Trust collapse           →  node dims, halo turns red, quarantine
```

### Planned SSE endpoints (brain_main.cpp)

| Endpoint | Purpose |
|----------|---------|
| `GET /brain/events/recent?n=50` | JSON snapshot of last N events |
| `GET /brain/events/stream` | SSE stream (text/event-stream) |
| `GET /brain/health` | AllCounts in one SQL round-trip |
| `GET /brain/runes?limit=&offset=` | Paginated rune list |
| `GET /brain/fusion` | Fusion log |
| `GET /brain/size` | size_assessment history |
| `POST /brain/trust` | Record a trust delta |
| `POST /brain/events` | Enqueue a node_event |

---

## 8. Build & Run

### Windows — full sequence

```powershell
# 1. Install toolchain (one-time, elevated PowerShell)
powershell -ExecutionPolicy Bypass -File scripts\bootstrap.ps1

# 2. Verify
powershell -ExecutionPolicy Bypass -File scripts\check-deps.ps1

# 3. Clone
git clone https://github.com/MgTMsYMitchell/Local.git
cd Local

# 4. Build (fetches all C++ deps automatically on first configure)
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd ..

# 5a. Run original rune node (port 7070)
.\rune-backend\build\rune.exe

# 5b. Run cognitive brain node (port 7071)
.\rune-backend\build\rune_brain.exe brain.db 7071

# 6. Web UI
cd rune-web && npm install && npm start    # http://localhost:3000

# 7. Agents
cd rune-agents
node chat-agent.js      # REPL + optional LLM
node node-agent.js      # health monitor
node wallet-agent.js    # Ed25519 CLI
```

> **First configure** clones `sqlite3`, `monocypher`, `cpp-httplib`, `nlohmann/json`
> into `rune-backend/build/_deps/`. Requires internet on build machine; subsequent
> builds are fully offline.

### Linux / Termux

```bash
# Install deps
pkg install git cmake ninja nodejs sqlite clang

# Verify
bash scripts/check-deps.sh

# Build
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run
./build/rune &                          # port 7070
./build/rune_brain brain.db 7071 &      # port 7071

# Web UI
cd ../rune-web && npm install && node index.js &

# Agents
cd ../rune-agents && node chat-agent.js
```

---

## 9. API Reference

### Original rune node (`:7070`)

| Method | Path | Description |
|--------|------|-------------|
| GET | `/health` | Status + AllCounts + public key prefix |
| GET | `/runes[?limit=&offset=]` | Paginated rune list |
| GET | `/runes/encode?text=hello` | Latin → Elder Futhark glyphs |
| GET | `/wallet/pubkey` | Ed25519 public key (hex) |
| POST | `/wallet/sign` | `{"message":"…"}` → `{"signature":"<hex>"}` |
| POST | `/wallet/verify` | `{"message":"…","signature":"<hex>"}` → `{"valid":bool}` |
| POST | `/shutdown` | Graceful stop |

### Cognitive brain node (`:7071`)

| Method | Path | Description |
|--------|------|-------------|
| GET | `/brain/health` | AllCounts (one SQL round-trip) |
| GET | `/brain/runes[?limit=&offset=]` | Paginated rune list with trust/level |
| GET | `/brain/runes/:name` | Single rune detail |
| POST | `/brain/runes` | Create rune |
| GET | `/brain/strategies` | All strategies (priority-sorted) |
| POST | `/brain/strategies` | Create strategy |
| GET | `/brain/fusion` | Fusion log |
| POST | `/brain/trust` | Record trust delta |
| GET | `/brain/trust` | Aggregate trust summary |
| GET | `/brain/audit[?limit=50]` | Audit log |
| GET | `/brain/size` | size_assessment history |
| POST | `/brain/events` | Enqueue a node_event |
| GET | `/brain/events/recent[?n=50]` | Last N events (JSON snapshot) |
| GET | `/brain/events/stream` | SSE live stream (`text/event-stream`) |
| POST | `/brain/shutdown` | Graceful stop |

---

## 10. LM Studio Integration

```powershell
# Windows
set RUNE_LLM_URL=http://localhost:1234/v1/chat/completions
set RUNE_LLM_MODEL=your-model-name
node rune-agents\chat-agent.js
```

```bash
# Linux / Termux
RUNE_LLM_URL=http://localhost:1234/v1/chat/completions \
RUNE_LLM_MODEL=your-model-name \
node rune-agents/chat-agent.js
```

When `RUNE_LLM_URL` is set the chat agent forwards every message to the LLM
endpoint and displays the reply. Without it the agent runs in echo mode.

---

## 11. VS Code — Clone & Compile

The `.vscode/` directory is fully pre-configured:

```bash
git clone https://github.com/MgTMsYMitchell/Local.git
code Local
# → Accept recommended extensions when prompted
# → Ctrl+Shift+B  →  "CMake: Build"  (configures + builds both executables)
# → F5  →  choose a launch config to debug
```

| Launch config | Target |
|---------------|--------|
| `rune — Debug (Linux / Termux)` | `rune` via GDB |
| `rune — Debug (Windows MSVC)` | `rune.exe` via vsdbg |
| `rune_brain — Debug (Linux / Termux)` | `rune_brain` via GDB |
| `rune_brain — Debug (Windows MSVC)` | `rune_brain.exe` via vsdbg |
| `chat-agent (Node.js)` | `chat-agent.js` |
| `node-agent (Node.js)` | `node-agent.js` |
| `wallet-agent (Node.js)` | `wallet-agent.js` |
| `rune-web (Node.js)` | `index.js` |

---

## 12. C++ Dependency Sources

All fetched automatically by CMake `FetchContent` on first configure — no manual
installs required beyond the build toolchain.

| Library | Source | Version | Role |
|---------|--------|---------|------|
| [sqlite/sqlite](https://github.com/sqlite/sqlite) | amalgamation (C) | 3.45.2 | SQLite database |
| [LoupVaillant/Monocypher](https://github.com/LoupVaillant/Monocypher) | C | 3.1.3 | Ed25519 sign/verify |
| [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib) | header-only C++ | v0.15.3 | Embedded HTTP server + SSE |
| [nlohmann/json](https://github.com/nlohmann/json) | header-only C++ | v3.11.3 | JSON serialization |

---

## 13. Copilot Chat TODO Board

> Open `src/brain/brain_system.hpp` and look for `// EXTEND:` markers to find
> the exact insertion points for each item below.

### A. Agents

- [ ] **ChatAgent** — real LM Studio / Ollama client (`RUNE_LLM_URL`); persist replies to `audit`; emit `chat_response`
- [ ] **WalletAgent** — real Ed25519 signing via `wallet::sign()` from `src/wallet/wallet.hpp`; handle `wallet_verify` events
- [ ] **NodeAgent** — stale heartbeat detection; poll `/brain/health` on peers; write trust deltas for unreachable nodes
- [ ] **HousekeepingAgent** — `PRAGMA optimize`; rebuild indexes; archive `audit` rows older than 30 days
- [ ] **OverwatchAgent** — global reasoning layer; spawn new agents or simulations based on event patterns
- [ ] **StrategyAgent** — extract reusable strategies from WorkerAgent behavior; store in `strategies` table
- [ ] **EpicRuneAgent** — detect high-trust / high-usage runes; generate narrative + glyph; archive as `artifacts`
- [ ] **LoadSimulatorAgent** — synthetic job/node/rune generation for throughput + stability measurement

### B. Cognitive Layer

- [ ] JIT/compiled promotion pipeline — promote level-10 runes to a "compiled" tier with faster lookup
- [ ] Mutation engine — introduce small random trust perturbations to escape local optima
- [ ] Reversible fusion — store full parent state in `fusion_log` so GC can un-fuse on trust collapse
- [ ] Strategy extraction — auto-infer `strategies` rows from co-occurring rune activation patterns
- [ ] Multi-way fusion — extend `RuneFusionEngine` beyond pairwise to triplet fusions

### C. Visualization

- [ ] SSE server (`brain_main.cpp`) — wire `EventBus` subscription to `GET /brain/events/stream`
- [ ] Dashboard — node map, rune heatmap, fusion graph, audit timeline, trust sparklines
- [ ] Mermaid diagram — auto-generate from live `strategies` and `fusion_log` data
- [ ] Heatmap export — `GET /brain/heatmap.json` returning rune positions + colour values

### D. Distribution

- [ ] **TorrentAgent** — piece tracking in `artifacts`; replication across peer nodes
- [ ] **EdgeAgent** — QR decode, image hash, edge-compute results as `edge_result` events
- [ ] Peer sync — `NodeAgent` syncs `trust_ledger` deltas with peer brain nodes

### E. Simulation & Testing

- [ ] **LoadSimulatorAgent** — stress-test matrix: N agents × M runes × K strategies
- [ ] Simulation metrics — throughput, queue depth, trust convergence time in `size_assessment`
- [ ] Audit summaries — periodic digest of `audit` grouped by agent + action

---

## 14. Project Philosophy

This system is designed to behave like a **living cognitive organism**:

| Biological analogy | System component |
|--------------------|-----------------|
| Neurons firing | Runes activated by WorkerAgent strategies |
| Synaptic strengthening | Trust scores increasing via RuneTrustManager |
| Cortical folding | Rune fusion creating higher-order cognitive units |
| Mycelial highways | High-trust strategy pathways between rune clusters |
| Memory consolidation | Audit log + size_assessment snapshots |
| Neuroplasticity | GC pruning + trust promotion/demotion |
| Immune response | GCAgent quarantining high-error runes |
| Meta-cognition | OverwatchAgent (planned) observing global state |
| Sensory cortex | SSE EventBus + real-time dashboard |

The architecture is intentionally **biological, mycelial, and neural** — not a
fixed pipeline but an adaptive, self-organizing network that improves with use.

---

## 15. License

MIT — see [LICENSE](LICENSE) or add your preferred license file.

---

*This README is auto-referenced by `brain_system.hpp` and `brain_system.cpp` via
their `INTENT FOR GITHUB COPILOT CHAT / AI` header blocks. Keep them in sync.*
