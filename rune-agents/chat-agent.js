#!/usr/bin/env node
// rune-agents/chat-agent.js
// Interactive CLI chat agent — proxies backend status and echos user input.
// Plug in an LM Studio / Ollama endpoint via RUNE_LLM_URL for real AI replies.
'use strict';

const http     = require('http');
const readline = require('readline');

const BACKEND_HOST = process.env.RUNE_BACKEND_HOST || '127.0.0.1';
const BACKEND_PORT = parseInt(process.env.RUNE_BACKEND_PORT || '7070', 10);
const LLM_URL      = process.env.RUNE_LLM_URL || null; // e.g. http://localhost:1234/v1/chat/completions

// ── context window ─────────────────────────────────────────────────────────────
const context = [];

// ── backend helpers ────────────────────────────────────────────────────────────
function backendGet(path) {
  return new Promise((resolve, reject) => {
    const req = http.get(
      { host: BACKEND_HOST, port: BACKEND_PORT, path, timeout: 3000 },
      (res) => {
        let data = '';
        res.on('data', (c) => { data += c; });
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

function backendPost(path, body) {
  return new Promise((resolve, reject) => {
    const payload = JSON.stringify(body);
    const options = {
      host: BACKEND_HOST, port: BACKEND_PORT, path,
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(payload) },
      timeout: 5000,
    };
    const req = http.request(options, (res) => {
      let data = '';
      res.on('data', (c) => { data += c; });
      res.on('end', () => {
        try { resolve(JSON.parse(data)); }
        catch { resolve({ raw: data }); }
      });
    });
    req.on('error', reject);
    req.on('timeout', () => { req.destroy(); reject(new Error('timeout')); });
    req.write(payload);
    req.end();
  });
}

// ── LLM call (optional) ────────────────────────────────────────────────────────
async function llmReply(userMessage) {
  if (!LLM_URL) {
    return `[echo] ${userMessage}  (set RUNE_LLM_URL to enable LLM replies)`;
  }
  const url = new URL(LLM_URL);
  const payload = JSON.stringify({
    model: process.env.RUNE_LLM_MODEL || 'local-model',
    messages: [...context, { role: 'user', content: userMessage }],
    stream: false,
  });
  return new Promise((resolve) => {
    const lib = url.protocol === 'https:' ? require('https') : require('http');
    const req = lib.request(
      { host: url.hostname, port: url.port || (url.protocol === 'https:' ? 443 : 80),
        path: url.pathname, method: 'POST',
        headers: { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(payload) },
        timeout: 30000 },
      (res) => {
        let data = '';
        res.on('data', (c) => { data += c; });
        res.on('end', () => {
          try {
            const j = JSON.parse(data);
            resolve(j?.choices?.[0]?.message?.content ?? `[llm] unexpected response shape: ${data.slice(0, 100)}`);
          } catch { resolve(`[llm error] malformed JSON: ${data.slice(0, 100)}`); }
        });
      }
    );
    req.on('error', (e) => resolve(`[llm error] ${e.message}`));
    req.on('timeout', () => { req.destroy(); resolve('[llm error] timeout'); });
    req.write(payload);
    req.end();
  });
}

// ── command handler ────────────────────────────────────────────────────────────
const COMMANDS = `Commands:
  /help             — show this help
  /health           — backend health check
  /runes            — list all runes
  /encode <text>    — encode text to rune glyphs
  /sign <message>   — sign a message with backend wallet
  /pubkey           — show backend public key
  /history          — show conversation context
  /clear            — clear conversation context
  /exit             — quit`;

async function handleLine(input) {
  const trimmed = input.trim();
  if (!trimmed) return;

  if (trimmed === '/help') { console.log(COMMANDS); return; }

  if (trimmed === '/health') {
    try { console.log(JSON.stringify(await backendGet('/health'), null, 2)); }
    catch (e) { console.error('backend error:', e.message); }
    return;
  }

  if (trimmed === '/runes') {
    try {
      const runes = await backendGet('/runes');
      if (Array.isArray(runes))
        runes.forEach((r) => console.log(`  ${r.glyph}  ${r.name.padEnd(12)} ${r.meaning}`));
    } catch (e) { console.error('backend error:', e.message); }
    return;
  }

  if (trimmed.startsWith('/encode ')) {
    const text = trimmed.slice(8);
    try {
      const res = await backendGet(`/runes/encode?text=${encodeURIComponent(text)}`);
      console.log(`  ${text}  →  ${res.encoded}`);
    } catch (e) { console.error('backend error:', e.message); }
    return;
  }

  if (trimmed.startsWith('/sign ')) {
    const msg = trimmed.slice(6);
    try {
      const res = await backendPost('/wallet/sign', { message: msg });
      console.log('  sig:', res.signature);
    } catch (e) { console.error('backend error:', e.message); }
    return;
  }

  if (trimmed === '/pubkey') {
    try { console.log((await backendGet('/wallet/pubkey')).public_key); }
    catch (e) { console.error('backend error:', e.message); }
    return;
  }

  if (trimmed === '/history') {
    context.forEach((m, i) => console.log(`[${i}] ${m.role}: ${m.content}`));
    return;
  }

  if (trimmed === '/clear') { context.length = 0; console.log('context cleared'); return; }

  if (trimmed === '/exit' || trimmed === '/quit') process.exit(0);

  // Chat message → LLM or echo
  context.push({ role: 'user', content: trimmed });
  const reply = await llmReply(trimmed);
  context.push({ role: 'assistant', content: reply });
  console.log(`\n  ${reply}\n`);
}

// ── main ───────────────────────────────────────────────────────────────────────
const rl = readline.createInterface({
  input: process.stdin, output: process.stdout, prompt: 'rune> ',
});

console.log('ᚠ Rune Chat Agent');
console.log(`  backend  http://${BACKEND_HOST}:${BACKEND_PORT}`);
console.log(LLM_URL ? `  llm      ${LLM_URL}` : '  llm      (echo mode — set RUNE_LLM_URL to enable)');
console.log('  type /help for commands\n');

rl.prompt();
rl.on('line', async (line) => { await handleLine(line); rl.prompt(); });
rl.on('close', () => { console.log('\n[chat-agent] bye'); process.exit(0); });
