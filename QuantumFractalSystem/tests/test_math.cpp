// QuantumFractalSystem — Math module tests

#include "math/AlignmentField.h"
#include "math/Tensor14.h"
#include "math/Vector14.h"

#include <gtest/gtest.h>
#include <cmath>

using namespace qfs;

TEST(Math, Vec14Dot) {
    Vec14 a(1.0f);
    Vec14 b(2.0f);
    EXPECT_NEAR(a.dot(b), 28.0f, 0.001f);
}

TEST(Math, Vec14Magnitude) {
    Vec14 v(0.0f);
    v[0] = 3.0f;
    v[1] = 4.0f;
    EXPECT_NEAR(v.magnitude(), 5.0f, 0.001f);
}

TEST(Math, Vec14Normalised) {
    Vec14 v(0.0f);
    v[0] = 3.0f;
    v[1] = 4.0f;
    auto n = v.normalised();
    EXPECT_NEAR(n.magnitude(), 1.0f, 0.001f);
}

TEST(Math, Vec14Arithmetic) {
    Vec14 a(1.0f);
    Vec14 b(2.0f);
    auto sum = a + b;
    auto diff = b - a;
    auto scaled = a * 3.0f;
    EXPECT_NEAR(sum[0], 3.0f, 0.001f);
    EXPECT_NEAR(diff[0], 1.0f, 0.001f);
    EXPECT_NEAR(scaled[0], 3.0f, 0.001f);
}

TEST(Math, Tensor14Identity) {
    auto I = Tensor14::identity();
    EXPECT_NEAR(I(0, 0), 1.0f, 0.001f);
    EXPECT_NEAR(I(0, 1), 0.0f, 0.001f);
    EXPECT_NEAR(I.trace(), 14.0f, 0.001f);
}

TEST(Math, Tensor14Apply) {
    auto I = Tensor14::identity();
    Vec14 v(1.0f);
    auto result = I.apply(v);
    for (int i = 0; i < Vec14::kDim; ++i)
        EXPECT_NEAR(result[i], v[i], 0.001f);
}

TEST(Math, Tensor14OuterProduct) {
    Vec14 a(0.0f);
    a[0] = 1.0f;
    auto T = Tensor14::outer(a, a);
    EXPECT_NEAR(T(0, 0), 1.0f, 0.001f);
    EXPECT_NEAR(T(1, 0), 0.0f, 0.001f);
}

TEST(Math, AlignmentFieldCompute) {
    Vec14 align(0.0f);
    align[0] = 1.0f;
    auto field = AlignmentField::compute_field(align);
    EXPECT_GT(field.frobenius_norm(), 0.0f);
}

TEST(Math, DriftCalculation) {
    Vec14 align(0.0f);
    align[0] = 1.0f;
    auto field = AlignmentField::compute_field(align);
    auto drift = AlignmentField::compute_drift(field, align);
    // For a well-aligned node, drift should be small
    EXPECT_LT(drift.magnitude(), 1.0f);
}

TEST(Math, HealingCorrection) {
    Vec14 align(0.0f);
    align[0] = 1.0f;
    auto field = AlignmentField::compute_field(align);
    auto healing = AlignmentField::compute_healing(field, align, 0.1f);
    // Healing should be opposite direction from drift
    EXPECT_LT(healing.magnitude(), 0.5f);
}

TEST(Math, Serialisation) {
    Vec14 v(0.0f);
    v[0] = 42.0f;
    v[13] = -7.5f;

    uint8_t buf[56];
    v.to_bytes(buf);
    auto v2 = Vec14::from_bytes(buf);
    EXPECT_NEAR(v2[0], 42.0f, 0.001f);
    EXPECT_NEAR(v2[13], -7.5f, 0.001f);
}
