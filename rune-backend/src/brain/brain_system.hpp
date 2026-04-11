#pragma once
/*
  PROJECT: QRrune / Cognitive Node Brain
  FILE: brain_system.hpp

  INTENT FOR GITHUB COPILOT CHAT / AI:
  - Declares all types, interfaces, and extension points used by brain_system.cpp.
  - Stub agent classes are marked with  // EXTEND:  comments that describe
    exactly what each agent should do when fully implemented.
  - To add a new agent:
      1. Subclass AgentBase and override tick().
      2. Register it with BrainDb::register_agent() in brain_main.cpp.
      3. Enqueue work via BrainDb::enqueue_event(); WorkerAgent picks it up.
      4. Emit results via AgentBase::emit() — EventBus routes to all subscribers.
*/

#include <atomic>
#include <chrono>
#include <cstdarg>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <sqlite3.h>
#include <nlohmann/json.hpp>

// ─────────────────────────────────────────────────────────────────────────────
// Utilities
// ─────────────────────────────────────────────────────────────────────────────

// Returns the current UTC time as an ISO-8601 string, e.g. "2026-04-10T09:29:08Z".
std::string utc_now();

// ─────────────────────────────────────────────────────────────────────────────
// BrainEvent — atomic message on the EventBus
// ─────────────────────────────────────────────────────────────────────────────

struct BrainEvent {
    std::string    source;     // emitting agent name
    std::string    type;       // event type token (e.g. "heartbeat", "gc_complete")
    nlohmann::json payload;    // arbitrary JSON
    std::string    timestamp;  // UTC ISO-8601, filled by EventBus::post() if empty
};

// ─────────────────────────────────────────────────────────────────────────────
// EventBus — SSE-style publish/subscribe message bus
//
// Agents call post() to broadcast.  HTTP /brain/events/stream subscribes and
// streams events to connected clients.  The in-memory ring buffer (last 1 000
// events) backs the /brain/events/recent snapshot endpoint.
// ─────────────────────────────────────────────────────────────────────────────

class EventBus {
public:
    using Handler = std::function<void(const BrainEvent&)>;

    // Publish an event; appends to ring buffer and notifies all live subscribers.
    void post(BrainEvent ev);

    // Register a handler.  Returns a subscription ID for later unsubscribe().
    int  subscribe(Handler h);

    // Remove a subscription.  Safe to call from inside a handler.
    void unsubscribe(int id);

    // Return the last n events as a JSON array (oldest first).
    nlohmann::json recent(int n = 50) const;

private:
    static constexpr int kMaxRing = 1000;
    mutable std::mutex       mtx_;
    std::deque<BrainEvent>   ring_;
    int                      next_id_{0};
    std::map<int, Handler>   subs_;
};

// ─────────────────────────────────────────────────────────────────────────────
// AllCounts — batched row-counts returned in a single SQL round-trip
// ─────────────────────────────────────────────────────────────────────────────

struct AllCounts {
    int nodes          = 0;
    int agents         = 0;
    int runes          = 0;
    int strategies     = 0;
    int fusion_entries = 0;
    int artifacts      = 0;
    int audit_entries  = 0;
    int pending_events = 0;
    int trust_entries  = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// BrainDb — thread-safe SQLite wrapper with the full brain schema
//
// All public methods acquire mtx_ before touching the DB handle.
// raw_exec() is unlocked and reserved for the constructor chain.
// Prefer sqlite3_bind_* over string concatenation throughout.
// ─────────────────────────────────────────────────────────────────────────────

class BrainDb {
public:
    explicit BrainDb(const std::string& path);
    ~BrainDb();

    BrainDb(const BrainDb&)            = delete;
    BrainDb& operator=(const BrainDb&) = delete;

    // ── Schema / seed ─────────────────────────────────────────────────────────
    void ensure_schema();   // CREATE TABLE/INDEX IF NOT EXISTS (idempotent)
    void seed_data();       // Insert default runes/strategies if tables are empty

    // ── Nodes ─────────────────────────────────────────────────────────────────
    void upsert_node(const std::string& node_id,
                     const std::string& label,
                     const std::string& status);

