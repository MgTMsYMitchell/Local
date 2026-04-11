// QuantumFractalSystem — Compute engine implementation

#include "engine/ComputeEngine.h"

#include <algorithm>
#include <future>

namespace qfs {

std::vector<float> ComputeEngine::compute_cpu(
    const std::vector<float>& input) const {
    return SIMDCompute::process(input);
}

std::vector<float> ComputeEngine::compress_and_compute(
    const std::vector<float>& input,
    const QuantumInspiredCompressor& compressor) const {

    // Encode → process the amplitudes → decode
    QState state = compressor.encode(input);

    // Process the amplitudes through the CPU pipeline
    state.amplitudes = SIMDCompute::process(state.amplitudes);

    return compressor.decode(state, input.size());
}

std::vector<std::vector<float>> ComputeEngine::parallel_compute(
    const std::vector<std::vector<float>>& inputs) const {

    std::vector<std::future<std::vector<float>>> futures;
    futures.reserve(inputs.size());

    for (auto& input : inputs) {
        futures.push_back(std::async(std::launch::async,
            [this, &input]() { return compute_cpu(input); }));
    }

    std::vector<std::vector<float>> results;
    results.reserve(futures.size());
    for (auto& f : futures)
        results.push_back(f.get());

    return results;
}

} // namespace qfs
