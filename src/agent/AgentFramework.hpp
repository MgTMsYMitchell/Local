// ============================================================================
// src/agent/AgentFramework.hpp
// ============================================================================
//
// PROJECT: QRrune / Distributed Compute Mesh
// MODULE : Agent Framework  (src/agent/)
//
// INTENT FOR GITHUB COPILOT CHAT / AI:
//   This header is the single declaration file for the Agent Framework module.
//   It mirrors the components shown in the Mermaid diagram in
//   docs/ARCHITECTURE_MASTER.md §2 and is designed so that:
//     • AgentManager owns the lifecycle of all registered agents.
//     • TaskGraph / TaskNode represent a DAG of work units.
//     • DependencyResolver linearises the DAG before execution.
//     • ToolRegistry maps string names to callable Tool implementations.
//     • MemoryStore provides short-term (in-process) and long-term (SQLite)
//       memory for agents.
//     • ExecutionLoop drives the action/observation cycle.
//     • ActionDispatcher routes actions to the correct tool.
//     • ObservationCollector records the results of each action.
//
// CONVENTIONS (same as rune-backend/src/brain/brain_system.hpp):
//   • C++20, no exceptions in hot paths.
//   • nlohmann::json for all structured data.
//   • sqlite3_bind_* for all SQLite queries (no string concatenation).
//   • Prefer header-only or FetchContent-fetched deps.
//
// ============================================================================

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace agent {

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────────────────────────────────────

class TaskGraph;
class ToolRegistry;
class MemoryStore;
class ExecutionLoop;
class ActionDispatcher;
class ObservationCollector;

// ─────────────────────────────────────────────────────────────────────────────
// TaskStatus — lifecycle state of a single TaskNode
// ─────────────────────────────────────────────────────────────────────────────

enum class TaskStatus {
    Pending,    // not yet started
    Ready,      // all dependencies satisfied; eligible to run
    Running,    // currently being executed
    Done,       // completed successfully
    Failed,     // terminal error
};

// ─────────────────────────────────────────────────────────────────────────────
// TaskNode — a single unit of work inside a TaskGraph
//
// EXTEND: populate `tool_name` and `input` to describe what the node should
//   do.  The ExecutionLoop resolves dependencies, dispatches the action via
//   ActionDispatcher, and writes the result into `output`.
// ─────────────────────────────────────────────────────────────────────────────

struct TaskNode {
    int                      id{0};
    std::string              label;
    std::string              tool_name;      // key into ToolRegistry
    nlohmann::json           input;          // arguments forwarded to the tool
    nlohmann::json           output;         // written by ExecutionLoop on success
    TaskStatus               status{TaskStatus::Pending};
    std::vector<int>         dep_ids;        // IDs of prerequisite TaskNodes
    std::chrono::steady_clock::time_point started_at;
    std::chrono::steady_clock::time_point finished_at;
};

// ─────────────────────────────────────────────────────────────────────────────
// DependencyResolver — topological sort of a TaskGraph
//
// resolve() returns the nodes in a valid execution order.  Cycles are reported
// via the returned optional (nullopt = success, string = cycle description).
// ─────────────────────────────────────────────────────────────────────────────

class DependencyResolver {
public:
    // Returns linearised node IDs in topological order, or nullopt on cycle.
    std::optional<std::vector<int>> resolve(const TaskGraph& graph) const;
};

// ─────────────────────────────────────────────────────────────────────────────
// TaskGraph — directed acyclic graph of TaskNodes
// ─────────────────────────────────────────────────────────────────────────────

class TaskGraph {
public:
    // Add a node; returns the assigned node ID.
    int add_node(TaskNode node);

    // Add a directed edge: `from` must complete before `to` can start.
    void add_edge(int from_id, int to_id);

    // Retrieve a node by ID.  Asserts if ID is unknown.
    TaskNode&       node(int id);
    const TaskNode& node(int id) const;

    // All nodes in insertion order.
    const std::vector<TaskNode>& nodes() const { return nodes_; }

    // Count nodes by status.
    int count(TaskStatus s) const;

    // Serialise the graph to JSON (for audit / logging).
    nlohmann::json to_json() const;

private:
    std::vector<TaskNode>        nodes_;
    std::unordered_map<int, int> id_to_index_;  // node id → index in nodes_
    int                          next_id_{1};
    mutable std::mutex           mtx_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Tool — abstract callable registered in the ToolRegistry
//
// EXTEND: implement the call() method to connect agent actions to LLM, mesh,
//   compute, or storage subsystems.
// ─────────────────────────────────────────────────────────────────────────────

class Tool {
public:
    virtual ~Tool() = default;

    // Execute the tool with the given JSON input; return JSON output.
    // Return an object with `{"error": "..."}` on failure — never throw.
    virtual nlohmann::json call(const nlohmann::json& input) noexcept = 0;

    // Human-readable description forwarded to the LLM for tool selection.
    virtual std::string description() const = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// ToolRegistry — maps string names to Tool implementations
//
// Built-in tool namespaces (populated by AgentManager::wire_tools()):
//   "llm.*"     — LLM Integration Layer
//   "mesh.*"    — Mesh Networking Layer
//   "compute.*" — Compute Pipeline
//   "storage.*" — Storage Layer
// ─────────────────────────────────────────────────────────────────────────────

class ToolRegistry {
public:
    // Register a tool; overwrites any existing registration for `name`.
    void register_tool(const std::string& name, std::shared_ptr<Tool> tool);

    // Look up a tool.  Returns nullptr if not found.
    std::shared_ptr<Tool> find(const std::string& name) const;

    // All registered tool names.
    std::vector<std::string> names() const;

    // Serialise registry to JSON schema fragment (for LLM function-calling).
    nlohmann::json schema() const;

private:
    mutable std::mutex                                   mtx_;
    std::unordered_map<std::string, std::shared_ptr<Tool>> tools_;
};

// ─────────────────────────────────────────────────────────────────────────────
// ShortTermMemory — in-process key/value store, evicts oldest on overflow
// ─────────────────────────────────────────────────────────────────────────────

class ShortTermMemory {
public:
    explicit ShortTermMemory(std::size_t capacity = 256);

    void             set(const std::string& key, nlohmann::json value);
    nlohmann::json   get(const std::string& key) const;   // null json if missing
    bool             has(const std::string& key) const;
    void             clear();
    std::size_t      size() const;

private:
    mutable std::mutex                             mtx_;
    std::unordered_map<std::string, nlohmann::json> store_;
    std::vector<std::string>                       order_;  // insertion order for LRU eviction
    std::size_t                                    capacity_;
};

// ─────────────────────────────────────────────────────────────────────────────
// LongTermMemory — SQLite-backed persistent key/value store
//
// Table: agent_memory(agent TEXT, key TEXT, value TEXT, updated_at TEXT)
// PRIMARY KEY (agent, key)
// ─────────────────────────────────────────────────────────────────────────────

class LongTermMemory {
public:
    // db must already be open and have ensure_schema() called.
    LongTermMemory(sqlite3* db, std::string agent_name);

    void           set(const std::string& key, const nlohmann::json& value);
    nlohmann::json get(const std::string& key) const;   // null json if missing
    void           remove(const std::string& key);
    nlohmann::json all() const;   // all key/value pairs as JSON object

private:
    sqlite3*    db_;
    std::string agent_name_;
    mutable std::mutex mtx_;
};

// ─────────────────────────────────────────────────────────────────────────────
// MemoryStore — aggregates ShortTermMemory and LongTermMemory
// ─────────────────────────────────────────────────────────────────────────────

class MemoryStore {
public:
    MemoryStore(sqlite3* db, const std::string& agent_name,
                std::size_t stm_capacity = 256);

    ShortTermMemory& stm() { return stm_; }
    LongTermMemory&  ltm() { return ltm_; }

    const ShortTermMemory& stm() const { return stm_; }
    const LongTermMemory&  ltm() const { return ltm_; }

private:
    ShortTermMemory stm_;
    LongTermMemory  ltm_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Observation — result of a single tool invocation
// ─────────────────────────────────────────────────────────────────────────────

struct Observation {
    int            task_id;
    std::string    tool_name;
    nlohmann::json input;
    nlohmann::json output;
    bool           ok{true};        // false if tool returned an "error" key
    std::chrono::steady_clock::time_point timestamp;
};

// ─────────────────────────────────────────────────────────────────────────────
// ObservationCollector — ordered log of Observations for the current episode
// ─────────────────────────────────────────────────────────────────────────────

class ObservationCollector {
public:
    void              push(Observation obs);
    const Observation& last() const;        // asserts if empty
    std::size_t        size() const;
    void               clear();

    // Serialise all observations to JSON array.
    nlohmann::json to_json() const;

private:
    mutable std::mutex         mtx_;
    std::vector<Observation>   log_;
};

// ─────────────────────────────────────────────────────────────────────────────
// ActionDispatcher — routes a TaskNode's tool_name+input to ToolRegistry
//
// On success writes `output` into the node and records an Observation.
// On failure marks the node Failed and records the error Observation.
// ─────────────────────────────────────────────────────────────────────────────

class ActionDispatcher {
public:
    ActionDispatcher(ToolRegistry& registry, ObservationCollector& collector);

    // Dispatch the tool call described by `node`; mutates node.status and node.output.
    void dispatch(TaskNode& node);

private:
    ToolRegistry&         registry_;
    ObservationCollector& collector_;
};

// ─────────────────────────────────────────────────────────────────────────────
// ExecutionLoop — drives a TaskGraph to completion
//
// EXTEND: override on_tick() to inject custom logic between steps.
// ─────────────────────────────────────────────────────────────────────────────

class ExecutionLoop {
public:
    ExecutionLoop(ToolRegistry& registry, MemoryStore& memory,
                  ActionDispatcher& dispatcher, ObservationCollector& collector);

    // Run all tasks in `graph` to completion (or until shutdown is set).
    // Returns true if all tasks reached Done, false if any Failed.
    bool run(TaskGraph& graph, std::atomic<bool>& shutdown);

protected:
    // Called once per iteration; override for custom inter-step logic.
    virtual void on_tick(const TaskGraph& graph) {}

private:
    ToolRegistry&         registry_;
    MemoryStore&          memory_;
    ActionDispatcher&     dispatcher_;
    ObservationCollector& collector_;
    DependencyResolver    resolver_;
};

// ─────────────────────────────────────────────────────────────────────────────
// AgentManager — top-level owner of the Agent Framework
//
// Typical usage:
//   AgentManager mgr(db, shutdown);
//   mgr.wire_tools();               // register built-in tools
//   auto& graph = mgr.new_graph();  // create a task graph
//   graph.add_node({.label="chat", .tool_name="llm.chat", .input={...}});
//   mgr.run(graph);
// ─────────────────────────────────────────────────────────────────────────────

class AgentManager {
public:
    AgentManager(sqlite3* db, std::atomic<bool>& shutdown,
                 std::string agent_name = "agent_manager");

    // Register the built-in tool namespaces (llm.*, mesh.*, compute.*, storage.*).
    // Concrete implementations must be linked in by each subsystem module.
    void wire_tools();

    // Register an arbitrary tool manually.
    void register_tool(const std::string& name, std::shared_ptr<Tool> tool);

    // Allocate a fresh TaskGraph; the manager retains ownership.
    TaskGraph& new_graph();

    // Execute a graph synchronously; returns true on full success.
    bool run(TaskGraph& graph);

    // Access the shared memory store for this manager instance.
    MemoryStore&          memory()   { return memory_; }
    ObservationCollector& observer() { return collector_; }
    ToolRegistry&         tools()    { return registry_; }

private:
    sqlite3*                         db_;
    std::atomic<bool>&               shutdown_;
    std::string                      name_;
    ToolRegistry                     registry_;
    MemoryStore                      memory_;
    ObservationCollector             collector_;
    ActionDispatcher                 dispatcher_;
    ExecutionLoop                    loop_;
    std::vector<std::unique_ptr<TaskGraph>> graphs_;
    mutable std::mutex               mtx_;
};

} // namespace agent
