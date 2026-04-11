// QuantumFractalSystem — Engine tests (self-healing + compute)

#include "compression/QuantumInspiredCompressor.h"
#include "engine/ComputeEngine.h"
#include "engine/SelfHealingEngine.h"
#include "math/AlignmentField.h"
#include "network/MeshNode.h"

#include <gtest/gtest.h>
#include <cmath>
#include <memory>

using namespace qfs;

TEST(Engine, DetectCorruptionClean) {
    SelfHealingEngine healer(0.3f, 0.05f);
    Vec14 align(0.0f);
    align[0] = 1.0f;
    auto field = AlignmentField::compute_field(align);
    EXPECT_FALSE(healer.detect_corruption(field));
}

TEST(Engine, DetectCorruptionBroken) {
    SelfHealingEngine healer(0.001f, 0.05f); // very tight threshold
    Tensor14 broken;
    for (int i = 0; i < Vec14::kDim; ++i)
        for (int j = 0; j < Vec14::kDim; ++j)
            broken(i, j) = 100.0f; // grossly wrong
    EXPECT_TRUE(healer.detect_corruption(broken));
}

TEST(Engine, RecursiveRepair) {
    SelfHealingEngine healer(0.3f, 0.05f);
    QuantumInspiredCompressor compressor(16);

    MeshNode node("test-node");
    Vec14 align(0.0f);
    align[0] = 1.0f;
    node.set_alignment(align);

    healer.recursive_repair(node, compressor);
    EXPECT_TRUE(node.is_healthy());
}

TEST(Engine, RerouteAroundFailure) {
    SelfHealingEngine healer;
    std::vector<std::string> all = {"a", "b", "c", "d"};
    auto alt = healer.reroute_around_failure("b", all);
    EXPECT_EQ(alt.size(), 3u);
    for (auto& id : alt)
        EXPECT_NE(id, "b");
}

TEST(Engine, Sweep) {
    SelfHealingEngine healer(0.3f, 0.05f);
    QuantumInspiredCompressor compressor(16);

    auto n1 = std::make_unique<MeshNode>("n1");
    auto n2 = std::make_unique<MeshNode>("n2");

    Vec14 a1(0.0f); a1[0] = 1.0f;
    Vec14 a2(0.0f); a2[1] = 1.0f;
    n1->set_alignment(a1);
    n2->set_alignment(a2);

    std::vector<MeshNode*> nodes = {n1.get(), n2.get()};
    auto repaired = healer.sweep(nodes, compressor);

    // With normal alignments, nothing should need repair
    for (auto* node : nodes)
        EXPECT_TRUE(node->is_healthy());
}

TEST(Engine, ComputeCPU) {
    ComputeEngine engine;
    std::vector<float> input = {0.0f, 1.0f, -1.0f, 0.5f};
    auto output = engine.compute_cpu(input);

    ASSERT_EQ(output.size(), input.size());
    EXPECT_NEAR(output[0], std::tanh(0.0f), 0.001f);
    EXPECT_NEAR(output[1], std::tanh(1.0f), 0.001f);
}

TEST(Engine, ParallelCompute) {
    ComputeEngine engine;
    std::vector<std::vector<float>> inputs = {
        {1.0f, 2.0f},
        {3.0f, 4.0f},
        {5.0f, 6.0f}
    };

    auto results = engine.parallel_compute(inputs);
    ASSERT_EQ(results.size(), 3u);
    for (auto& r : results)
        EXPECT_EQ(r.size(), 2u);
}