    // ── Agents ────────────────────────────────────────────────────────────────
    void register_agent(const std::string& name, const std::string& type);
    void update_agent_tick(const std::string& name);
    void update_agent_status(const std::string& name, const std::string& status);
    void increment_agent_errors(const std::string& name);

    // ── Runes ─────────────────────────────────────────────────────────────────
    nlohmann::json all_runes(int limit = 200, int offset = 0);
    nlohmann::json rune_by_name(const std::string& name);
    int  insert_rune(const std::string& name, const std::string& glyph,
                     const std::string& category, int level,
                     int ttl_seconds = 0);
    void increment_rune_usage(const std::string& name);
    void increment_rune_errors(const std::string& name);
    void set_rune_trust(const std::string& name, double trust);
    void set_rune_active(const std::string& name, int active);
    void set_rune_level(const std::string& name, int level);

    // ── Strategies ────────────────────────────────────────────────────────────
    nlohmann::json all_strategies(int limit = 100, int offset = 0);
    nlohmann::json strategy_by_name(const std::string& name);
    int  insert_strategy(const std::string& name, const std::string& description,
                         const nlohmann::json& rune_ids, int priority = 0);
    void increment_strategy_usage(const std::string& name);

    // ── Node events (async work queue) ────────────────────────────────────────
    int  enqueue_event(const std::string& source, const std::string& event_type,
                       const nlohmann::json& payload, int priority = 0);
    nlohmann::json pending_events(int limit = 10);
    void mark_processed(int event_id);
    void increment_event_attempts(int event_id);

    // ── Trust ledger ──────────────────────────────────────────────────────────
    void record_trust(const std::string& subject, const std::string& subject_type,
                      double delta, const std::string& reason);
    nlohmann::json aggregate_trust(const std::string& subject_type = "rune");
    int  apply_trust_updates(const std::string& subject_type = "rune");
    int  quarantine_low_trust_runes(double threshold = 0.1);
    int  level_up_high_trust_runes(double threshold = 3.0);

    // ── Fusion ────────────────────────────────────────────────────────────────
    nlohmann::json fusion_candidates(double min_trust = 1.0, int limit = 5);
    bool already_fused(const std::string& a, const std::string& b);
    int  insert_fusion(const std::string& input_a, const std::string& input_b,
                       const std::string& output_name, double score);
    nlohmann::json fusion_log(int limit = 50);

    // ── Artifacts ─────────────────────────────────────────────────────────────
    int insert_artifact(const std::string& type, const std::string& source,
                        const nlohmann::json& payload, int ttl_seconds = 86400);

    // ── Audit ─────────────────────────────────────────────────────────────────
    void audit_log(const std::string& agent, const std::string& action,
                   const std::string& subject,
                   const nlohmann::json& detail = nlohmann::json::object());

    // ── GC helpers ────────────────────────────────────────────────────────────
    int gc_expired_runes();
    int gc_expired_artifacts();
    int gc_old_events(int days_old = 1);
    int quarantine_high_error_runes(int error_threshold = 10);

    // ── Counts + size assessment ───────────────────────────────────────────────
    AllCounts all_counts();          // one SQL round-trip for all row-counts
    void record_size_assessment();   // snapshot all_counts() into size_assessment table

    // ── Raw exec (locked) ─────────────────────────────────────────────────────
    void exec(const char* sql);      // acquires mtx_

    // ── Extended node queries ──────────────────────────────────────────────────
    nlohmann::json nodes_list();

    // ── Typed event queries ────────────────────────────────────────────────────
    nlohmann::json pending_events_of_type(const std::string& type, int limit = 5);

    // ── Agent registry queries ─────────────────────────────────────────────────
    nlohmann::json all_agents();

    // ── Audit archival ─────────────────────────────────────────────────────────
    int archive_old_audit(int days_old = 30);

    // ── Symbols (14D QFS) ─────────────────────────────────────────────────────
    int  insert_symbol(const nlohmann::json& doc);
    nlohmann::json symbol_by_id(int id);
    nlohmann::json symbols_query(const std::string& radical,
                                  const std::string& layer, int limit = 50);

