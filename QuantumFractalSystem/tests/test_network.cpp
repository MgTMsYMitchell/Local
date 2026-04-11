// QuantumFractalSystem — Network mesh tests

#include "network/FractalMesh.h"
#include "network/MeshNode.h"
#include "network/Routing.h"

#include <gtest/gtest.h>
#include <memory>

using namespace qfs;

TEST(Network, RoutingDirect) {
    RoutingTable rt;
    rt.add_route("A", "B");

    auto path = rt.find_path("A", "B");
    ASSERT_EQ(path.size(), 2u);
    EXPECT_EQ(path[0], "A");
    EXPECT_EQ(path[1], "B");
}

TEST(Network, RoutingMultiHop) {
    RoutingTable rt;
    rt.add_route("A", "B");
    rt.add_route("B", "C");
    rt.add_route("C", "D");

    auto path = rt.find_path("A", "D");
    ASSERT_EQ(path.size(), 4u);
    EXPECT_EQ(path.front(), "A");
    EXPECT_EQ(path.back(), "D");
}

TEST(Network, RoutingUnreachable) {
    RoutingTable rt;
    rt.add_route("A", "B");

    auto path = rt.find_path("A", "Z");
    EXPECT_TRUE(path.empty());
}

TEST(Network, RoutingSelfPath) {
    RoutingTable rt;
    auto path = rt.find_path("A", "A");
    ASSERT_EQ(path.size(), 1u);
    EXPECT_EQ(path[0], "A");
}

TEST(Network, MeshAddRemove) {
    FractalMesh mesh;
    mesh.add_node(std::make_unique<MeshNode>("n1"));
    mesh.add_node(std::make_unique<MeshNode>("n2"));
    mesh.add_node(std::make_unique<MeshNode>("n3"));

    EXPECT_EQ(mesh.size(), 3u);

    mesh.remove_node("n2");
    EXPECT_EQ(mesh.size(), 2u);
    EXPECT_EQ(mesh.find_node("n2"), nullptr);
}

TEST(Network, MeshRouting) {
    FractalMesh mesh;
    mesh.add_node(std::make_unique<MeshNode>("n1"));
    mesh.add_node(std::make_unique<MeshNode>("n2"));
    mesh.add_node(std::make_unique<MeshNode>("n3"));

    auto path = mesh.route("n1", "n3");
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path.front(), "n1");
    EXPECT_EQ(path.back(), "n3");
}

TEST(Network, MeshMessageDelivery) {
    FractalMesh mesh;
    mesh.add_node(std::make_unique<MeshNode>("sender"));
    mesh.add_node(std::make_unique<MeshNode>("receiver"));

    bool received = false;
    std::vector<float> received_data;

    mesh.find_node("receiver")->on_receive(
        [&](const std::string&, const std::vector<float>& data) {
            received = true;
            received_data = data;
        });

    mesh.find_node("sender")->send_to("receiver", {1.0f, 2.0f, 3.0f});
    mesh.flush_messages();

    EXPECT_TRUE(received);
    ASSERT_EQ(received_data.size(), 3u);
    EXPECT_NEAR(received_data[0], 1.0f, 0.001f);
}
