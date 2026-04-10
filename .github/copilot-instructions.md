# Copilot Instructions — QRrune Cognitive Node

## Project owner
**MgTMsYMitchell** — GitHub username for all commit attribution and references.

## Architecture
- **rune-backend** (C++20): Two executables — `rune` (original node, :7070) and `rune_brain` (cognitive node, :7071).
- **rune-agents** (Node.js): Zero-dependency CLI agents — chat, node monitor, wallet.
- **rune-web** (Node.js + Express): Status dashboard proxying the backend.
- **SQLite WAL mode** everywhere with `synchronous=NORMAL`, 40 MB page cache, `temp_store=MEMORY`.

## Coding conventions
- C++20 standard, no exceptions in hot paths, `sqlite3_bind_*` for all queries (no string concatenation).
- All cognitive agents subclass `AgentBase` and override `tick()`.
- New agents register via `BrainDb::register_agent()` in `brain_main.cpp`.
- Async work goes through `BrainDb::enqueue_event()` → `WorkerAgent` dispatch.
- Events broadcast via `AgentBase::emit()` → `EventBus` → SSE stream.
- Node.js agents use only Node built-ins (http, crypto, readline, fs, path).
- Prefer header-only libraries; all C++ deps fetched via CMake FetchContent.

## Extension points
- Look for `// EXTEND:` markers in `brain_system.hpp` for agent implementation guidance.
- Look for `// EXTEND:` markers in `brain_system.cpp` WorkerAgent::handle() for new event types.

## Build
```bash
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Biological metaphor
Runes = neurons, strategies = pathways, fusion = cortical folding, trust = synaptic strength, GC = microglia, audit = memory traces, EventBus = action potentials.
