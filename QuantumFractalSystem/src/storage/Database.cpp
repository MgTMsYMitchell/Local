// QuantumFractalSystem — SQLite storage layer implementation

#include "storage/Database.h"

#include <cstring>
#include <stdexcept>

namespace qfs {

// ── helpers ──────────────────────────────────────────────────────────────────

static void throw_if(int rc, sqlite3* db, const char* context) {
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::string msg = std::string(context) + ": " + sqlite3_errmsg(db);
        throw std::runtime_error(msg);
    }
}

// ── Database ─────────────────────────────────────────────────────────────────

Database::Database(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);
    throw_if(rc, db_, "sqlite3_open");

    // Enable WAL mode and performance pragmas
    execute("PRAGMA journal_mode = WAL;");
    execute("PRAGMA synchronous  = NORMAL;");
    execute("PRAGMA cache_size   = -40000;");
    execute("PRAGMA temp_store   = MEMORY;");
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lk(mtx_);
    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error("execute: " + msg);
    }
}

std::vector<Row> Database::query(const std::string& sql) {
    std::lock_guard<std::mutex> lk(mtx_);
    std::vector<Row> rows;

    auto callback = [](void* data, int argc, char** argv, char** col_names) -> int {
        auto* result = static_cast<std::vector<Row>*>(data);
        Row row;
        for (int i = 0; i < argc; ++i) {
            row.columns.emplace_back(col_names[i] ? col_names[i] : "");
            row.values.emplace_back(argv[i] ? argv[i] : "");
        }
        result->push_back(std::move(row));
        return 0;
    };

    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), callback, &rows, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error("query: " + msg);
    }

    return rows;
}

nlohmann::json Database::query_json(const std::string& sql) {
    auto rows = query(sql);
    nlohmann::json arr = nlohmann::json::array();
    for (auto& row : rows) {
        nlohmann::json obj;
        for (std::size_t i = 0; i < row.columns.size(); ++i)
            obj[row.columns[i]] = row.values[i];
        arr.push_back(std::move(obj));
    }
    return arr;
}

int Database::count(const std::string& table) {
    // Use a prepared statement to safely inject the table name.
    // Since SQLite doesn't support binding table names, we validate the name.
    for (char c : table) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
            throw std::runtime_error("invalid table name");
    }
    auto rows = query("SELECT COUNT(*) FROM " + table + ";");
    if (rows.empty() || rows[0].values.empty()) return 0;
    return std::stoi(rows[0].values[0]);
}

void Database::apply_schema(const std::string& schema_sql) {
    execute(schema_sql);
}

} // namespace qfs
