// QuantumFractalSystem — FractalMesh implementation

#include "network/FractalMesh.h"

namespace qfs {

void FractalMesh::add_node(std::unique_ptr<MeshNode> node) {
    std::lock_guard<std::mutex> lk(mtx_);
    std::string id = node->id();

    // Register routes to all existing nodes (direct, 1-hop)
    for (auto& [existing_id, _] : nodes_) {
        routing_.add_route(id, existing_id);
        routing_.add_route(existing_id, id);
    }

    nodes_[id] = std::move(node);
}

bool FractalMesh::remove_node(const std::string& id) {
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return false;

    routing_.remove_routes_for(id);
    nodes_.erase(it);
    return true;
}

MeshNode* FractalMesh::find_node(const std::string& id) {
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = nodes_.find(id);
    return it != nodes_.end() ? it->second.get() : nullptr;
}

const MeshNode* FractalMesh::find_node(const std::string& id) const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = nodes_.find(id);
    return it != nodes_.end() ? it->second.get() : nullptr;
}

std::vector<std::string> FractalMesh::route(const std::string& from,
                                              const std::string& to) const {
    std::lock_guard<std::mutex> lk(mtx_);
    return routing_.find_path(from, to);
}

void FractalMesh::flush_messages() {
    std::lock_guard<std::mutex> lk(mtx_);

    for (auto& [id, node] : nodes_) {
        auto msgs = node->drain_outbox();
        for (auto& msg : msgs) {
            auto path = routing_.find_path(id, msg.target);
            if (path.empty()) continue;

            // Deliver to the final destination
            auto target_it = nodes_.find(msg.target);
            if (target_it != nodes_.end()) {
                target_it->second->deliver(id, msg.data);
            }
        }
    }
}

std::vector<std::string> FractalMesh::node_ids() const {
    std::lock_guard<std::mutex> lk(mtx_);
    std::vector<std::string> ids;
    ids.reserve(nodes_.size());
    for (auto& [id, _] : nodes_) ids.push_back(id);
    return ids;
}

std::size_t FractalMesh::size() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return nodes_.size();
}

} // namespace qfs
