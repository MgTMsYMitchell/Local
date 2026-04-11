// QuantumFractalSystem — Self-healing engine implementation

#include "engine/SelfHealingEngine.h"

namespace qfs {

SelfHealingEngine::SelfHealingEngine(float corruption_threshold,
                                      float repair_rate)
    : corruption_threshold_(corruption_threshold)
    , repair_rate_(repair_rate) {}

bool SelfHealingEngine::detect_corruption(const Tensor14& field) const {
    return AlignmentField::is_corrupted(field, corruption_threshold_);
}

void SelfHealingEngine::recursive_repair(
    MeshNode& node, const QuantumInspiredCompressor& compressor) {

    // Build the node's alignment field
    Tensor14 field = AlignmentField::compute_field(node.alignment());

    // Iteratively apply healing corrections until the field stabilises
    constexpr int kMaxIter = 100;
    for (int i = 0; i < kMaxIter; ++i) {
        if (!AlignmentField::is_corrupted(field, corruption_threshold_))
            break;

        Vec14 correction = AlignmentField::compute_healing(
            field, node.alignment(), repair_rate_);
        Vec14 new_alignment = node.alignment() + correction;
        node.set_alignment(new_alignment.normalised());

        // Add a small turbulence to escape local minima
        Vec14 turb = AlignmentField::compute_turbulence(field, 0.001f);
        node.set_alignment((node.alignment() + turb).normalised());

        field = AlignmentField::compute_field(node.alignment());
    }

    // Recompress the healed state
    node.compress_state(compressor);
    node.set_healthy(true);
}

std::vector<std::string> SelfHealingEngine::reroute_around_failure(
    const std::string& failed_node_id,
    const std::vector<std::string>& all_node_ids) const {

    std::vector<std::string> alternatives;
    for (auto& id : all_node_ids) {
        if (id != failed_node_id)
            alternatives.push_back(id);
    }
    return alternatives;
}

std::vector<std::string> SelfHealingEngine::sweep(
    std::vector<MeshNode*>& nodes,
    const QuantumInspiredCompressor& compressor) {

    std::vector<std::string> repaired;

    for (MeshNode* node : nodes) {
        Tensor14 field = AlignmentField::compute_field(node->alignment());
        if (detect_corruption(field)) {
            node->set_healthy(false);
            recursive_repair(*node, compressor);
            repaired.push_back(node->id());
        }
    }

    return repaired;
}

} // namespace qfs
