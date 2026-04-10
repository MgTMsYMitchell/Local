#!/usr/bin/env node
// rune-agents/wallet-agent.js
// Ed25519 key management using Node's built-in crypto (Node >= 15).
// Persists keypair to wallet.json; provides sign / verify via CLI.
'use strict';

const crypto   = require('crypto');
const fs       = require('fs');
const path     = require('path');
const readline = require('readline');

const WALLET_PATH = process.env.WALLET_PATH
  || path.join(process.cwd(), 'wallet.json');

// ── wallet I/O ──────────────────────────────────────────────────────────────

function generateWallet() {
  const { privateKey, publicKey } = crypto.generateKeyPairSync('ed25519');
  return {
    version:     1,
    public_key:  publicKey .export({ type: 'spki',  format: 'der' }).toString('hex'),
    private_key: privateKey.export({ type: 'pkcs8', format: 'der' }).toString('hex'),
    created_at:  new Date().toISOString(),
  };
}

function loadOrCreate(walletPath) {
  if (fs.existsSync(walletPath)) {
    const w = JSON.parse(fs.readFileSync(walletPath, 'utf8'));
    console.log(`[wallet] loaded  ${walletPath}`);
    console.log(`[wallet] pubkey  ${w.public_key.slice(0, 32)}…`);
    return w;
  }
  const w = generateWallet();
  fs.writeFileSync(walletPath, JSON.stringify(w, null, 2) + '\n');
  console.log(`[wallet] created ${walletPath}`);
  console.log(`[wallet] pubkey  ${w.public_key.slice(0, 32)}…`);
  return w;
}

// ── crypto helpers ──────────────────────────────────────────────────────────

function sign(w, message) {
  const privKey = crypto.createPrivateKey({
    key:    Buffer.from(w.private_key, 'hex'),
    format: 'der',
    type:   'pkcs8',
  });
  return crypto.sign(null, Buffer.from(message, 'utf8'), privKey).toString('hex');
}

function verify(w, message, sigHex) {
  const pubKey = crypto.createPublicKey({
    key:    Buffer.from(w.public_key, 'hex'),
    format: 'der',
    type:   'spki',
  });
  return crypto.verify(
    null,
    Buffer.from(message, 'utf8'),
    pubKey,
    Buffer.from(sigHex, 'hex')
  );
}

// ── CLI ─────────────────────────────────────────────────────────────────────

const w = loadOrCreate(WALLET_PATH);

const HELP = `Commands:
  pubkey                    — print full public key (hex)
  sign <message>            — sign a message; prints hex signature
  verify <message> <sig>    — verify a signature (true/false)
  regen                     — generate a new keypair (overwrites wallet)
  exit                      — quit`;

const rl = readline.createInterface({
  input: process.stdin, output: process.stdout, prompt: 'wallet> ',
});

console.log('\nᚠ Rune Wallet Agent  —  type "help" for commands\n');
rl.prompt();

rl.on('line', (line) => {
  const parts = line.trim().split(/\s+/);
  const cmd   = parts[0];

  if (!cmd) { rl.prompt(); return; }

  if (cmd === 'help' || cmd === '?') {
    console.log(HELP);
  } else if (cmd === 'pubkey') {
    console.log(w.public_key);
  } else if (cmd === 'sign') {
    const msg = parts.slice(1).join(' ');
    if (!msg) { console.log('usage: sign <message>'); }
    else      { console.log('sig:', sign(w, msg)); }
  } else if (cmd === 'verify') {
    // Last token = signature hex; everything in between = message.
    if (parts.length < 3) { console.log('usage: verify <message> <sig>'); }
    else {
      const sig = parts[parts.length - 1];
      const msg = parts.slice(1, -1).join(' ');
      try { console.log(verify(w, msg, sig) ? 'VALID ✓' : 'INVALID ✗'); }
      catch (e) { console.error('verify error:', e.message); }
    }
  } else if (cmd === 'regen') {
    const fresh = generateWallet();
    Object.assign(w, fresh);
    fs.writeFileSync(WALLET_PATH, JSON.stringify(w, null, 2) + '\n');
    console.log('new pubkey:', w.public_key.slice(0, 32) + '…');
  } else if (cmd === 'exit' || cmd === 'quit') {
    process.exit(0);
  } else {
    console.log(`unknown command "${cmd}" — type help`);
  }

  rl.prompt();
});

rl.on('close', () => { console.log('\n[wallet-agent] bye'); process.exit(0); });