    // ── Knowledge substrate ────────────────────────────────────────────────────
    int  store_knowledge(const std::string& radical, const std::string& layer,
                         const nlohmann::json& entry);
    nlohmann::json query_knowledge(const std::string& radical, int limit = 20);

    // ── Agent checkpoints ─────────────────────────────────────────────────────
    int  save_checkpoint(const std::string& agent_name,
                          const nlohmann::json& state);
    nlohmann::json load_checkpoint(const std::string& agent_name, int checkpoint_id);

private:
    sqlite3*           db_  = nullptr;
    mutable std::mutex mtx_;

    void raw_exec(const char* sql);  // NOT locked — constructor use only
};

// ─────────────────────────────────────────────────────────────────────────────
// AgentBase — base class for all cognitive agents
//
// EXTEND: subclass this and override tick() to create new agent types.
//   • tick() should do one unit of work and return the desired sleep (ms).
//   • Call emit() to broadcast BrainEvents to the shared EventBus.
//   • Call db_.enqueue_event() to dispatch async work to WorkerAgent.
//   • The run() loop handles shutdown cleanly — no extra threading needed.
// ─────────────────────────────────────────────────────────────────────────────

class AgentBase {
public:
    AgentBase(std::string name, std::string type,
              BrainDb& db, EventBus& bus, std::atomic<bool>& shutdown);
    virtual ~AgentBase() = default;

    AgentBase(const AgentBase&)            = delete;
    AgentBase& operator=(const AgentBase&) = delete;
    AgentBase(AgentBase&&)                 = default;
    AgentBase& operator=(AgentBase&&)      = default;

    const std::string& name() const { return name_; }

    // Entry point: blocks until shutdown is set; calls tick() in a loop.
    void run();

protected:
    // Perform one unit of work.  Return desired sleep interval in milliseconds.
    virtual int tick() = 0;

    // Publish a BrainEvent on the shared bus.
    void emit(const std::string& type, nlohmann::json payload = {});

    // Thread-safe printf-style log to stdout.
    void log(const char* fmt, ...) const;

    std::string        name_;
    std::string        type_;
    BrainDb&           db_;
    EventBus&          bus_;
    std::atomic<bool>& shutdown_;
    int                tick_count_{0};

private:
    static std::mutex& log_mutex();
};

// ─────────────────────────────────────────────────────────────────────────────
// HeartAgent — node vitals, size-assessment snapshots, heartbeat events
//   Tick interval: 5 000 ms
//   Publishes: "heartbeat" with AllCounts; records size_assessment every 12 ticks
// ─────────────────────────────────────────────────────────────────────────────
class HeartAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// WorkerAgent — async event-queue consumer
//   Tick interval: 100 ms (busy) / 1 000 ms (idle)
//   Handles: heartbeat, chat_request, wallet_sign, strategy_execute, rune_trust
//   EXTEND: add new event_type branches in handle() to activate new pathways.
// ─────────────────────────────────────────────────────────────────────────────
class WorkerAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
private:
    void handle(const nlohmann::json& ev);
};

// ─────────────────────────────────────────────────────────────────────────────
// GCAgent — TTL reaping, quarantine, event pruning, artifact expiry
//   Tick interval: 30 000 ms
//   Publishes: "gc_complete" with counts of each cleanup category
// ─────────────────────────────────────────────────────────────────────────────
class GCAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// RuneTrustManager — aggregates trust_ledger and adjusts rune trust + level
//   Tick interval: 10 000 ms
//   Publishes: "trust_cycle" with quarantine and level-up counts
// ─────────────────────────────────────────────────────────────────────────────
class RuneTrustManager : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// RuneFusionEngine — discovers eligible rune pairs and fuses them
//   Tick interval: 20 000 ms
//   Publishes: "fusion_cycle" with count of new fused runes
//   Fusion score = (trust_a + trust_b)/2 × (level_a + level_b)/2
// ─────────────────────────────────────────────────────────────────────────────
class RuneFusionEngine : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
private:
    // Determine the output category for a pair of input categories.
    static std::string fused_category(const std::string& a, const std::string& b);
    // Derive the output rune name from the two input names.
    static std::string fused_name(const std::string& a, const std::string& b);
    // Compose a synthetic glyph string for the fused rune.
    static std::string fused_glyph(const std::string& a, const std::string& b);
};

