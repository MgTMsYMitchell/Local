#pragma once
// QuantumFractalSystem — GPU dispatcher stub
// This header defines the interface for GPU-accelerated computation.
// The actual CUDA kernels are in ComputeKernels.cu and are only compiled
// when QFS_ENABLE_GPU is ON.

#include <cstddef>
#include <vector>

namespace qfs {

/// GPU dispatcher — sends work to CUDA kernels when available,
/// falls back to CPU otherwise.
class GPUDispatcher {
public:
    /// Check if CUDA GPU is available at runtime.
    static bool is_available() {
#ifdef QFS_HAS_GPU
        return true;   // Will be refined with cudaGetDeviceCount
#else
        return false;
#endif
    }

    /// Dispatch a compute kernel to the GPU (or CPU fallback).
    /// Returns the processed output vector.
    static std::vector<float> dispatch(const std::vector<float>& input,
                                        std::size_t output_size);
};

} // namespace qfs
