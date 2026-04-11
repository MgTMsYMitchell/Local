/*
  PROJECT: QRrune / Cognitive Node Brain
  FILE: brain_main.cpp

  INTENT FOR GITHUB COPILOT CHAT / AI:
  - This is the entry-point for the rune_brain executable.
  - It wires BrainDb + EventBus + all cognitive agents together,
    then starts them in dedicated threads.
  - HTTP server on :7071 exposes the brain REST + SSE API.
  - Graceful shutdown via POST /brain/shutdown or ENTER key.

  Usage:
    ./rune_brain [db_path] [http_port]
    ./rune_brain brain.db 7071
*/

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "brain/brain_system.hpp"

using json = nlohmann::json;

// ── logging ───────────────────────────────────────────────────────────────────

static std::mutex g_log_mtx;

template <typename... Args>
static void brain_log(const char* fmt, Args&&... args)
{
    std::lock_guard<std::mutex> lk(g_log_mtx);
    std::printf(fmt, std::forward<Args>(args)...);
    std::fflush(stdout);
}

// ── HTTP routes ───────────────────────────────────────────────────────────────

static void setup_brain_routes(httplib::Server& srv,
                                BrainDb&         db,
                                EventBus&        bus,
                                std::atomic<bool>& shutdown)
{
    auto cors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    };

    // GET /brain/health — AllCounts in one SQL round-trip
    srv.Get("/brain/health", [&](const httplib::Request&, httplib::Response& res) {
        auto c = db.all_counts();
        json body = {
            {"status",         "ok"},
            {"nodes",          c.nodes},
            {"agents",         c.agents},
            {"runes",          c.runes},
            {"strategies",     c.strategies},
            {"fusion_entries", c.fusion_entries},
            {"artifacts",      c.artifacts},
            {"audit_entries",  c.audit_entries},
            {"pending_events", c.pending_events},
            {"trust_entries",  c.trust_entries}
        };
        cors(res);
        res.set_content(body.dump(2), "application/json");
    });

    // GET /brain/runes[?limit=&offset=]
    srv.Get("/brain/runes", [&](const httplib::Request& req, httplib::Response& res) {
        int limit  = 200;
        int offset = 0;
        if (req.has_param("limit"))  limit  = std::stoi(req.get_param_value("limit"));
        if (req.has_param("offset")) offset = std::stoi(req.get_param_value("offset"));
        cors(res);
        res.set_content(db.all_runes(limit, offset).dump(2), "application/json");
    });

    // GET /brain/runes/:name
    srv.Get(R"(/brain/runes/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        auto name = req.matches[1].str();
        auto rune = db.rune_by_name(name);
        cors(res);
        if (rune.is_null()) {
            res.status = 404;
            res.set_content(json({{"error", "rune not found"}}).dump(), "application/json");
        } else {
            res.set_content(rune.dump(2), "application/json");
        }
    });

    // POST /brain/runes — create rune
    srv.Post("/brain/runes", [&](const httplib::Request& req, httplib::Response& res) {
        cors(res);
        try {
            auto body = json::parse(req.body);
            std::string name     = body.at("name").get<std::string>();
            std::string glyph    = body.value("glyph", "");
            std::string category = body.value("category", "general");
            int level            = body.value("level", 1);
            int ttl              = body.value("ttl_seconds", 0);
            int id = db.insert_rune(name, glyph, category, level, ttl);
            res.status = 201;
            res.set_content(json({{"id", id}, {"name", name}}).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // GET /brain/strategies
    srv.Get("/brain/strategies", [&](const httplib::Request& req, httplib::Response& res) {
        int limit  = 100;
        int offset = 0;
        if (req.has_param("limit"))  limit  = std::stoi(req.get_param_value("limit"));
        if (req.has_param("offset")) offset = std::stoi(req.get_param_value("offset"));
        cors(res);
        res.set_content(db.all_strategies(limit, offset).dump(2), "application/json");
    });

    // POST /brain/strategies — create strategy
    srv.Post("/brain/strategies", [&](const httplib::Request& req, httplib::Response& res) {
        cors(res);
        try {
            auto body = json::parse(req.body);
            std::string name = body.at("name").get<std::string>();
            std::string desc = body.value("description", "");
            json rune_ids    = body.value("rune_ids", json::array());
            int priority     = body.value("priority", 0);
            int id = db.insert_strategy(name, desc, rune_ids, priority);
            res.status = 201;
            res.set_content(json({{"id", id}, {"name", name}}).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // GET /brain/fusion
    srv.Get("/brain/fusion", [&](const httplib::Request& req, httplib::Response& res) {
        int limit = 50;
        if (req.has_param("limit")) limit = std::stoi(req.get_param_value("limit"));
        cors(res);
        res.set_content(db.fusion_log(limit).dump(2), "application/json");
    });

    // POST /brain/trust — record trust delta
    srv.Post("/brain/trust", [&](const httplib::Request& req, httplib::Response& res) {
        cors(res);
        try {
            auto body = json::parse(req.body);
            std::string subject      = body.at("subject").get<std::string>();
            std::string subject_type = body.value("subject_type", "rune");
            double delta             = body.at("delta").get<double>();
            std::string reason       = body.value("reason", "api");
            db.record_trust(subject, subject_type, delta, reason);
            res.set_content(json({{"status", "recorded"}}).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // GET /brain/trust — aggregate trust summary
    srv.Get("/brain/trust", [&](const httplib::Request& req, httplib::Response& res) {
        std::string type = "rune";
        if (req.has_param("type")) type = req.get_param_value("type");
        cors(res);
        res.set_content(db.aggregate_trust(type).dump(2), "application/json");
    });

    // GET /brain/size — size_assessment history
    srv.Get("/brain/size", [&](const httplib::Request&, httplib::Response& res) {
        cors(res);
        // Return the latest snapshot via all_counts
        auto c = db.all_counts();
        json body = {
            {"nodes",          c.nodes},
            {"agents",         c.agents},
            {"runes",          c.runes},
            {"strategies",     c.strategies},
            {"fusion_entries", c.fusion_entries},
            {"artifacts",      c.artifacts},
            {"audit_entries",  c.audit_entries},
            {"pending_events", c.pending_events},
            {"trust_entries",  c.trust_entries}
        };
        res.set_content(body.dump(2), "application/json");
    });

    // POST /brain/events — enqueue a node_event
    srv.Post("/brain/events", [&](const httplib::Request& req, httplib::Response& res) {
        cors(res);
        try {
            auto body = json::parse(req.body);
            std::string source     = body.value("source", "api");
            std::string event_type = body.at("event_type").get<std::string>();
            json payload           = body.value("payload", json::object());
            int priority           = body.value("priority", 0);
            int id = db.enqueue_event(source, event_type, payload, priority);
            res.status = 201;
            res.set_content(json({{"id", id}, {"status", "queued"}}).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // GET /brain/events/recent[?n=50]
    srv.Get("/brain/events/recent", [&](const httplib::Request& req, httplib::Response& res) {
        int n = 50;
        if (req.has_param("n")) n = std::stoi(req.get_param_value("n"));
        cors(res);
        res.set_content(bus.recent(n).dump(2), "application/json");
    });

    // GET /brain/events/stream — SSE live stream
    srv.Get("/brain/events/stream",
        [&](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Cache-Control", "no-cache");
            res.set_header("Connection", "keep-alive");
            res.set_content_provider(
                "text/event-stream",
                [&](size_t /*offset*/, httplib::DataSink& sink) {
                    // Subscribe to the EventBus and push events to the SSE stream.
                    std::mutex cv_mtx;
                    std::condition_variable cv;
                    std::deque<std::string> queue;

                    int sub_id = bus.subscribe([&](const BrainEvent& ev) {
                        json payload = {
                            {"source",    ev.source},
                            {"type",      ev.type},
                            {"payload",   ev.payload},
                            {"timestamp", ev.timestamp}
                        };
                        std::string data = "data: " + payload.dump() + "\n\n";
                        {
                            std::lock_guard<std::mutex> lk(cv_mtx);
                            queue.push_back(std::move(data));
                        }
                        cv.notify_one();
                    });

                    while (!shutdown.load(std::memory_order_relaxed)) {
                        std::unique_lock<std::mutex> lk(cv_mtx);
                        cv.wait_for(lk, std::chrono::seconds(15), [&] {
                            return !queue.empty() || shutdown.load(std::memory_order_relaxed);
                        });

                        if (queue.empty()) {
                            // Send keepalive comment
                            if (!sink.write(": keepalive\n\n", 14)) {
                                break;
                            }
                            continue;
                        }

                        while (!queue.empty()) {
                            auto& msg = queue.front();
                            if (!sink.write(msg.data(), msg.size())) {
                                bus.unsubscribe(sub_id);
                                return false;
                            }
                            queue.pop_front();
                        }
                    }

                    bus.unsubscribe(sub_id);
                    sink.done();
                    return false;
                },
                [](bool /*success*/) {}
            );
        });

    // POST /brain/shutdown
    srv.Post("/brain/shutdown", [&](const httplib::Request&, httplib::Response& res) {
        cors(res);
        res.set_content(json({{"status", "shutting_down"}}).dump(), "application/json");
        shutdown.store(true);
        srv.stop();
    });

    // ── Root-level convenience aliases ───────────────────────────────────────

    // GET /health — same as /brain/health
    srv.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
        auto c = db.all_counts();
        json body = {
            {"status",         "ok"},
            {"nodes",          c.nodes},
            {"agents",         c.agents},
            {"runes",          c.runes},
            {"strategies",     c.strategies},
            {"fusion_entries", c.fusion_entries},
            {"artifacts",      c.artifacts},
            {"audit_entries",  c.audit_entries},
            {"pending_events", c.pending_events},
            {"trust_entries",  c.trust_entries}
        };
        cors(res);
        res.set_content(body.dump(2), "application/json");
    });

    // GET /events — SSE live stream (alias for /brain/events/stream)
    srv.Get("/events",
        [&](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Cache-Control", "no-cache");
            res.set_header("Connection", "keep-alive");
            res.set_content_provider(
                "text/event-stream",
                [&](size_t /*offset*/, httplib::DataSink& sink) {
                    std::mutex cv_mtx;
                    std::condition_variable cv;
                    std::deque<std::string> queue;

                    int sub_id = bus.subscribe([&](const BrainEvent& ev) {
                        std::string msg = "event: " + ev.type + "\n"
                                        + "data: "
                                        + json({
                                              {"source",    ev.source},
                                              {"type",      ev.type},
                                              {"payload",   ev.payload},
                                              {"timestamp", ev.timestamp}
                                          }).dump()
                                        + "\n\n";
                        {
                            std::lock_guard<std::mutex> lk(cv_mtx);
                            queue.push_back(std::move(msg));
                        }
                        cv.notify_one();
                    });

                    while (!shutdown.load(std::memory_order_relaxed)) {
                        std::unique_lock<std::mutex> lk(cv_mtx);
                        cv.wait_for(lk, std::chrono::seconds(15), [&] {
                            return !queue.empty() || shutdown.load(std::memory_order_relaxed);
                        });

                        if (queue.empty()) {
                            if (!sink.write(": keepalive\n\n", 14)) {
                                break;
                            }
                            continue;
                        }

                        while (!queue.empty()) {
                            auto& msg = queue.front();
                            if (!sink.write(msg.data(), msg.size())) {
                                bus.unsubscribe(sub_id);
                                return false;
                            }
                            queue.pop_front();
                        }
                    }

                    bus.unsubscribe(sub_id);
                    sink.done();
                    return false;
                },
                [](bool /*success*/) {}
            );
        });

    // POST /event — enqueue an event (simplified payload: {"type":"...","message":"..."})
    srv.Post("/event", [&](const httplib::Request& req, httplib::Response& res) {
        cors(res);
        try {
            auto body = json::parse(req.body);
            std::string event_type = body.value("type", "note");
            json payload = body;
            payload.erase("type");
            int id = db.enqueue_event("api", event_type, payload);

            // Also broadcast immediately on the EventBus so SSE clients see it
            bus.post(BrainEvent{"api", event_type, payload, utc_now()});

            res.status = 201;
            res.set_content(json({{"id", id}, {"status", "queued"}}).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    const std::string db_path   = (argc > 1) ? argv[1] : "brain.db";
    const int         http_port = (argc > 2) ? std::stoi(argv[2]) : 7071;

    brain_log("[brain] db=%s  port=%d\n", db_path.c_str(), http_port);

    // ── Database + EventBus ──────────────────────────────────────────────────
    BrainDb  db(db_path);
    EventBus bus;
    brain_log("[brain] schema ensured, data seeded\n");

    // ── Shutdown flag ────────────────────────────────────────────────────────
    std::atomic<bool> shutdown{false};

    // ── Register and create agents ───────────────────────────────────────────
    HeartAgent        heart  ("Heart",        "heart",   db, bus, shutdown);
    WorkerAgent       worker ("Worker",       "worker",  db, bus, shutdown);
    GCAgent           gc     ("GC",           "gc",      db, bus, shutdown);
    RuneTrustManager  trust  ("TrustManager", "trust",   db, bus, shutdown);
    RuneFusionEngine  fusion ("FusionEngine", "fusion",  db, bus, shutdown);
    ChatAgent         chat   ("ChatAgent",    "chat",    db, bus, shutdown);
    WalletAgent       wallet ("WalletAgent",  "wallet",  db, bus, shutdown);
    NodeAgent         node   ("NodeAgent",    "node",    db, bus, shutdown);
    TorrentAgent      torrent("TorrentAgent", "torrent", db, bus, shutdown);
    EdgeAgent         edge   ("EdgeAgent",    "edge",    db, bus, shutdown);

    // Register all agents in the DB
    db.register_agent("Heart",        "heart");
    db.register_agent("Worker",       "worker");
    db.register_agent("GC",           "gc");
    db.register_agent("TrustManager", "trust");
    db.register_agent("FusionEngine", "fusion");
    db.register_agent("ChatAgent",    "chat");
    db.register_agent("WalletAgent",  "wallet");
    db.register_agent("NodeAgent",    "node");
    db.register_agent("TorrentAgent", "torrent");
    db.register_agent("EdgeAgent",    "edge");

    // Register the local node
    db.upsert_node("local", "local_brain", "active");
    brain_log("[brain] 10 agents registered, local node active\n");

    // ── Launch agent threads ─────────────────────────────────────────────────
    std::vector<std::thread> threads;
    threads.reserve(12);

    threads.emplace_back([&heart]()   { heart.run();   });
    threads.emplace_back([&worker]()  { worker.run();  });
    threads.emplace_back([&gc]()      { gc.run();      });
    threads.emplace_back([&trust]()   { trust.run();   });
    threads.emplace_back([&fusion]()  { fusion.run();  });
    threads.emplace_back([&chat]()    { chat.run();    });
    threads.emplace_back([&wallet]()  { wallet.run();  });
    threads.emplace_back([&node]()    { node.run();    });
    threads.emplace_back([&torrent]() { torrent.run(); });
    threads.emplace_back([&edge]()    { edge.run();    });

    // ── HTTP server ──────────────────────────────────────────────────────────
    httplib::Server srv;
    setup_brain_routes(srv, db, bus, shutdown);

    threads.emplace_back([&srv, http_port]() {
        brain_log("[http] brain API listening on :%d\n", http_port);
        srv.listen("0.0.0.0", http_port);
        brain_log("[http] brain API stopped\n");
    });

    brain_log(
        "[brain] QRrune Cognitive Node running.\n"
        "  GET  http://localhost:%d/health              (root alias)\n"
        "  GET  http://localhost:%d/events              (SSE root alias)\n"
        "  POST http://localhost:%d/event               (enqueue root alias)\n"
        "  GET  http://localhost:%d/brain/health\n"
        "  GET  http://localhost:%d/brain/runes\n"
        "  GET  http://localhost:%d/brain/strategies\n"
        "  GET  http://localhost:%d/brain/fusion\n"
        "  GET  http://localhost:%d/brain/trust\n"
        "  GET  http://localhost:%d/brain/events/recent\n"
        "  GET  http://localhost:%d/brain/events/stream (SSE)\n"
        "  POST http://localhost:%d/brain/events\n"
        "  POST http://localhost:%d/brain/trust\n"
        "  POST http://localhost:%d/brain/shutdown\n"
        "Press ENTER to stop.\n",
        http_port, http_port, http_port,
        http_port, http_port, http_port, http_port, http_port,
        http_port, http_port, http_port, http_port, http_port);

    std::cin.get();

    shutdown.store(true);
    srv.stop();
    for (auto& t : threads) t.join();

    brain_log("[brain] shutdown complete.\n");
    return 0;
}
