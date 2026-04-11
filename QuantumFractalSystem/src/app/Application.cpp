// QuantumFractalSystem — Application implementation

#include "app/Application.h"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace qfs {

Application::Application() = default;
Application::~Application() = default;

// ── init ─────────────────────────────────────────────────────────────────────

void Application::init(const std::string& config_path) {
    std::printf("[qfs] initialising...\n");

    // Load configuration
    load_config(config_path);

    // Open database
    db_ = std::make_unique<Database>("qfs.db");
    std::printf("[qfs] database opened (WAL mode)\n");

    // Apply schema
    apply_schema();

    // Seed initial mesh nodes
    seed_nodes(4);

    running_ = true;
    std::printf("[qfs] initialisation complete — %zu mesh nodes\n", mesh_.size());
}

// ── run ──────────────────────────────────────────────────────────────────────

void Application::run() {
    std::printf("[qfs] running main loop (press ENTER to stop)\n");

    int tick = 0;
    while (running_) {
        // 1. Flush inter-node messages
        mesh_.flush_messages();

        // 2. Run self-healing sweep
        auto ids = mesh_.node_ids();
        std::vector<MeshNode*> nodes;
        for (auto& id : ids) {
            MeshNode* n = mesh_.find_node(id);
            if (n) nodes.push_back(n);
        }
        auto repaired = healer_.sweep(nodes, compressor_);
        if (!repaired.empty()) {
            std::printf("[qfs] tick %d: healed %zu node(s)\n",
                        tick, repaired.size());
        }

        // 3. Sleep between ticks
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++tick;

        // Check for user interrupt (non-blocking stdin check)
        // For the skeleton, we just run a few ticks then stop
        if (tick >= 5) {
            std::printf("[qfs] demo complete after %d ticks\n", tick);
            running_ = false;
        }
    }
}

// ── shutdown ─────────────────────────────────────────────────────────────────

void Application::shutdown() {
    running_ = false;
    std::printf("[qfs] shutdown complete\n");
}

// ── private helpers ──────────────────────────────────────────────────────────

void Application::load_config(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::printf("[qfs] config not found at '%s', using defaults\n",
                    path.c_str());
        return;
    }

    try {
        auto cfg = nlohmann::json::parse(f);
        if (cfg.contains("compression") && cfg["compression"].contains("basis_size"))
            compressor_ = QuantumInspiredCompressor(
                cfg["compression"]["basis_size"].get<int>());
        if (cfg.contains("healing")) {
            float thresh = cfg["healing"].value("corruption_threshold", 0.3f);
            float rate   = cfg["healing"].value("repair_rate", 0.05f);
            healer_ = SelfHealingEngine(thresh, rate);
        }
        std::printf("[qfs] config loaded from '%s'\n", path.c_str());
    } catch (const std::exception& e) {
        std::printf("[qfs] config parse error: %s\n", e.what());
    }
}

void Application::apply_schema() {
    const char* schema = R"sql(
        CREATE TABLE IF NOT EXISTS nodes (
            id             TEXT PRIMARY KEY,
            alignment      BLOB,
            compressed_state BLOB,
            status         TEXT    NOT NULL DEFAULT 'active',
            trust          REAL    NOT NULL DEFAULT 1.0,
            last_seen      TEXT    DEFAULT (datetime('now')),
            created_at     TEXT    DEFAULT (datetime('now'))
        );
        CREATE TABLE IF NOT EXISTS tasks (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            node_id     TEXT    REFERENCES nodes(id),
            payload     BLOB,
            status      TEXT    NOT NULL DEFAULT 'pending',
            result      BLOB,
            created_at  TEXT    DEFAULT (datetime('now')),
            completed_at TEXT
        );
        CREATE TABLE IF NOT EXISTS alignment_snapshots (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            node_id     TEXT    REFERENCES nodes(id),
            field_data  BLOB,
            drift       BLOB,
            sampled_at  TEXT    DEFAULT (datetime('now'))
        );
        CREATE TABLE IF NOT EXISTS routes (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            from_node   TEXT    NOT NULL,
            to_node     TEXT    NOT NULL,
            via_node    TEXT,
            hops        INTEGER NOT NULL DEFAULT 1,
            latency_ms  REAL,
            updated_at  TEXT    DEFAULT (datetime('now'))
        );
        CREATE TABLE IF NOT EXISTS events (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            source      TEXT    NOT NULL,
            event_type  TEXT    NOT NULL,
            payload     TEXT    NOT NULL DEFAULT '{}',
            created_at  TEXT    DEFAULT (datetime('now'))
        );
    )sql";
    db_->apply_schema(schema);
    std::printf("[qfs] schema applied\n");
}

void Application::seed_nodes(int count) {
    for (int i = 0; i < count; ++i) {
        std::string id = "node-" + std::to_string(i + 1);
        auto node = std::make_unique<MeshNode>(id);

        // Give each node a unique alignment (unit vector along dimension i)
        Vec14 align(0.0f);
        align[i % Vec14::kDim] = 1.0f;
        node->set_alignment(align);
        node->compress_state(compressor_);

        // Persist to DB using parameterized query
        {
            sqlite3_stmt* stmt = nullptr;
            sqlite3_prepare_v2(db_->raw(),
                "INSERT OR IGNORE INTO nodes (id, status) VALUES (?, 'active');",
                -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        mesh_.add_node(std::move(node));
    }
}

} // namespace qfs
