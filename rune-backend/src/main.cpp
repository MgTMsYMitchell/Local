#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "db/database.hpp"
#include "encoding/rune_codec.hpp"
#include "node/rune_node.hpp"
#include "wallet/wallet.hpp"

using json = nlohmann::json;

// ── logging ───────────────────────────────────────────────────────────────────

static std::mutex g_log_mtx;

template <typename... Args>
static void log(const char* fmt, Args&&... args)
{
    std::lock_guard<std::mutex> lk(g_log_mtx);
    std::printf(fmt, std::forward<Args>(args)...);
    std::fflush(stdout);
}

// ── HTTP routes ───────────────────────────────────────────────────────────────

static void setup_routes(httplib::Server& srv,
                         Database&        db,
                         wallet::Keypair& kp,
                         std::atomic<bool>& shutdown)
{
    auto cors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    };

    // GET /health
    srv.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
        json body = {
            {"status",         "ok"},
            {"runes",          db.count("runes")},
            {"macro_runes",    db.count("macro_runes")},
            {"stories",        db.count("stories")},
            {"mythic_moments", db.count("mythic_moments")},
            {"public_key",     wallet::to_hex(kp.public_key).substr(0, 16) + "..."}
        };
        cors(res);
        res.set_content(body.dump(2), "application/json");
    });

    // GET /runes
    srv.Get("/runes", [&](const httplib::Request&, httplib::Response& res) {
        cors(res);
        res.set_content(db.all_runes().dump(2), "application/json");
    });

    // GET /runes/encode?text=hello
    srv.Get("/runes/encode", [](const httplib::Request& req, httplib::Response& res) {
        std::string text = req.has_param("text") ? req.get_param_value("text") : "";
        json body = {
            {"input",   text},
            {"encoded", rune_codec::encode(text)}
        };
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(body.dump(2), "application/json");
    });

    // GET /wallet/pubkey
    srv.Get("/wallet/pubkey", [&](const httplib::Request&, httplib::Response& res) {
        json body = {{"public_key", wallet::to_hex(kp.public_key)}};
        cors(res);
        res.set_content(body.dump(2), "application/json");
    });

    // POST /wallet/sign   body: {"message":"..."}
    srv.Post("/wallet/sign", [&](const httplib::Request& req, httplib::Response& res) {
        cors(res);
        try {
            auto body = json::parse(req.body);
            std::string msg = body.at("message").get<std::string>();
            std::vector<uint8_t> bytes(msg.begin(), msg.end());
            auto sig = wallet::sign(kp, bytes);
            res.set_content(
                json({{"signature", wallet::to_hex(sig)}}).dump(2),
                "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // POST /wallet/verify  body: {"message":"...","signature":"<hex>"}
    srv.Post("/wallet/verify", [&](const httplib::Request& req, httplib::Response& res) {
        cors(res);
        try {
            auto body = json::parse(req.body);
            std::string msg    = body.at("message"  ).get<std::string>();
            std::string sighex = body.at("signature").get<std::string>();
            std::vector<uint8_t> msgbytes(msg.begin(), msg.end());
            auto sigbytes = wallet::from_hex(sighex);
            if (sigbytes.size() != 64) throw std::runtime_error("signature must be 64 bytes");
            std::array<uint8_t, 64> sig64{};
            std::copy(sigbytes.begin(), sigbytes.end(), sig64.begin());
            bool ok = wallet::verify(kp.public_key, msgbytes, sig64);
            res.set_content(json({{"valid", ok}}).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // POST /shutdown
    srv.Post("/shutdown", [&](const httplib::Request&, httplib::Response& res) {
        cors(res);
        res.set_content(json({{"status", "shutting_down"}}).dump(), "application/json");
        shutdown.store(true);
        srv.stop();
    });
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    const std::string db_path   = (argc > 1) ? argv[1] : "rune.db";
    const std::string kp_path   = (argc > 2) ? argv[2] : "wallet.json";
    const int         http_port = (argc > 3) ? std::stoi(argv[3]) : 7070;

    log("[rune] db=%s  wallet=%s  port=%d\n",
        db_path.c_str(), kp_path.c_str(), http_port);

    // ── Database ──────────────────────────────────────────────────────────────
    Database db(db_path);
    log("[db] migrations applied\n");

    // Seed initial runes on first run.
    if (db.count("runes") == 0) {
        db.exec(R"sql(
            INSERT INTO runes (name, glyph, meaning) VALUES
                ('Fehu',     'ᚠ', 'Cattle, wealth, abundance'),
                ('Uruz',     'ᚢ', 'Aurochs, strength, endurance'),
                ('Thurisaz', 'ᚦ', 'Giant, thorn, chaos'),
                ('Ansuz',    'ᚨ', 'God, mouth, wisdom'),
                ('Raidho',   'ᚱ', 'Ride, journey, rhythm'),
                ('Kenaz',    'ᚲ', 'Torch, knowledge, enlightenment'),
                ('Gebo',     'ᚷ', 'Gift, exchange, partnership'),
                ('Wunjo',    'ᚹ', 'Joy, fellowship, harmony');
        )sql");
        log("[db] seeded 8 Elder Futhark runes\n");
    }

    // ── Wallet ────────────────────────────────────────────────────────────────
    wallet::Keypair kp;
    try {
        kp = wallet::load(kp_path);
        log("[wallet] loaded  pubkey=%s...\n",
            wallet::to_hex(kp.public_key).substr(0, 16).c_str());
    } catch (...) {
        kp = wallet::generate();
        wallet::save(kp, kp_path);
        log("[wallet] created pubkey=%s...\n",
            wallet::to_hex(kp.public_key).substr(0, 16).c_str());
    }

    // ── Rune codec demo ───────────────────────────────────────────────────────
    log("[codec] encode('rune') = %s\n", rune_codec::encode("rune").c_str());

    // ── Node threads ──────────────────────────────────────────────────────────
    const int NODE_COUNT = 2;
    std::atomic<bool> shutdown{false};
    std::vector<std::thread> threads;
    threads.reserve(NODE_COUNT + 1);

    for (int i = 0; i < NODE_COUNT; ++i) {
        RuneNode node{i + 1, shutdown, db};
        threads.emplace_back([n = std::move(node)]() mutable { n.run(); });
    }

    // ── HTTP server ───────────────────────────────────────────────────────────
    httplib::Server srv;
    setup_routes(srv, db, kp, shutdown);

    threads.emplace_back([&srv, http_port]() {
        log("[http] listening on :%d\n", http_port);
        srv.listen("0.0.0.0", http_port);
        log("[http] server stopped\n");
    });

    log("[rune] %d node(s) running.\n"
        "  GET  http://localhost:%d/health\n"
        "  GET  http://localhost:%d/runes\n"
        "  GET  http://localhost:%d/runes/encode?text=hello\n"
        "  GET  http://localhost:%d/wallet/pubkey\n"
        "  POST http://localhost:%d/wallet/sign\n"
        "  POST http://localhost:%d/wallet/verify\n"
        "  POST http://localhost:%d/shutdown\n"
        "Press ENTER to stop.\n",
        NODE_COUNT,
        http_port, http_port, http_port, http_port,
        http_port, http_port, http_port);

    std::cin.get();

    shutdown.store(true);
    srv.stop();
    for (auto& t : threads) t.join();

    log("[rune] shutdown complete.\n");
    return 0;
}


