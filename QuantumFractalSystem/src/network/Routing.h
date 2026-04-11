#pragma once
// QuantumFractalSystem — Mesh routing table

#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace qfs {

/// Simple adjacency-list routing table with BFS path finding.
class RoutingTable {
public:
    /// Add a bidirectional edge between two nodes.
    void add_route(const std::string& a, const std::string& b) {
        adj_[a].insert(b);
        adj_[b].insert(a);
    }

    /// Remove all routes involving a node.
    void remove_routes_for(const std::string& id) {
        auto it = adj_.find(id);
        if (it != adj_.end()) {
            for (auto& neighbour : it->second)
                adj_[neighbour].erase(id);
            adj_.erase(it);
        }
    }

    /// Find the shortest path from `from` to `to` using BFS.
    /// Returns the path including both endpoints, or empty if unreachable.
    std::vector<std::string> find_path(const std::string& from,
                                        const std::string& to) const {
        if (from == to) return {from};

        std::unordered_map<std::string, std::string> parent;
        std::queue<std::string> q;
        q.push(from);
        parent[from] = "";

        while (!q.empty()) {
            std::string cur = q.front();
            q.pop();

            auto it = adj_.find(cur);
            if (it == adj_.end()) continue;

            for (auto& next : it->second) {
                if (parent.count(next)) continue;
                parent[next] = cur;
                if (next == to) {
                    // Reconstruct path
                    std::vector<std::string> path;
                    for (std::string n = to; !n.empty(); n = parent[n])
                        path.push_back(n);
                    std::reverse(path.begin(), path.end());
                    return path;
                }
                q.push(next);
            }
        }

        return {}; // unreachable
    }

    /// Number of known nodes.
    std::size_t node_count() const { return adj_.size(); }

private:
    std::unordered_map<std::string, std::unordered_set<std::string>> adj_;
};

} // namespace qfs
