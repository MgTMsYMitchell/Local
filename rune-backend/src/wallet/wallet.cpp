#include "wallet.hpp"

#include <charconv>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <system_error>

// Monocypher: core + Ed25519 (SHA-512) extension
#include <monocypher.h>
#include <monocypher-ed25519.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wallet {

// ── internal helpers ──────────────────────────────────────────────────────────

static std::array<uint8_t, 32> random_seed()
{
    std::array<uint8_t, 32> seed{};
    std::random_device rd;
    // Fill 8 bytes per random_device call (uint32_t, 4 bytes each → 8 calls).
    for (size_t i = 0; i < seed.size(); i += sizeof(uint32_t)) {
        uint32_t val = rd();
        for (size_t j = 0; j < sizeof(uint32_t) && (i + j) < seed.size(); ++j)
            seed[i + j] = static_cast<uint8_t>((val >> (8 * j)) & 0xFF);
    }
    return seed;
}

// ── public API ────────────────────────────────────────────────────────────────

Keypair generate()
{
    Keypair kp;
    auto seed = random_seed();
    crypto_ed25519_key_pair(kp.secret_key.data(), kp.public_key.data(), seed.data());
    crypto_wipe(seed.data(), seed.size());
    return kp;
}

std::array<uint8_t, 64> sign(const Keypair& kp, std::span<const uint8_t> message)
{
    std::array<uint8_t, 64> sig{};
    crypto_ed25519_sign(sig.data(), kp.secret_key.data(),
                        message.data(), message.size());
    return sig;
}

bool verify(const std::array<uint8_t, 32>& public_key,
            std::span<const uint8_t>        message,
            const std::array<uint8_t, 64>&  signature)
{
    return crypto_ed25519_check(signature.data(), public_key.data(),
                                message.data(), message.size()) == 0;
}

std::string to_hex(std::span<const uint8_t> bytes)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto b : bytes) oss << std::setw(2) << static_cast<unsigned>(b);
    return oss.str();
}

std::vector<uint8_t> from_hex(std::string_view hex)
{
    if (hex.size() % 2 != 0)
        throw std::invalid_argument("from_hex: odd length");
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned val = 0;
        auto [ptr, ec] = std::from_chars(hex.data() + i, hex.data() + i + 2, val, 16);
        if (ec != std::errc{})
            throw std::invalid_argument("from_hex: invalid character");
        out.push_back(static_cast<uint8_t>(val));
    }
    return out;
}

void save(const Keypair& kp, const std::string& path)
{
    json j = {
        {"version",    1},
        {"public_key", to_hex(kp.public_key)},
        {"secret_key", to_hex(kp.secret_key)}
    };
    std::ofstream f(path);
    if (!f) throw std::runtime_error("wallet save: cannot write " + path);
    f << j.dump(2) << '\n';
}

Keypair load(const std::string& path)
{
    std::ifstream f(path);
    if (!f) throw std::runtime_error("wallet load: cannot open " + path);
    json j;
    f >> j;

    auto pk = from_hex(j.at("public_key").get<std::string>());
    auto sk = from_hex(j.at("secret_key").get<std::string>());
    if (pk.size() != 32 || sk.size() != 64)
        throw std::runtime_error("wallet load: invalid key sizes");

    Keypair kp;
    std::copy(pk.begin(), pk.end(), kp.public_key.begin());
    std::copy(sk.begin(), sk.end(), kp.secret_key.begin());
    return kp;
}

} // namespace wallet
