// QuantumFractalSystem — MeshNode implementation

#include "network/MeshNode.h"

namespace qfs {

MeshNode::MeshNode(std::string id) : id_(std::move(id)) {}

void MeshNode::set_alignment(const Vec14& a) {
    alignment_ = a;
}

void MeshNode::compress_state(const QuantumInspiredCompressor& compressor) {
    // Compress the alignment vector as the node's state snapshot
    std::vector<float> state_vec(alignment_.v.begin(), alignment_.v.end());
    compressed_state_ = compressor.encode(state_vec);
}

void MeshNode::receive_task(const std::vector<float>& data) {
    if (on_receive_) on_receive_("task", data);
}

void MeshNode::send_to(const std::string& node_id, const std::vector<float>& data) {
    std::lock_guard<std::mutex> lk(mtx_);
    outbox_.push({node_id, data});
}

std::vector<MeshNode::OutMessage> MeshNode::drain_outbox() {
    std::lock_guard<std::mutex> lk(mtx_);
    std::vector<OutMessage> msgs;
    while (!outbox_.empty()) {
        msgs.push_back(std::move(outbox_.front()));
        outbox_.pop();
    }
    return msgs;
}

void MeshNode::on_receive(DataCallback cb) {
    on_receive_ = std::move(cb);
}

void MeshNode::deliver(const std::string& from, const std::vector<float>& data) {
    if (on_receive_) on_receive_(from, data);
}

} // namespace qfs
