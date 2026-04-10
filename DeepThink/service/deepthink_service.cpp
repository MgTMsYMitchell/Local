// deepthink_service.cpp
//
// Ingest path:
//   [Scraper/Feedback JSON]
//       → POST /ingest
//       → LocalAIClient::embed()   (LocalAI /v1/embeddings)
//       → project_to_14d()
//       → IngestItem batch
//       → QR2RuneEngine::ingest_batch()
//
// Query path:
//   [User query JSON]
//       → POST /query
//       → LocalAIClient::embed()
//       → QR2RuneEngine::query()   (RuneIndex L2 search + QR2Store fetch)
//       → build context string
//       → LocalAIClient::chat()    (LocalAI /v1/chat/completions)
//       → JSON reply
//
// Environment variables:
//   LOCALAI_HOST   (default: 127.0.0.1)
//   LOCALAI_PORT   (default: 8080)
//   SERVICE_PORT   (default: 9090)

#define CPPHTTPLIB_NO_SSL   // HTTP only; remove if HTTPS to LocalAI is needed
#include <httplib.h>
#include <nlohmann/json.hpp>

#include "localai_client.hpp"
#include "../engine/qr2_rune_engine.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace deepthink {

// ---------------------------------------------------------------------------
// LocalAIClient — implementation
// ---------------------------------------------------------------------------

LocalAIClient::LocalAIClient(Config cfg) : cfg_(std::move(cfg)) {}

SigilVec LocalAIClient::project_to_14d(const std::vector<float>& v)
{
    SigilVec s{};
    s.v.fill(0.f);
    if (v.empty()) return s;

    const std::size_t n = v.size();
    std::array<int, 14> counts{};
    counts.fill(0);

    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t b = (i * 14) / n;
        s.v[b] += v[i];
        counts[b]++;
    }
    for (int j = 0; j < 14; ++j) {
        if (counts[j] > 0) s.v[j] /= static_cast<float>(counts[j]);
    }

    // L2 normalise
    float norm = 0.f;
    for (float x : s.v) norm += x * x;
    norm = std::sqrt(norm);
    if (norm > 1e-8f) {
        for (float& x : s.v) x /= norm;
    }
    return s;
}

SigilVec LocalAIClient::embed(const std::string& text) const
{
    httplib::Client cli(cfg_.host, cfg_.port);
    cli.set_connection_timeout(cfg_.timeout_s);
    cli.set_read_timeout(cfg_.timeout_s);

    const json body = {{"model", cfg_.embed_model}, {"input", text}};

    auto res = cli.Post("/v1/embeddings", body.dump(), "application/json");
    if (!res || res->status != 200) {
        throw std::runtime_error(
            std::string("LocalAI embed failed: ") +
            (res ? std::to_string(res->status) + " " + res->body : "no response"));
    }

    const auto j = json::parse(res->body);
    const auto& arr = j.at("data").at(0).at("embedding");
    std::vector<float> full_emb;
    full_emb.reserve(arr.size());
    for (float x : arr) full_emb.push_back(x);

    return project_to_14d(full_emb);
}

std::string LocalAIClient::chat(const std::string& context,
                                const std::string& query) const
{
    httplib::Client cli(cfg_.host, cfg_.port);
    cli.set_connection_timeout(cfg_.timeout_s);
    cli.set_read_timeout(cfg_.timeout_s);

    const json body = {
        {"model", cfg_.chat_model},
        {"messages", json::array({
            {{"role", "system"}, {"content", context}},
            {{"role", "user"},   {"content", query}}
        })}
    };

    auto res = cli.Post("/v1/chat/completions", body.dump(), "application/json");
    if (!res || res->status != 200) {
        throw std::runtime_error(
            std::string("LocalAI chat failed: ") +
            (res ? std::to_string(res->status) + " " + res->body : "no response"));
    }

    const auto j = json::parse(res->body);
    return j.at("choices").at(0).at("message").at("content").get<std::string>();
}

} // namespace deepthink

// ---------------------------------------------------------------------------
// HTTP service entry point
// ---------------------------------------------------------------------------

