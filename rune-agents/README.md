# rune-agents — Chat · Node Monitor · Brain Monitor · Wallet

Four Node.js CLI agents for the Rune System.  
No npm dependencies — uses Node.js built-ins only (`crypto`, `http`, `readline`, `fs`).

## Requirements

```
Node.js >= 18
```

## Agents

### Chat agent

Interactive CLI that talks to the C++ backend and optionally to a local LLM
(LM Studio / Ollama compatible endpoint).

```powershell
# Windows / Termux
node chat-agent.js

# With LM Studio
set RUNE_LLM_URL=http://localhost:1234/v1/chat/completions
node chat-agent.js
```

Commands: `/health` `/runes` `/encode <text>` `/sign <msg>` `/pubkey`
`/history` `/clear` `/exit`

### Node monitor agent

Polls `/health` every 10 s and alerts after 3 consecutive failures.

```powershell
node node-agent.js

# Custom interval (ms) and alert threshold
set NODE_POLL_INTERVAL=5000
set NODE_ALERT_AFTER=5
node node-agent.js
```

### Brain monitor agent

Connects to the `rune_brain` SSE event stream and prints events in real time.
Automatically reconnects on disconnect.

```powershell
node brain-monitor.js

# Custom brain host/port
set RUNE_BRAIN_HOST=127.0.0.1
set RUNE_BRAIN_PORT=7071
node brain-monitor.js
```

### Wallet agent

Ed25519 keypair management backed by `wallet.json`.

```powershell
node wallet-agent.js

# Custom wallet path
set WALLET_PATH=C:\rune\my-wallet.json
node wallet-agent.js
```

Commands: `pubkey` `sign <message>` `verify <message> <sig>` `regen` `exit`

## Environment variables

| Variable | Default | Used by |
|----------|---------|---------|
| `RUNE_BACKEND_HOST` | `127.0.0.1` | chat, node |
| `RUNE_BACKEND_PORT` | `7070` | chat, node |
| `RUNE_LLM_URL` | *(none)* | chat |
| `RUNE_LLM_MODEL` | `local-model` | chat |
| `NODE_POLL_INTERVAL` | `10000` | node |
| `NODE_ALERT_AFTER` | `3` | node |
| `RUNE_BRAIN_HOST` | `127.0.0.1` | brain-monitor |
| `RUNE_BRAIN_PORT` | `7071` | brain-monitor |
| `RUNE_SSE_PATH` | `/events` | brain-monitor |
| `WALLET_PATH` | `./wallet.json` | wallet |
