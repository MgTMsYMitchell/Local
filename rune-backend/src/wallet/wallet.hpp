#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

// ── Wallet ────────────────────────────────────────────────────────────────────
// Ed25519 keypair management backed by Monocypher (SHA-512 variant).
// Keys are persisted as JSON: { version, public_key, secret_key } (hex-encoded).

namespace wallet {

struct Keypair {
    std::array<uint8_t, 32> public_key{};
    // Monocypher Ed25519 secret key is 64 bytes: seed (32) || public_key (32).
    std::array<uint8_t, 64> secret_key{};
};

// Generate a new random keypair.
Keypair generate();

// Sign `message` with `kp`. Returns a 64-byte signature.
std::array<uint8_t, 64> sign(const Keypair& kp, std::span<const uint8_t> message);

// Verify a signature. Returns true iff valid.
bool verify(const std::array<uint8_t, 32>& public_key,
            std::span<const uint8_t>        message,
            const std::array<uint8_t, 64>&  signature);

// Hex helpers.
std::string              to_hex  (std::span<const uint8_t> bytes);
std::vector<uint8_t>     from_hex(std::string_view hex);

// Persist / load wallet JSON.
void    save(const Keypair& kp, const std::string& path);
Keypair load(const std::string& path);

} // namespace wallet
