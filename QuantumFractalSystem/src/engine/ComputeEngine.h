#pragma once
// QuantumFractalSystem — Compute engine (CPU + optional GPU dispatch)

#include "compression/QuantumInspiredCompressor.h"
#include "cpu/SIMDCompute.h"

#include <functional>
#include <future>
#include <string>
#include <vector>

namespace qfs {

/// High-level compute engine that dispatches work to CPU (SIMD) or GPU.
class ComputeEngine {
public:
    /// Process a data vector through the CPU pipeline.
    std::vector<float> compute_cpu(const std::vector<float>& input) const;

    /// Compress, process, and decompress a data vector.
    std::vector<float> compress_and_compute(
        const std::vector<float>& input,
        const QuantumInspiredCompressor& compressor) const;

    /// Run multiple compute tasks in parallel using std::async.
    std::vector<std::vector<float>> parallel_compute(
        const std::vector<std::vector<float>>& inputs) const;
};

} // namespace qfs
