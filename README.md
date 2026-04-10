# RN2 — Local‑First, Hallucination‑Free AI Agent Ecosystem

RN2 is a deterministic, symbolic, self‑organising AI system built on RSL (Rune Structured Language), skill dictionaries, Dewey/QR‑rune classification, pattern signatures, tiny transformer models trained from scratch, a Three.js "Living Knowledge Tree" UI, and a mesh sync protocol for community learning.

---

## Directory Structure

```
rn2/
  backend/
    rn2_model.py      # PyTorch tiny transformer
    train_rn2.py      # Synthetic training loop
    schema.sql        # SQLite schema
    data/             # Runtime database (git-ignored)
    models/           # Saved model weights (git-ignored)
  node/
    src/
      vm.ts           # RSL VM
      index.ts        # Runtime entry point
    package.json
    tsconfig.json
  ui/
    src/
      main.ts         # Three.js Knowledge Tree
    vite.config.ts
    package.json
  config/
    config.yaml
    mesh_protocol.json
  rn2-bootstrap-all.ps1
```

---

## Quick Start (Windows 11 + PowerShell 7)

```powershell
pwsh -ExecutionPolicy Bypass -File .\rn2-bootstrap-all.ps1
```

### Train the model

```powershell
cd backend
.\.venv\Scripts\Activate.ps1
python train_rn2.py
```

### Run the node runtime

```powershell
cd node
npx ts-node src/index.ts
```

### Run the UI

```powershell
cd ui
npx vite
```

---

## License

MIT
