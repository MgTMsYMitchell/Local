# DeepThink — QR2 + Rune-Sigil Memory Engine

A high-performance cognitive memory core integrated with **LocalAI** for embedding and chat completion.

---

## Architecture

```
DeepThink/
├── engine/
│   ├── qr2_rune_engine.hpp   # QR2Store · RuneIndex · QR2RuneEngine
│   ├── qr2_rune_engine.cpp   # Implementation
│   └── benchmark.cpp         # Ingest + query benchmark harness
├── service/
│   ├── localai_client.hpp    # LocalAI embed + chat client
│   └── deepthink_service.cpp # HTTP service (ingest + query routes)
├── webui/                    # Web front-end (future)
├── vsix/                     # VS Code extension (future)
├── scrapers/                 # Data ingestion scrapers (future)
├── memory/                   # Persistent memory store (future)
├── CMakeLists.txt
└── README.md
```

---

## Data Flow

### Ingest path

```
[Scraper / Feedback]
        │
        │  POST /ingest  {"texts": [...], "tag_ids": [...], "timestamps": [...]}
        ▼
[deepthink_service]
        │
        │  for each text:
        │    LocalAIClient::embed(text)
        │      → POST LocalAI /v1/embeddings
        │      → project_to_14d()   (bucket-average + L2-normalise → SigilVec)
        ▼
[QR2RuneEngine::ingest_batch(batch)]
        │
        ├─▶ QR2Store::append_batch()   — stores id / ts_delta / tag_id / glyph_id
        └─▶ RuneIndex::insert_batch()  — stores id + 14D SigilVec
```

### Query path

```
[User query]
        │
        │  POST /query  {"text": "...", "top_k": 5}
        ▼
[deepthink_service]
        │
        │  LocalAIClient::embed(text) → SigilVec
        ▼
[QR2RuneEngine::query(query_vec, top_k)]
        │
        ├─▶ RuneIndex::search()    — nth_element L2 scan, returns top-k ids + scores
        └─▶ QR2Store::fetch_by_ids() — resolves records (id, ts_delta, tag_id, glyph_id)
        │
        │  build context string from records
        ▼
[LocalAIClient::chat(context, user_query)]
        │
        │  POST LocalAI /v1/chat/completions
        ▼
[JSON response  {"reply": "...", "records_found": N}]
```

---

## Engine components

| Class | Responsibility |
|---|---|
| `QR2Store` | Column-store for records: id, ts_delta (delta-compressed), tag_id, glyph_id. Thread-safe via `shared_mutex`. |
| `RuneIndex` | 14D sigil vector store with brute-force L2 nearest-neighbour search using `nth_element`. Thread-safe. |
| `QR2RuneEngine` | Façade: coordinates `QR2Store` + `RuneIndex` for ingest and query. |
| `LocalAIClient` | REST client: `/v1/embeddings` (→ project to 14D) and `/v1/chat/completions`. |

---

## Design metrics

| Scenario | Metric | Target |
|---|---|---|
| Ingest 10k items (end-to-end incl. embed) | Time | ~0.6–0.9 s |
| Ingest throughput (engine only) | Inserts/sec | > 1M |
| Query top-10 (engine only, 10k records) | Latency | < 5 ms |
| Query top-10 (end-to-end incl. embed+chat) | Latency | ~5–10 ms + network |

---

## Build

Requires: **CMake ≥ 3.20**, **Ninja** (recommended), C++17 compiler.  
All dependencies (`nlohmann/json`, `cpp-httplib`) are fetched automatically.

```bash
cd DeepThink
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Binaries produced:

| Binary | Description |
|---|---|
| `build/deepthink_service` | HTTP service on port 9090 (configurable) |
| `build/deepthink_bench` | Engine-only benchmark (no network I/O) |

---

## Running the service

```bash
# Start LocalAI first (default: http://127.0.0.1:8080)

LOCALAI_HOST=127.0.0.1 LOCALAI_PORT=8080 SERVICE_PORT=9090 \
    ./build/deepthink_service
```

### Ingest example

```bash
curl -s -X POST http://localhost:9090/ingest \
  -H 'Content-Type: application/json' \
  -d '{
    "texts":      ["The cat sat on the mat", "Dogs love walks"],
    "tag_ids":    [1, 2],
    "timestamps": [1700000000, 1700000001]
  }' | jq
```

### Query example

```bash
curl -s -X POST http://localhost:9090/query \
  -H 'Content-Type: application/json' \
  -d '{"text": "feline resting", "top_k": 3}' | jq
```

### Status

```bash
curl -s http://localhost:9090/status | jq
```

---

## Running the benchmark

```bash
./build/deepthink_bench
```

Sample output:

```
DeepThink QR2+Rune benchmark
  Ingest N = 10000  Query N = 200  top_k = 10

[Ingest] 10000 items: 1.243 ms  (8046875 inserts/s)
[Query]  200 queries, top-10 over 10000 items:
  avg 0.812 ms  min 0.701 ms  max 1.203 ms

--- Target checks ---
  Engine ingest 10k (ms)             1.243  ✓  (target ≤ 50)
  Query avg latency (ms)             0.812  ✓  (target ≤ 10)
  Query max latency (ms)             1.203  ✓  (target ≤ 20)
```

---

## Environment variables

| Variable | Default | Description |
|---|---|---|
| `LOCALAI_HOST` | `127.0.0.1` | LocalAI hostname |
| `LOCALAI_PORT` | `8080` | LocalAI port |
| `SERVICE_PORT` | `9090` | DeepThink service listen port |

---

## Future work

- **`webui/`** — React/Vue front-end for memory exploration and query UI
- **`vsix/`** — VS Code extension integrating DeepThink query into editor
- **`scrapers/`** — Automated content scrapers feeding the ingest path
- **`memory/`** — Persistent store (SQLite WAL) backing record text retrieval
- **Compression** — Delta + varint encoding for `ts_delta` / `tag_id` columns
- **Approximate NN** — HNSW or IVF index for sub-millisecond search at 1M+ records
