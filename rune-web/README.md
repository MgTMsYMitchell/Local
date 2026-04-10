# rune-web — Node.js status UI

Minimal Express app that proxies the C++ backend and serves the Rune status UI.

## Dependencies

```powershell
winget install --id OpenJS.NodeJS.LTS -e   # Node.js >= 18
```

## Install & run

```powershell
cd rune-web
npm install
npm start
```

Open **http://localhost:3000** — the UI polls `/health` and `/runes` from the
C++ backend and renders them.

## Environment variables

| Variable | Default | Description |
|----------|---------|-------------|
| `PORT` | `3000` | Web UI listen port |
| `RUNE_BACKEND_HOST` | `127.0.0.1` | C++ backend host |
| `RUNE_BACKEND_PORT` | `7070` | C++ backend port |