int main()
{
    using namespace deepthink;

    // --- Configuration via environment variables ---
    auto env_or = [](const char* name, const char* def) -> std::string {
        const char* v = std::getenv(name);
        return v ? v : def;
    };

    LocalAIClient::Config ai_cfg;
    ai_cfg.host = env_or("LOCALAI_HOST", "127.0.0.1");
    ai_cfg.port = std::stoi(env_or("LOCALAI_PORT", "8080"));

    const int service_port = std::stoi(env_or("SERVICE_PORT", "9090"));

    LocalAIClient  ai(ai_cfg);
    QR2RuneEngine  engine;
    httplib::Server svr;

    // ------------------------------------------------------------------
    // POST /ingest
    //
    // Request body (JSON):
    // {
    //   "texts":      ["text1", "text2", ...],
    //   "tag_ids":    [1, 2, ...],
    //   "timestamps": [1700000000, 1700000001, ...]
    // }
    //
    // Response:
    // { "ingested": N, "total": M }
    // ------------------------------------------------------------------
    svr.Post("/ingest", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto body = json::parse(req.body);
            const auto& texts        = body.at("texts");
            const auto& tag_ids_j    = body.at("tag_ids");
            const auto& timestamps_j = body.at("timestamps");

            if (texts.size() != tag_ids_j.size() ||
                texts.size() != timestamps_j.size()) {
                throw std::invalid_argument(
                    "texts, tag_ids, and timestamps must have the same length");
            }

            std::vector<IngestItem> batch;
            batch.reserve(texts.size());

            for (std::size_t i = 0; i < texts.size(); ++i) {
                IngestItem item;
                item.timestamp = timestamps_j[i].get<uint32_t>();
                item.tag_id    = tag_ids_j[i].get<uint32_t>();
                item.sigil     = ai.embed(texts[i].get<std::string>());
                batch.push_back(std::move(item));
            }

            engine.ingest_batch(batch);

            res.set_content(
                json{{"ingested", batch.size()},
                     {"total",    engine.qr2().size()}}.dump(),
                "application/json");

        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(json{{"error", e.what()}}.dump(),
                            "application/json");
        }
    });

    // ------------------------------------------------------------------
    // POST /query
    //
    // Request body (JSON):
    // { "text": "...", "top_k": 5 }
    //
    // Response:
    // { "reply": "...", "records_found": N, "top_k_requested": K }
    // ------------------------------------------------------------------
    svr.Post("/query", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto body       = json::parse(req.body);
            const std::string text = body.at("text").get<std::string>();
            const std::size_t top_k = body.value("top_k", std::size_t{5});

            // 1. Embed query → SigilVec
            const SigilVec query_vec = ai.embed(text);

            // 2. Engine search
            std::vector<Record> records;
            std::vector<float>  scores;
            engine.query(query_vec, top_k, records, scores);

            // 3. Build context string from retrieved records
            //    (In production, augment with full text looked up from memory store)
            std::string context = "Retrieved memory records (id | tag | l2_score):\n";
            for (std::size_t i = 0; i < records.size(); ++i) {
                context += "  id=" + std::to_string(records[i].id)
                         + " tag=" + std::to_string(records[i].tag_id)
                         + " score=" + std::to_string(scores[i]) + "\n";
            }

            // 4. Chat completion with context
            const std::string reply = ai.chat(context, text);

            res.set_content(
                json{{"reply",           reply},
                     {"records_found",   records.size()},
                     {"top_k_requested", top_k}}.dump(),
                "application/json");

        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(json{{"error", e.what()}}.dump(),
                            "application/json");
        }
    });

    // ------------------------------------------------------------------
    // GET /status — health check
    // ------------------------------------------------------------------
    svr.Get("/status", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(
            json{{"status",    "ok"},
                 {"qr2_size",  engine.qr2().size()},
                 {"rune_size", engine.rune().size()}}.dump(),
            "application/json");
    });

    std::cout << "[DeepThink] service listening on 0.0.0.0:" << service_port
              << "  (LocalAI at " << ai_cfg.host << ":" << ai_cfg.port << ")\n";

    svr.listen("0.0.0.0", service_port);
    return 0;
}
