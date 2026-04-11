#pragma once
// QuantumFractalSystem — 14×14 tensor (matrix) type

#include "Vector14.h"
#include <array>
#include <cmath>
#include <cstring>

namespace qfs {

/// A 14×14 floating-point tensor (symmetric or general).
struct Tensor14 {
    static constexpr int kDim = Vec14::kDim;
    std::array<std::array<float, kDim>, kDim> t{};

    Tensor14() = default;

    float& operator()(int r, int c) { return t[r][c]; }
    const float& operator()(int r, int c) const { return t[r][c]; }

    /// Identity tensor
    static Tensor14 identity() {
        Tensor14 I;
        for (int i = 0; i < kDim; ++i) I.t[i][i] = 1.0f;
        return I;
    }

    /// Outer product:  v ⊗ w
    static Tensor14 outer(const Vec14& a, const Vec14& b) {
        Tensor14 r;
        for (int i = 0; i < kDim; ++i)
            for (int j = 0; j < kDim; ++j)
                r.t[i][j] = a.v[i] * b.v[j];
        return r;
    }

    /// Matrix-vector multiply:  T · v
    Vec14 apply(const Vec14& v) const {
        Vec14 r;
        for (int i = 0; i < kDim; ++i) {
            float sum = 0.0f;
            for (int j = 0; j < kDim; ++j) sum += t[i][j] * v.v[j];
            r.v[i] = sum;
        }
        return r;
    }

    /// Element-wise add
    Tensor14 operator+(const Tensor14& rhs) const {
        Tensor14 r;
        for (int i = 0; i < kDim; ++i)
            for (int j = 0; j < kDim; ++j)
                r.t[i][j] = t[i][j] + rhs.t[i][j];
        return r;
    }

    /// Scalar multiply
    Tensor14 operator*(float s) const {
        Tensor14 r;
        for (int i = 0; i < kDim; ++i)
            for (int j = 0; j < kDim; ++j)
                r.t[i][j] = t[i][j] * s;
        return r;
    }

    /// Frobenius norm:  sqrt(sum of t[i][j]^2)
    float frobenius_norm() const {
        float sum = 0.0f;
        for (int i = 0; i < kDim; ++i)
            for (int j = 0; j < kDim; ++j)
                sum += t[i][j] * t[i][j];
        return std::sqrt(sum);
    }

    /// Trace: sum of diagonal elements
    float trace() const {
        float sum = 0.0f;
        for (int i = 0; i < kDim; ++i) sum += t[i][i];
        return sum;
    }

    /// Serialise to raw bytes (784 bytes)
    void to_bytes(void* dst) const { std::memcpy(dst, t.data(), sizeof(t)); }

    /// Deserialise from raw bytes
    static Tensor14 from_bytes(const void* src) {
        Tensor14 r;
        std::memcpy(r.t.data(), src, sizeof(r.t));
        return r;
    }
};

} // namespace qfs
