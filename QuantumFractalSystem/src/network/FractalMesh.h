#pragma once
// QuantumFractalSystem — Fractal mesh topology and management

#include "network/MeshNode.h"
#include "network/Routing.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace qfs {

/// Manages a collection of MeshNodes arranged in a fractal topology.
/// Provides node lifecycle, message routing, and topology queries.
class FractalMesh {
public:
    FractalMesh() = default;

    /// Add a node to the mesh. Takes ownership.
    void add_node(std::unique_ptr<MeshNode> node);

    /// Remove a node by ID.
    bool remove_node(const std::string& id);

    /// Look up a node by ID (returns nullptr if not found).
    MeshNode* find_node(const std::string& id);
    const MeshNode* find_node(const std::string& id) const;

    /// Route a message from one node to another.
    /// Returns the intermediate hop path (may be direct or multi-hop).
    std::vector<std::string> route(const std::string& from,
                                    const std::string& to) const;

    /// Deliver all pending outgoing messages across the mesh.
    void flush_messages();

    /// Return all node IDs in the mesh.
    std::vector<std::string> node_ids() const;

    /// Total node count.
    std::size_t size() const;

private:
    mutable std::mutex mtx_;
    std::unordered_map<std::string, std::unique_ptr<MeshNode>> nodes_;
    RoutingTable routing_;
};

} // namespace qfs
