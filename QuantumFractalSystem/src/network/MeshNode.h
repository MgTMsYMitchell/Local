#pragma once
// QuantumFractalSystem — Mesh node in the distributed compute network

#include "compression/QuantumInspiredCompressor.h"
#include "math/Vector14.h"

#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace qfs {

/// Callback type for incoming data from peers.
using DataCallback = std::function<void(const std::string& from,
                                         const std::vector<float>& data)>;

/// A single node in the fractal mesh network.
class MeshNode {
public:
    explicit MeshNode(std::string id);

    const std::string& id() const { return id_; }
    const Vec14& alignment() const { return alignment_; }
    const QState& compressed_state() const { return compressed_state_; }

    /// Set the node's alignment vector.
    void set_alignment(const Vec14& a);

    /// Compress the node's current state.
    void compress_state(const QuantumInspiredCompressor& compressor);

    /// Receive a task payload.
    void receive_task(const std::vector<float>& data);

    /// Enqueue data to send to another node (via the mesh router).
    void send_to(const std::string& node_id, const std::vector<float>& data);

    /// Drain the outgoing message queue.
    struct OutMessage {
        std::string target;
        std::vector<float> data;
    };
    std::vector<OutMessage> drain_outbox();

    /// Register callback for incoming data.
    void on_receive(DataCallback cb);

    /// Deliver data from a peer.
    void deliver(const std::string& from, const std::vector<float>& data);

    /// Check if this node is healthy (alignment not drifted).
    bool is_healthy() const { return healthy_; }
    void set_healthy(bool h) { healthy_ = h; }

private:
    std::string id_;
    Vec14 alignment_;
    QState compressed_state_;
    bool healthy_ = true;

    std::mutex mtx_;
    std::queue<OutMessage> outbox_;
    DataCallback on_receive_;
};

} // namespace qfs
