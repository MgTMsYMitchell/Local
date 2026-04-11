# QuantumFractalSystem

A distributed compute mesh system optimised for laptops and small devices.

## Architecture

```
QuantumFractalSystem/
├── src/
│   ├── main.cpp              — Entry point
│   ├── app/                  — Application orchestrator
│   ├── compression/          — Quantum-inspired data compression
│   ├── math/                 — 14D vector/tensor/alignment math
│   ├── network/              — Fractal mesh topology & routing
│   ├── engine/               — Self-healing + compute engines
│   ├── cpu/                  — SIMD-accelerated CPU pipeline
│   ├── gpu/                  — CUDA GPU kernels (optional)
│   └── storage/              — SQLite storage layer (WAL mode)
├── tests/                    — GoogleTest suite
├── db/                       — Schema & migrations
├── config/                   — Runtime configuration
├── Dockerfile                — Container build
└── docker-compose.yml        — Orchestration
```

## Modules

| Module | Purpose |
|--------|---------|
| **math** | 14-dimensional vectors (`Vec14`), tensors (`Tensor14`), and alignment field operations |
| **compression** | Quantum-inspired compressor: encode data as superpositions, entangle, collapse |
| **network** | Fractal mesh topology with BFS routing and message passing |
| **engine** | Self-healing (corruption detection, recursive repair) and parallel compute |
| **cpu** | SIMD-ready CPU compute pipeline (FMA, dot product, normalisation) |
| **gpu** | Optional CUDA kernels for GPU-accelerated computation |
| **storage** | SQLite database with WAL mode, thread-safe queries, JSON export |

## Build

```bash
cd QuantumFractalSystem
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Run tests

```bash
cd build && ctest --output-on-failure
```

### Enable GPU (requires CUDA toolkit)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DQFS_ENABLE_GPU=ON
cmake --build build --parallel
```

## Docker

```bash
cd QuantumFractalSystem
docker compose up --build
```

## Database

SQLite with WAL mode. Schema in `db/schema.sql`. Tables:

- `nodes` — mesh node registry with alignment vectors
- `tasks` — distributed compute task queue
- `alignment_snapshots` — field diagnostics
- `routes` — routing table cache
- `events` — append-only audit log

## Configuration

Edit `config/app_config.json`:

```json
{
    "database":    { "path": "qfs.db", "wal_mode": true },
    "network":     { "listen_port": 9090, "max_peers": 64 },
    "compute":     { "thread_pool_size": 4, "use_gpu": false },
    "healing":     { "check_interval_ms": 10000, "corruption_threshold": 0.3 },
    "compression": { "basis_size": 32 }
}
```

## Extending

- Add new compute kernels in `src/cpu/` or `src/gpu/`
- Add new node behaviours by extending `MeshNode`
- Add new routing strategies by extending `RoutingTable`
- Add new healing heuristics in `SelfHealingEngine`
