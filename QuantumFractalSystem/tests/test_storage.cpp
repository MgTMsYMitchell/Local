// QuantumFractalSystem — Storage layer tests

#include "storage/Database.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <string>

using namespace qfs;

class StorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_path_ = "/tmp/qfs_test_" + std::to_string(reinterpret_cast<uintptr_t>(this)) + ".db";
        db_ = std::make_unique<Database>(db_path_);
    }

    void TearDown() override {
        db_.reset();
        std::remove(db_path_.c_str());
        std::remove((db_path_ + "-wal").c_str());
        std::remove((db_path_ + "-shm").c_str());
    }

    std::string db_path_;
    std::unique_ptr<Database> db_;
};

TEST_F(StorageTest, CreateTable) {
    db_->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, val TEXT);");
    db_->execute("INSERT INTO test (val) VALUES ('hello');");
    EXPECT_EQ(db_->count("test"), 1);
}

TEST_F(StorageTest, Query) {
    db_->execute("CREATE TABLE items (id INTEGER PRIMARY KEY, name TEXT);");
    db_->execute("INSERT INTO items (name) VALUES ('alpha');");
    db_->execute("INSERT INTO items (name) VALUES ('beta');");

    auto rows = db_->query("SELECT name FROM items ORDER BY name;");
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0].values[0], "alpha");
    EXPECT_EQ(rows[1].values[0], "beta");
}

TEST_F(StorageTest, QueryJSON) {
    db_->execute("CREATE TABLE kv (k TEXT PRIMARY KEY, v TEXT);");
    db_->execute("INSERT INTO kv VALUES ('key1', 'val1');");

    auto json = db_->query_json("SELECT * FROM kv;");
    ASSERT_EQ(json.size(), 1u);
    EXPECT_EQ(json[0]["k"], "key1");
    EXPECT_EQ(json[0]["v"], "val1");
}

TEST_F(StorageTest, CountEmpty) {
    db_->execute("CREATE TABLE empty_tbl (id INTEGER PRIMARY KEY);");
    EXPECT_EQ(db_->count("empty_tbl"), 0);
}

TEST_F(StorageTest, ApplySchema) {
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS nodes (
            id TEXT PRIMARY KEY,
            status TEXT DEFAULT 'active'
        );
    )";
    db_->apply_schema(schema);

    // Applying twice should be idempotent
    db_->apply_schema(schema);

    db_->execute("INSERT INTO nodes (id) VALUES ('n1');");
    EXPECT_EQ(db_->count("nodes"), 1);
}

TEST_F(StorageTest, DatabaseOps) {
    // Full lifecycle: create, insert, query, count
    db_->execute(R"(
        CREATE TABLE tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            node_id TEXT,
            status TEXT DEFAULT 'pending'
        );
    )");

    db_->execute("INSERT INTO tasks (node_id, status) VALUES ('n1', 'pending');");
    db_->execute("INSERT INTO tasks (node_id, status) VALUES ('n2', 'running');");

    EXPECT_EQ(db_->count("tasks"), 2);

    auto rows = db_->query("SELECT * FROM tasks WHERE status='pending';");
    EXPECT_EQ(rows.size(), 1u);
}