// ─────────────────────────────────────────────────────────────────────────────
// Stub agents — implement tick() to activate these cognitive pathways.
// Each is pre-registered in brain_main.cpp so it appears in the agents table
// and emits "agent_idle" events, making extension auditable from day one.
// ─────────────────────────────────────────────────────────────────────────────

// EXTEND: ChatAgent
//   Handle "chat_request" events dispatched by WorkerAgent.
//   Integrate with an LLM endpoint via RUNE_LLM_URL (see rune-agents/chat-agent.js
//   for the Node.js reference).  Post replies as "chat_response" events and
//   persist to audit_log for heatmap analysis.
class ChatAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;   // stub — emits "agent_idle" every 5 s
};

// EXTEND: WalletAgent
//   Handle "wallet_sign" and "wallet_verify" events dispatched by WorkerAgent.
//   Reuse wallet::sign() / wallet::verify() from src/wallet/wallet.hpp.
//   Record every signing operation in audit_log for trust heatmap analysis.
class WalletAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;   // stub — emits "agent_idle" every 5 s
};

// EXTEND: NodeAgent
//   Monitor peer rune_brain instances listed in the nodes table.
//   Poll /brain/health on each peer, update nodes.last_seen and nodes.status,
//   and record trust deltas for unreachable peers via BrainDb::record_trust().
class NodeAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;   // stub — emits "agent_idle" every 10 s
};

// EXTEND: TorrentAgent
//   Manage distributed chunked data across the node mesh.
//   Enqueue "torrent_piece" events for WorkerAgent; track piece availability
//   in the artifacts table (type = "torrent_piece") with appropriate TTLs.
class TorrentAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;   // stub — emits "agent_idle" every 30 s
};

// EXTEND: EdgeAgent
//   Handle low-latency edge-compute tasks (QR decode, image hash, etc.).
//   Post results as "edge_result" events to the EventBus so visualisation
//   layers can render them in real time without polling the DB.
class EdgeAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// HousekeepingAgent — DB maintenance, audit archival, index optimization
//   Tick interval: 30 000 ms
//   Publishes: "housekeeping_cycle" with archived_audit count
// ─────────────────────────────────────────────────────────────────────────────
class HousekeepingAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// StrategyAgent — infers co-activation patterns; inserts strategies automatically
//   Tick interval: 30 000 ms
//   Publishes: "strategy_inferred" when a new strategy is created
// ─────────────────────────────────────────────────────────────────────────────
class StrategyAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// EpicRuneAgent — detects runes with trust≥3.0 & usage≥100; archives them
//   Tick interval: 60 000 ms
//   Publishes: "epic_rune_born" when a rune achieves epic status
// ─────────────────────────────────────────────────────────────────────────────
class EpicRuneAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// OverwatchAgent — monitors EventBus for anomalies; enforces system fairness
//   Tick interval: 15 000 ms
//   Publishes: "overwatch_alert" for anomalies, "overwatch_report" every 4 ticks
// ─────────────────────────────────────────────────────────────────────────────
class OverwatchAgent : public AgentBase {
public:
    OverwatchAgent(std::string name, std::string type,
                   BrainDb& db, EventBus& bus, std::atomic<bool>& shutdown);
    ~OverwatchAgent();
protected:
    int tick() override;
private:
    int sub_id_{-1};
    std::mutex               win_mtx_;
    std::map<std::string,int> event_window_;
    int gc_kills_{0};
};

// ─────────────────────────────────────────────────────────────────────────────
// LibrarianAgent — classifies 14D symbols; routes to appropriate agents
//   Tick interval: 20 000 ms
//   Publishes: "symbol_classified", "symbol_routed"
// ─────────────────────────────────────────────────────────────────────────────
class LibrarianAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// LoadSimulatorAgent — generates synthetic load for stress testing
//   Tick interval: 10 000 ms (first 5 ticks only, then every 30 ticks)
//   Publishes: "load_sim_tick"
// ─────────────────────────────────────────────────────────────────────────────
class LoadSimulatorAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};
