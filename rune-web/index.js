'use strict';

const express = require('express');
const http    = require('http');
const path    = require('path');

const app = express();

const BACKEND_HOST = process.env.RUNE_BACKEND_HOST || '127.0.0.1';
const BACKEND_PORT = parseInt(process.env.RUNE_BACKEND_PORT || '7070', 10);
const WEB_PORT     = parseInt(process.env.PORT             || '3000', 10);

// ── static UI ─────────────────────────────────────────────────────────────────
app.use(express.static(path.join(__dirname, 'public')));

// ── proxy helpers ─────────────────────────────────────────────────────────────

function backendGet(endpoint) {
  return new Promise((resolve, reject) => {
    const req = http.get(
      { host: BACKEND_HOST, port: BACKEND_PORT, path: endpoint, timeout: 3000 },
      (res) => {
        let data = '';
        res.on('data', (chunk) => { data += chunk; });
        res.on('end', () => {
          try { resolve(JSON.parse(data)); }
          catch { resolve({ raw: data }); }
        });
      }
    );
    req.on('error', reject);
    req.on('timeout', () => { req.destroy(); reject(new Error('timeout')); });
  });
}

// ── API routes ────────────────────────────────────────────────────────────────

// Forward /health from C++ backend
app.get('/health', async (_req, res) => {
  try {
    const data = await backendGet('/health');
    res.json(data);
  } catch (err) {
    res.status(503).json({ status: 'backend_unavailable', error: err.message });
  }
});

// Forward /runes from C++ backend
app.get('/runes', async (_req, res) => {
  try {
    const data = await backendGet('/runes');
    res.json(data);
  } catch (err) {
    res.status(503).json({ status: 'backend_unavailable', error: err.message });
  }
});

// ── start ─────────────────────────────────────────────────────────────────────
app.listen(WEB_PORT, () => {
  console.log(`[rune-web] listening on http://localhost:${WEB_PORT}`);
  console.log(`[rune-web] backend  at  http://${BACKEND_HOST}:${BACKEND_PORT}`);
});
