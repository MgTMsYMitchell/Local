#!/usr/bin/env node
// rune-agents/node-agent.js
// Polls the rune backend health endpoint and logs status.
// Alerts after 3 consecutive failures.
'use strict';

const http = require('http');

const BACKEND_HOST  = process.env.RUNE_BACKEND_HOST  || '127.0.0.1';
const BACKEND_PORT  = parseInt(process.env.RUNE_BACKEND_PORT  || '7070', 10);
const POLL_INTERVAL = parseInt(process.env.NODE_POLL_INTERVAL || '10000', 10);
const ALERT_AFTER   = parseInt(process.env.NODE_ALERT_AFTER   || '3',     10);

let failCount = 0;
let running   = true;

function ts() { return new Date().toISOString(); }

function backendGet(path) {
  return new Promise((resolve) => {
    const req = http.get(
      { host: BACKEND_HOST, port: BACKEND_PORT, path, timeout: 3000 },
      (res) => {
        let data = '';
        res.on('data', (c) => { data += c; });
        res.on('end', () => {
          try { resolve({ ok: true, status: res.statusCode, data: JSON.parse(data) }); }
          catch { resolve({ ok: true, status: res.statusCode, data: { raw: data } }); }
        });
      }
    );
    req.on('error',   (e) => resolve({ ok: false, error: e.message }));
    req.on('timeout', ()  => { req.destroy(); resolve({ ok: false, error: 'timeout' }); });
  });
}

async function poll() {
  const result = await backendGet('/health');
  if (result.ok && result.status === 200) {
    failCount = 0;
    const { status, runes, stories, mythic_moments } = result.data;
    console.log(`[${ts()}] OK  status=${status}  runes=${runes}  stories=${stories}  mythic=${mythic_moments}`);
  } else {
    failCount++;
    const reason = result.error ?? `HTTP ${result.status}`;
    console.error(`[${ts()}] FAIL (${failCount}/${ALERT_AFTER})  ${reason}`);
    if (failCount >= ALERT_AFTER) {
      console.error(`[${ts()}] ALERT: backend unreachable for ${failCount} polls.`);
      console.error(`         Check that rune.exe is running on ${BACKEND_HOST}:${BACKEND_PORT}`);
    }
  }
}

async function loop() {
  console.log(`[node-agent] polling http://${BACKEND_HOST}:${BACKEND_PORT}/health every ${POLL_INTERVAL}ms`);
  while (running) {
    await poll();
    await new Promise((r) => setTimeout(r, POLL_INTERVAL));
  }
  console.log('[node-agent] stopped');
}

process.on('SIGINT',  () => { running = false; console.log('\n[node-agent] stopping…'); });
process.on('SIGTERM', () => { running = false; });

loop();
