#pragma once
// QuantumFractalSystem — Quantum-inspired data compressor

#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace qfs {

/// Compressed quantum-inspired state: a set of basis indices and amplitudes
/// that approximate the original data vector.
struct QState {
    std::vector<float> amplitudes;  // probability amplitudes (complex moduli)
    std::vector<int>   basis;       // basis vector indices

    /// Total "probability mass" — should be close to 1.0 for normalised states.
    float total_probability() const {
        float sum = 0.0f;
        for (float a : amplitudes) sum += a * a;
        return sum;
    }

    /// Number of basis elements retained.
    std::size_t rank() const { return basis.size(); }
};

/// A quantum-inspired compressor that represents data vectors as
/// superpositions of basis vectors.  Compression discards low-amplitude
/// components; decompression reconstructs an approximation.
class QuantumInspiredCompressor {
public:
    explicit QuantumInspiredCompressor(int basis_size = 32)
        : basis_size_(basis_size) {}

    /// Encode a float vector into a compressed QState.
    QState encode(const std::vector<float>& input) const;

    /// Decode a QState back to an approximate float vector.
    std::vector<float> decode(const QState& state, std::size_t output_size) const;

    /// Entangle two states (combine their basis representations).
    static QState entangle(const QState& a, const QState& b);

    /// Collapse a superposition to the single highest-amplitude basis element.
    static QState collapse(const QState& state);

private:
    int basis_size_;
};

} // namespace qfs
