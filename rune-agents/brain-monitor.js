#!/usr/bin/env node
// rune-agents/brain-monitor.js
// Connects to the rune_brain SSE stream and prints events to stdout.
// Uses only Node.js built-ins (http).
'use strict';

const http = require('http');

const BRAIN_HOST = process.env.RUNE_BRAIN_HOST || '127.0.0.1';
const BRAIN_PORT = parseInt(process.env.RUNE_BRAIN_PORT || '7071', 10);
const SSE_PATH   = process.env.RUNE_SSE_PATH   || '/events';

let running = true;

function ts() { return new Date().toISOString(); }

function connect() {
  console.log(`[brain-monitor] connecting to http://${BRAIN_HOST}:${BRAIN_PORT}${SSE_PATH}`);

  const req = http.get(
    { host: BRAIN_HOST, port: BRAIN_PORT, path: SSE_PATH, timeout: 0 },
    (res) => {
      if (res.statusCode !== 200) {
        console.error(`[${ts()}] unexpected status ${res.statusCode}`);
        res.resume();
        scheduleReconnect();
        return;
      }

      console.log(`[${ts()}] connected — streaming events\n`);

      let buffer = '';

      res.setEncoding('utf8');
      res.on('data', (chunk) => {
        buffer += chunk;

        // SSE messages are separated by double newlines
        let idx;
        while ((idx = buffer.indexOf('\n\n')) !== -1) {
          const raw = buffer.slice(0, idx);
          buffer = buffer.slice(idx + 2);

          // Skip keepalive comments (lines starting with ':')
          if (raw.trimStart().startsWith(':')) continue;

          // Parse SSE fields
          let eventType = '';
          let data = '';
          for (const line of raw.split('\n')) {
            if (line.startsWith('event: ')) {
              eventType = line.slice(7);
            } else if (line.startsWith('data: ')) {
              data += line.slice(6);
            }
          }

          if (data) {
            const prefix = eventType ? `[${eventType}]` : '[event]';
            try {
              const parsed = JSON.parse(data);
              console.log(`${ts()}  ${prefix}  ${JSON.stringify(parsed)}`);
            } catch {
              console.log(`${ts()}  ${prefix}  ${data}`);
            }
          }
        }
      });

      res.on('end', () => {
        console.log(`[${ts()}] stream ended`);
        scheduleReconnect();
      });

      res.on('error', (err) => {
        console.error(`[${ts()}] stream error: ${err.message}`);
        scheduleReconnect();
      });
    }
  );

  req.on('error', (err) => {
    console.error(`[${ts()}] connection error: ${err.message}`);
    scheduleReconnect();
  });

  req.on('timeout', () => {
    // SSE connections are long-lived; this should not fire with timeout: 0
    req.destroy();
    scheduleReconnect();
  });
}

function scheduleReconnect() {
  if (!running) return;
  console.log(`[${ts()}] reconnecting in 3s…`);
  setTimeout(() => { if (running) connect(); }, 3000);
}

process.on('SIGINT', () => {
  running = false;
  console.log('\n[brain-monitor] stopping…');
  process.exit(0);
});
process.on('SIGTERM', () => {
  running = false;
  process.exit(0);
});

console.log('ᚠ Rune Brain Monitor');
console.log(`  target  http://${BRAIN_HOST}:${BRAIN_PORT}${SSE_PATH}`);
console.log('  press Ctrl+C to stop\n');

connect();
