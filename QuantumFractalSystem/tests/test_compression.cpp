// QuantumFractalSystem — Compression module tests

#include "compression/QuantumInspiredCompressor.h"

#include <gtest/gtest.h>
#include <cmath>
#include <vector>

using namespace qfs;

TEST(Compression, EncodeEmpty) {
    QuantumInspiredCompressor c;
    auto state = c.encode({});
    EXPECT_EQ(state.rank(), 0u);
}

TEST(Compression, EncodeDecode) {
    QuantumInspiredCompressor c(16);
    std::vector<float> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    auto state = c.encode(input);
    EXPECT_GT(state.rank(), 0u);

    auto output = c.decode(state, input.size());
    EXPECT_EQ(output.size(), input.size());
}

TEST(Compression, EncodeDecodePreservesEnergy) {
    QuantumInspiredCompressor c(8);
    std::vector<float> input = {1.0f, 0.5f, 0.25f, 0.1f, 3.0f, 2.0f, 1.5f, 0.8f};

    auto state = c.encode(input);
    EXPECT_NEAR(state.total_probability(), 1.0f, 0.01f);
}

TEST(Compression, Entangle) {
    QuantumInspiredCompressor c(4);
    std::vector<float> a = {1, 2, 3, 4};
    std::vector<float> b = {5, 6, 7, 8};

    auto sa = c.encode(a);
    auto sb = c.encode(b);
    auto entangled = QuantumInspiredCompressor::entangle(sa, sb);

    EXPECT_GE(entangled.rank(), sa.rank());
    EXPECT_NEAR(entangled.total_probability(), 1.0f, 0.01f);
}

TEST(Compression, Collapse) {
    QuantumInspiredCompressor c(8);
    std::vector<float> input = {1, 2, 3, 4, 5, 6, 7, 8};

    auto state = c.encode(input);
    auto collapsed = QuantumInspiredCompressor::collapse(state);

    EXPECT_EQ(collapsed.rank(), 1u);
    EXPECT_NEAR(collapsed.total_probability(), 1.0f, 0.001f);
}
