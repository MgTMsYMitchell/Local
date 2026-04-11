#pragma once
/*
  PROJECT: QRrune / Cognitive Node Brain
  FILE: brain_system.hpp

  INTENT FOR GITHUB COPILOT CHAT / AI:
  - Declares all types, interfaces, and extension points used by brain_system.cpp.
  - All 16 cognitive agents have real tick() implementations.
  - WorkerAgent owns: heartbeat, strategy_execute, rune_trust.
  - Specialized agents consume their own event types directly from the queue:
      ChatAgent      → "chat_request"
      WalletAgent    → "wallet_sign", "wallet_verify"
      TorrentAgent   → "torrent_piece"
      EdgeAgent      → "edge_task"
      LibrarianAgent → "classify_symbol"
  - To add a new agent:
      1. Subclass AgentBase and override tick().
      2. Register it with BrainDb::register_agent() in brain_main.cpp.
      3. Enqueue work via BrainDb::enqueue_event(); consume with pending_events_of_type().
      4. Emit results via AgentBase::emit() — EventBus routes to all subscribers.
      5. If WorkerAgent should NOT consume the new event type, add it to
         kSpecializedTypes in WorkerAgent::tick().
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

    // ── RQ^R2 Dream buffer ────────────────────────────────────────────────────
    int  insert_dream(const nlohmann::json& doc);
    nlohmann::json dreams_query(const std::string& dream_type = "",
                                 double min_confidence = 0.0, int limit = 50);

    // ── RQ^R2 Theory buffer ───────────────────────────────────────────────────
    int  insert_theory(const nlohmann::json& doc);
    nlohmann::json theories_query(const std::string& status = "", int limit = 50);

    // ── RQ^R2 Trust escalation ────────────────────────────────────────────────
    bool trust_escalate(int symbol_id, const std::string& new_state);

    // ── RQ^R2 Tendrils (mycelium routing) ────────────────────────────────────
    int  insert_tendril(const nlohmann::json& doc);
    nlohmann::json tendrils_query(int source = 0, int target = 0,
                                   const std::string& tendril_type = "",
                                   int limit = 50);

    // ── RQ^R2 Radicals registry ───────────────────────────────────────────────
    nlohmann::json radicals_query(int tier = 0, const std::string& domain = "");

    // ── RQ^R2 Consolidation cycle ─────────────────────────────────────────────
    nlohmann::json consolidation_tick();

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
//   Handles: heartbeat, strategy_execute, rune_trust
//   Skips event types owned by specialized agents (chat_request, wallet_sign,
//   wallet_verify, torrent_piece, edge_task, classify_symbol).
//   EXTEND: add new Worker-owned event_type branches in handle(); also add
//           new specialized types to kSpecializedTypes in tick() to protect them.
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
// Specialized agents — each has a real tick() implementation and consumes
// its own event type(s) directly from the queue via pending_events_of_type().
// WorkerAgent skips these event types (see kSpecializedTypes in tick()).
// ─────────────────────────────────────────────────────────────────────────────

// ChatAgent — LLM integration via RUNE_LLM_URL / RUNE_LLM_MODEL env vars
//   Consumes: "chat_request" events from the queue.
//   Posts replies as "chat_response" events; persists to audit_log.
//   Falls back to echo mode when RUNE_LLM_URL is not set.
class ChatAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// WalletAgent — Ed25519 signing and verification
//   Consumes: "wallet_sign" and "wallet_verify" events from the queue.
//   Uses wallet::sign() / wallet::verify() from src/wallet/wallet.hpp.
//   Records every operation in audit_log; emits trust delta on success.
class WalletAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// NodeAgent — peer rune_brain health monitoring
//   Consumes: nodes table (polls each peer's /brain/health endpoint).
//   Updates nodes.last_seen / nodes.status; records trust deltas for
//   unreachable peers via BrainDb::record_trust().
class NodeAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// TorrentAgent — distributed piece tracking across the node mesh
//   Consumes: "torrent_piece" events from the queue.
//   Stores pieces as artifacts (type="torrent_piece", TTL=7 days);
//   emits periodic "torrent_status" reports with peer count.
class TorrentAgent : public AgentBase {
public:
    using AgentBase::AgentBase;
protected:
    int tick() override;
};

// EdgeAgent — low-latency edge-compute tasks (hashing, QR detection)
//   Consumes: "edge_task" events from the queue.
//   Supports task_type="hash" (FNV-1a 64-bit); "qr_detect" returns a
//   not-linked notice until a QR library is linked.
//   Posts results as "edge_result" events; audits every completed task.
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
