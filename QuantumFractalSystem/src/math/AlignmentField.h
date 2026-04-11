#pragma once
// QuantumFractalSystem — Alignment field operations over 14D space

#include "Tensor14.h"
#include "Vector14.h"
#include <cmath>
#include <random>

namespace qfs {

/// Computes alignment fields, drift vectors, healing corrections, and
/// turbulence from Tensor14 fields.  The alignment field is defined as
/// the outer product of a node's position/orientation in 14D space.
class AlignmentField {
public:
    /// Build the alignment field tensor from a node's 14D alignment vector.
    /// Field = normalised outer product + identity damping.
    static Tensor14 compute_field(const Vec14& alignment, float damping = 0.01f) {
        Vec14 n = alignment.normalised();
        Tensor14 field = Tensor14::outer(n, n);
        Tensor14 I = Tensor14::identity();
        return field + I * damping;
    }

    /// Compute the drift vector — how far a node has moved from alignment.
    /// drift = field · alignment − alignment  (deviation from self-consistency)
    static Vec14 compute_drift(const Tensor14& field, const Vec14& alignment) {
        return field.apply(alignment) - alignment;
    }

    /// Compute a healing correction vector that steers a node back toward
    /// alignment.  healing = −rate × drift
    static Vec14 compute_healing(const Tensor14& field,
                                  const Vec14& alignment,
                                  float rate = 0.05f) {
        Vec14 drift = compute_drift(field, alignment);
        return drift * (-rate);
    }

    /// Compute a stochastic turbulence perturbation.
    /// Uses the field's Frobenius norm to scale Gaussian noise.
    static Vec14 compute_turbulence(const Tensor14& field, float scale = 0.01f) {
        static thread_local std::mt19937 gen{std::random_device{}()};
        std::normal_distribution<float> dist(0.0f, scale * field.frobenius_norm());

        Vec14 noise;
        for (int i = 0; i < Vec14::kDim; ++i)
            noise.v[i] = dist(gen);
        return noise;
    }

    /// Check whether a field has diverged beyond a corruption threshold.
    static bool is_corrupted(const Tensor14& field, float threshold = 0.3f) {
        float expected = std::sqrt(static_cast<float>(Vec14::kDim)); // identity norm
        float actual = field.frobenius_norm();
        return std::abs(actual - expected) > threshold * expected;
    }
};

} // namespace qfs
