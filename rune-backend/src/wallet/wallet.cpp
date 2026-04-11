// Updates made with minimal edits

#include <monocypher.h>

void Keypair::generate(const std::array<uint8_t, 32>& seed) {
    // Updated crypto function usage
    crypto_ed25519_key_pair_seed(kp.secret_key.data(), kp.public_key.data(), seed.data());
}

void sign(const Keypair& kp, std::span<const uint8_t> message) {
    // Updated sign function usage
    crypto_ed25519_sign(sig.data(), kp.secret_key.data(), kp.public_key.data(), message.data(), message.size());
}