// Original wallet implementation

#include <monocypher.h>

class Wallet {
public:
    // ... other members ...
    void generate_keypair(const std::vector<uint8_t>& seed) {
        crypto_ed25519_seed_keypair(kp.public_key.data(), kp.secret_key.data(), seed.data());
    }

    void sign_message(const std::vector<uint8_t>& message, std::vector<uint8_t>& sig) {
        crypto_ed25519_sign(sig.data(), kp.secret_key.data(), kp.public_key.data(), message.data(), message.size());
    }

    bool verify_signature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& public_key, const std::vector<uint8_t>& message) {
        return crypto_ed25519_check(signature.data(), public_key.data(), message.data(), message.size()) == 0;
    }
};
