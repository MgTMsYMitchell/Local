# QRrune Documentation

Architecture reference, API guides, and system diagrams for the
**QRrune Cognitive Node** (the `rune` + `rune_brain` executables) and the
broader **Quantum Fractal System (QFS)**.

## Contents

```
docs/
├── architecture/          # Component architecture documents with Mermaid diagrams
│   ├── 01-system-overview.md
│   ├── 02-agent-framework.md
│   ├── 03-rqr2-module.md
│   ├── 04-llm-integration.md
│   ├── 05-self-healing.md
│   ├── 06-mesh-networking.md
│   ├── 07-storage.md
│   ├── 08-compute-pipeline.md
│   └── 09-cicd-pipeline.md
└── diagrams/
    └── README.md          # Diagram index + static export instructions
```

## Quick links

| Topic | File |
|---|---|
| Full system picture | [01-system-overview.md](architecture/01-system-overview.md) |
| All agents & extension guide | [02-agent-framework.md](architecture/02-agent-framework.md) |
| 14D symbol encoding | [03-rqr2-module.md](architecture/03-rqr2-module.md) |
| LLM / chat integration | [04-llm-integration.md](architecture/04-llm-integration.md) |
| Self-healing & recovery | [05-self-healing.md](architecture/05-self-healing.md) |
| Mesh / peer networking | [06-mesh-networking.md](architecture/06-mesh-networking.md) |
| Database schema | [07-storage.md](architecture/07-storage.md) |
| Compute & task queue | [08-compute-pipeline.md](architecture/08-compute-pipeline.md) |
| Build & CI/CD | [09-cicd-pipeline.md](architecture/09-cicd-pipeline.md) |

## Build

```bash
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
# → build/rune      (:7070)
# → build/rune_brain (:7071)
```

## Runtime

```bash
# Cognitive node (brain)
RUNE_LLM_URL=http://localhost:11434/v1 ./build/rune_brain

# Main node
./build/rune
```

Set `RUNE_LLM_URL` to any OpenAI-compatible endpoint. Omit it for echo-mode.
