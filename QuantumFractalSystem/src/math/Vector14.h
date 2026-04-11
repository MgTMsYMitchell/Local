#pragma once
// QuantumFractalSystem — 14-dimensional vector type

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <numeric>

namespace qfs {

/// A 14-dimensional floating-point vector.
struct Vec14 {
    static constexpr int kDim = 14;
    std::array<float, kDim> v{};

    Vec14() = default;

    explicit Vec14(float fill) { v.fill(fill); }

    Vec14(std::initializer_list<float> il) {
        auto it = il.begin();
        for (int i = 0; i < kDim && it != il.end(); ++i, ++it)
            v[i] = *it;
    }

    float& operator[](int i) { return v[i]; }
    const float& operator[](int i) const { return v[i]; }

    // Element-wise arithmetic
    Vec14 operator+(const Vec14& rhs) const {
        Vec14 r;
        for (int i = 0; i < kDim; ++i) r.v[i] = v[i] + rhs.v[i];
        return r;
    }

    Vec14 operator-(const Vec14& rhs) const {
        Vec14 r;
        for (int i = 0; i < kDim; ++i) r.v[i] = v[i] - rhs.v[i];
        return r;
    }

    Vec14 operator*(float s) const {
        Vec14 r;
        for (int i = 0; i < kDim; ++i) r.v[i] = v[i] * s;
        return r;
    }

    /// Dot product
    float dot(const Vec14& rhs) const {
        float sum = 0.0f;
        for (int i = 0; i < kDim; ++i) sum += v[i] * rhs.v[i];
        return sum;
    }

    /// Euclidean magnitude
    float magnitude() const { return std::sqrt(dot(*this)); }

    /// Returns a unit-length vector (or zero if magnitude is ~0)
    Vec14 normalised() const {
        float m = magnitude();
        if (m < 1e-8f) return Vec14{};
        return *this * (1.0f / m);
    }

    /// Serialise to raw bytes (56 bytes)
    void to_bytes(void* dst) const { std::memcpy(dst, v.data(), sizeof(v)); }

    /// Deserialise from raw bytes
    static Vec14 from_bytes(const void* src) {
        Vec14 r;
        std::memcpy(r.v.data(), src, sizeof(r.v));
        return r;
    }
};

} // namespace qfs
