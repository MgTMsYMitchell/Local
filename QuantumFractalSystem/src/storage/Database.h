#pragma once
// QuantumFractalSystem — SQLite storage layer

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include <sqlite3.h>
#include <nlohmann/json.hpp>

namespace qfs {

/// A single result row from a query.
struct Row {
    std::vector<std::string> columns;
    std::vector<std::string> values;
};

/// Thin RAII wrapper around an SQLite3 database with WAL mode,
/// thread-safe execution, and schema migration support.
class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /// Execute a non-query statement (INSERT, UPDATE, DELETE, DDL).
    void execute(const std::string& sql);

    /// Execute a query and return all result rows.
    std::vector<Row> query(const std::string& sql);

    /// Execute a query and return the result as JSON array of objects.
    nlohmann::json query_json(const std::string& sql);

    /// Return the number of rows in a table.
    int count(const std::string& table);

    /// Apply the schema file (idempotent — uses IF NOT EXISTS).
    void apply_schema(const std::string& schema_sql);

    /// Raw handle (for advanced use only).
    sqlite3* raw() { return db_; }

private:
    sqlite3*   db_ = nullptr;
    std::mutex mtx_;
};

} // namespace qfs
