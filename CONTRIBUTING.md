# Contributing to RN2 / RQ2

Thank you for your interest in contributing to RN2 — a local‑first, hallucination‑free AI agent ecosystem.

## Core Principles

- **Deterministic** — no hidden randomness in reasoning
- **Symbolic-first** — RSL, skills, Dewey codes
- **Local-first** — user owns their data
- **Privacy-safe** — no raw content leaves the node
- **Community-driven** — patterns, skills, adjacency shared safely
- **Extensible** — agents, handlers, UI, mesh

## How to Contribute

### 1. Fork the repository
Create a feature branch:

```
git checkout -b feature/my-feature
```

### 2. Follow the project structure
- `backend/` — tiny model + training
- `node/` — VM + librarian
- `ui/` — Three.js knowledge tree
- `config/` — mesh protocol + system config

### 3. Coding Guidelines
- TypeScript for node runtime
- Python for backend training
- Three.js for UI
- SQLite for persistence
- Keep code deterministic
- Avoid external dependencies unless essential

### 4. Submitting a PR
- Use the PR template
- Keep commits atomic
- Include tests where possible
- Document new skills, handlers, or patterns

### 5. Community Mesh Contributions
- Never include raw JSON or user data
- Only share pattern signatures, skills, adjacency
