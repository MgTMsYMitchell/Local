#pragma once
// QuantumFractalSystem — SIMD-accelerated CPU compute pipeline

#include <cstdint>
#include <vector>

namespace qfs {

/// CPU compute pipeline using SIMD intrinsics where available.
/// Falls back to scalar loops when SIMD is not supported.
class SIMDCompute {
public:
    /// Element-wise multiply-accumulate:  output[i] = a[i] * b[i] + c[i]
    static std::vector<float> fma(const std::vector<float>& a,
                                   const std::vector<float>& b,
                                   const std::vector<float>& c);

    /// Parallel sum-of-squares (for energy calculations)
    static float sum_of_squares(const std::vector<float>& input);

    /// Parallel element-wise processing: apply a scalar function to each element
    static std::vector<float> process(const std::vector<float>& input);

    /// Normalise a vector in-place to unit length
    static void normalise(std::vector<float>& v);

    /// Dot product of two vectors
    static float dot(const std::vector<float>& a, const std::vector<float>& b);
};

} // namespace qfs
