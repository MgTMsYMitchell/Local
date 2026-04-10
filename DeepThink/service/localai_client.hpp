#pragma once
#include "../engine/qr2_rune_engine.hpp"
#include <string>
#include <vector>
#include <stdexcept>

namespace deepthink {

// ---------------------------------------------------------------------------
// LocalAIClient
//
// Thin wrapper around the LocalAI REST API (OpenAI-compatible).
// embed()  → calls /v1/embeddings, projects the N-dim result to a 14D SigilVec.
// chat()   → calls /v1/chat/completions with a system context + user message.
//
// Implementation lives in deepthink_service.cpp so that <httplib.h> is
// included only in that translation unit.
// ---------------------------------------------------------------------------

struct LocalAIConfig {
    std::string host        = "127.0.0.1";
    int         port        = 8080;           // LocalAI default
    std::string embed_model = "text-embedding-ada-002";
    std::string chat_model  = "gpt-3.5-turbo";
    int         timeout_s   = 30;
};

class LocalAIClient {
public:
    using Config = LocalAIConfig;

    explicit LocalAIClient(Config cfg = Config{});

    // Embed a single text and return a 14D SigilVec.
    SigilVec    embed(const std::string& text) const;

    // Run a chat-completion using context as the system prompt.
    std::string chat(const std::string& context,
                     const std::string& query) const;

private:
    Config cfg_;

    // Project an arbitrary-dimension embedding down to 14D via
    // uniform-bucket averaging followed by L2 normalisation.
    static SigilVec project_to_14d(const std::vector<float>& full_emb);
};

} // namespace deepthink

