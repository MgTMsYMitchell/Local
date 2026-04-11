#pragma once
// QuantumFractalSystem — Self-healing engine

#include "math/AlignmentField.h"
#include "math/Tensor14.h"
#include "network/MeshNode.h"

#include <string>
#include <vector>

namespace qfs {

/// Monitors mesh nodes for alignment drift and corruption, and applies
/// automatic repair and rerouting.
class SelfHealingEngine {
public:
    explicit SelfHealingEngine(float corruption_threshold = 0.3f,
                                float repair_rate = 0.05f);

    /// Check whether a node's alignment field is corrupted.
    bool detect_corruption(const Tensor14& field) const;

    /// Recursively repair a corrupted node by applying the healing correction
    /// to its alignment vector and recompressing its state.
    void recursive_repair(MeshNode& node,
                          const QuantumInspiredCompressor& compressor);

    /// Mark a node as failed and return a list of alternative node IDs
    /// that traffic should be rerouted through.
    std::vector<std::string> reroute_around_failure(
        const std::string& failed_node_id,
        const std::vector<std::string>& all_node_ids) const;

    /// Run a full health check on all nodes in a list, repair any that
    /// are corrupted, and return the IDs of nodes that were repaired.
    std::vector<std::string> sweep(
        std::vector<MeshNode*>& nodes,
        const QuantumInspiredCompressor& compressor);

private:
    float corruption_threshold_;
    float repair_rate_;
};

} // namespace qfs
