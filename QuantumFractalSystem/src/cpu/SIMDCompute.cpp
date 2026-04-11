// QuantumFractalSystem — SIMD compute implementation

#include "cpu/SIMDCompute.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace qfs {

std::vector<float> SIMDCompute::fma(const std::vector<float>& a,
                                     const std::vector<float>& b,
                                     const std::vector<float>& c) {
    std::size_t n = std::min({a.size(), b.size(), c.size()});
    std::vector<float> out(n);
    for (std::size_t i = 0; i < n; ++i)
        out[i] = a[i] * b[i] + c[i];
    return out;
}

float SIMDCompute::sum_of_squares(const std::vector<float>& input) {
    float sum = 0.0f;
    for (float v : input) sum += v * v;
    return sum;
}

std::vector<float> SIMDCompute::process(const std::vector<float>& input) {
    // Default processing: clamp to [-1, 1] and apply tanh activation
    std::vector<float> out(input.size());
    for (std::size_t i = 0; i < input.size(); ++i)
        out[i] = std::tanh(input[i]);
    return out;
}

void SIMDCompute::normalise(std::vector<float>& v) {
    float mag = std::sqrt(sum_of_squares(v));
    if (mag < 1e-8f) return;
    float inv = 1.0f / mag;
    for (float& x : v) x *= inv;
}

float SIMDCompute::dot(const std::vector<float>& a,
                        const std::vector<float>& b) {
    std::size_t n = std::min(a.size(), b.size());
    float sum = 0.0f;
    for (std::size_t i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

} // namespace qfs
