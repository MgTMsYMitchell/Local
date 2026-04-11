/*
 * bench_routing.cpp — QRrune WorkerAgent routing: before vs after benchmark
 *
 * Measures the impact of the kSpecializedTypes fix:
 *   BEFORE: WorkerAgent consumes ALL pending events, marks them processed.
 *           Specialized agents (Chat, Wallet, Torrent, Edge, Librarian) find
 *           empty queues on every tick — 0 % delivery.
 *   AFTER:  WorkerAgent skips specialized types; dedicated agents consume them.
 *           100 % delivery to the correct agent.
 *
 * Metrics:
 *   - Enqueue throughput (events/s)
 *   - Worker processing throughput (events/s)
 *   - Specialized-agent delivery rate (%)
 *   - Average event latency: enqueue → specialized agent pick-up (µs)
 *   - Dead events (lost to WorkerAgent in the old path)
 *   - Worker wasted iterations (processing events it should not own)
 *
 * Build (from rune-backend/):
 *   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel
 *   g++ -std=c++20 -O2 \
 *       -I build/_deps/nlohmann_json-src/include \
 *       bench/bench_routing.cpp -lsqlite3 -o bench/bench_routing
 *   ./bench/bench_routing
 */

#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <initializer_list>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include <sqlite3.h>

using Clock = std::chrono::steady_clock;
using us    = std::chrono::microseconds;

// ── helpers ───────────────────────────────────────────────────────────────────

// ── minimal in-process SQLite queue ──────────────────────────────────────────

struct Queue {
    sqlite3*   db   = nullptr;
    std::mutex mtx;

    void open(const char* path)
    {
        sqlite3_open(path, &db);
        exec("PRAGMA journal_mode=WAL;");
        exec("PRAGMA synchronous=NORMAL;");
        exec("PRAGMA cache_size=-40960;");
        exec("CREATE TABLE IF NOT EXISTS events("
             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "event_type TEXT NOT NULL,"
             "processed  INTEGER NOT NULL DEFAULT 0,"
             "enqueued_at_us INTEGER NOT NULL DEFAULT 0);");
        exec("CREATE INDEX IF NOT EXISTS idx_pending ON events(processed,event_type);");
    }

    void exec(const char* sql)
    {
        sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    }

    int64_t enqueue(const char* type, int64_t now_us)
    {
        std::lock_guard<std::mutex> lk(mtx);
        sqlite3_stmt* st = nullptr;
        sqlite3_prepare_v2(db,
            "INSERT INTO events(event_type,enqueued_at_us) VALUES(?,?);",
            -1, &st, nullptr);
        sqlite3_bind_text (st, 1, type, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(st, 2, now_us);
        sqlite3_step(st);
        int64_t rowid = sqlite3_last_insert_rowid(db);
        sqlite3_finalize(st);
        return rowid;
    }

    struct Row { int64_t id; char type[64]; };

    std::vector<Row> pending_all(int limit)
    {
        std::lock_guard<std::mutex> lk(mtx);
        sqlite3_stmt* st = nullptr;
        sqlite3_prepare_v2(db,
            "SELECT id,event_type FROM events WHERE processed=0 ORDER BY id ASC LIMIT ?;",
            -1, &st, nullptr);
        sqlite3_bind_int(st, 1, limit);
        std::vector<Row> rows;
        while (sqlite3_step(st) == SQLITE_ROW) {
            Row r;
            r.id = sqlite3_column_int64(st, 0);
            const char* t = reinterpret_cast<const char*>(sqlite3_column_text(st, 1));
            std::strncpy(r.type, t ? t : "", 63);
            rows.push_back(r);
        }
        sqlite3_finalize(st);
        return rows;
    }

    std::vector<Row> pending_type(const char* type, int limit)
    {
        std::lock_guard<std::mutex> lk(mtx);
        sqlite3_stmt* st = nullptr;
        sqlite3_prepare_v2(db,
            "SELECT id,event_type FROM events WHERE processed=0 AND event_type=? ORDER BY id ASC LIMIT ?;",
            -1, &st, nullptr);
        sqlite3_bind_text(st, 1, type, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (st, 2, limit);
        std::vector<Row> rows;
        while (sqlite3_step(st) == SQLITE_ROW) {
            Row r;
            r.id = sqlite3_column_int64(st, 0);
            const char* t = reinterpret_cast<const char*>(sqlite3_column_text(st, 1));
            std::strncpy(r.type, t ? t : "", 63);
            rows.push_back(r);
        }
        sqlite3_finalize(st);
        return rows;
    }

    int64_t enqueued_at(int64_t id)
    {
        std::lock_guard<std::mutex> lk(mtx);
        sqlite3_stmt* st = nullptr;
        sqlite3_prepare_v2(db,
            "SELECT enqueued_at_us FROM events WHERE id=?;",
            -1, &st, nullptr);
        sqlite3_bind_int64(st, 1, id);
        int64_t v = 0;
        if (sqlite3_step(st) == SQLITE_ROW) v = sqlite3_column_int64(st, 0);
        sqlite3_finalize(st);
        return v;
    }

    void mark_done(int64_t id)
    {
        std::lock_guard<std::mutex> lk(mtx);
        sqlite3_stmt* st = nullptr;
        sqlite3_prepare_v2(db,
            "UPDATE events SET processed=1 WHERE id=?;",
            -1, &st, nullptr);
        sqlite3_bind_int64(st, 1, id);
        sqlite3_step(st);
        sqlite3_finalize(st);
    }

    void close()  { if (db) { sqlite3_close(db); db = nullptr; } }
    ~Queue()      { close(); }
};

// ── scenario constants ────────────────────────────────────────────────────────

static constexpr int N_EVENTS = 2000;

static constexpr std::array<const char*,9> EVENT_TYPES = {
    "heartbeat", "strategy_execute", "rune_trust",    // worker-owned  (3 of 9 = 33%)
    "chat_request", "wallet_sign", "wallet_verify",   // specialized   (6 of 9 = 67%)
    "torrent_piece", "edge_task", "classify_symbol"
};

static constexpr std::initializer_list<std::string_view> kSpecializedTypes = {
    "chat_request", "wallet_sign", "wallet_verify",
    "torrent_piece", "edge_task", "classify_symbol"
};

static bool is_specialized(const char* t)
{
    for (auto sv : kSpecializedTypes)
        if (sv == t) return true;
    return false;
}

// ── result struct ─────────────────────────────────────────────────────────────

struct Result {
    const char* label;
    double enqueue_rate_kps    = 0;
    double worker_rate_kps     = 0;
    double specialist_rate_kps = 0;
    double delivery_pct        = 0;
    double lost_pct            = 0;
    double avg_latency_us      = 0;
    int64_t wasted_worker_ops  = 0;
    int  specialist_seen       = 0;
    int  spec_total            = 0;
};

// ── BEFORE scenario ───────────────────────────────────────────────────────────

Result run_before()
{
    Result r;
    r.label = "BEFORE (broken routing)";

    Queue q;
    q.open("/tmp/bench_before.db");
    q.exec("DELETE FROM events;");

    int spec_total = 0;

    auto t0 = Clock::now();
    for (int i = 0; i < N_EVENTS; ++i) {
        const char* type = EVENT_TYPES[i % EVENT_TYPES.size()];
        int64_t now_us = std::chrono::duration_cast<us>(
            Clock::now().time_since_epoch()).count();
        q.enqueue(type, now_us);
        if (is_specialized(type)) ++spec_total;
    }
    auto t1 = Clock::now();
    double enq_ms = std::chrono::duration<double,std::milli>(t1 - t0).count();
    r.enqueue_rate_kps = N_EVENTS / enq_ms;
    r.spec_total = spec_total;

    // WorkerAgent (old): consumes ALL events — including specialized ones.
    int64_t worker_done = 0, wasted = 0;
    auto tw0 = Clock::now();
    while (true) {
        auto rows = q.pending_all(20);
        if (rows.empty()) break;
        for (auto& row : rows) {
            if (is_specialized(row.type)) ++wasted;
            q.mark_done(row.id);
            ++worker_done;
        }
    }
    auto tw1 = Clock::now();
    double wk_ms = std::chrono::duration<double,std::milli>(tw1 - tw0).count() + 0.001;
    r.worker_rate_kps   = worker_done / wk_ms;
    r.wasted_worker_ops = wasted;

    // Specialized agents try to pick up — find nothing.
    int spec_seen = 0;
    double total_lat = 0.0;
    auto ts0 = Clock::now();
    for (auto sv : kSpecializedTypes) {
        auto rows = q.pending_type(std::string(sv).c_str(), N_EVENTS);
        for (auto& row : rows) {
            int64_t enq = q.enqueued_at(row.id);
            int64_t now = std::chrono::duration_cast<us>(
                Clock::now().time_since_epoch()).count();
            total_lat += (double)(now - enq);
            q.mark_done(row.id);
            ++spec_seen;
        }
    }
    auto ts1 = Clock::now();
    double sp_ms = std::chrono::duration<double,std::milli>(ts1 - ts0).count() + 0.001;

    r.specialist_seen     = spec_seen;
    r.specialist_rate_kps = spec_seen / sp_ms;
    r.delivery_pct = spec_total > 0 ? 100.0 * spec_seen / spec_total : 0.0;
    r.lost_pct     = 100.0 - r.delivery_pct;
    r.avg_latency_us = spec_seen > 0 ? total_lat / spec_seen : -1.0;

    q.close();
    return r;
}

// ── AFTER scenario ────────────────────────────────────────────────────────────

Result run_after()
{
    Result r;
    r.label = "AFTER  (correct routing)";

    Queue q;
    q.open("/tmp/bench_after.db");
    q.exec("DELETE FROM events;");

    int spec_total = 0;

    auto t0 = Clock::now();
    for (int i = 0; i < N_EVENTS; ++i) {
        const char* type = EVENT_TYPES[i % EVENT_TYPES.size()];
        int64_t now_us = std::chrono::duration_cast<us>(
            Clock::now().time_since_epoch()).count();
        q.enqueue(type, now_us);
        if (is_specialized(type)) ++spec_total;
    }
    auto t1 = Clock::now();
    double enq_ms = std::chrono::duration<double,std::milli>(t1 - t0).count();
    r.enqueue_rate_kps = N_EVENTS / enq_ms;
    r.spec_total = spec_total;

    // WorkerAgent (new): skips specialized types.
    int64_t worker_done = 0;
    auto tw0 = Clock::now();
    {
        bool any;
        do {
            any = false;
            auto rows = q.pending_all(20);
            if (rows.empty()) break;
            for (auto& row : rows) {
                if (is_specialized(row.type)) continue;   // ← THE FIX
                q.mark_done(row.id);
                ++worker_done;
                any = true;
            }
        } while (any);
    }
    auto tw1 = Clock::now();
    double wk_ms = std::chrono::duration<double,std::milli>(tw1 - tw0).count() + 0.001;
    r.worker_rate_kps   = worker_done / wk_ms;
    r.wasted_worker_ops = 0;

    // Specialized agents pick up their events.
    int spec_seen = 0;
    double total_lat = 0.0;
    auto ts0 = Clock::now();
    for (auto sv : kSpecializedTypes) {
        auto rows = q.pending_type(std::string(sv).c_str(), N_EVENTS);
        for (auto& row : rows) {
            int64_t enq = q.enqueued_at(row.id);
            int64_t now = std::chrono::duration_cast<us>(
                Clock::now().time_since_epoch()).count();
            total_lat += (double)(now - enq);
            q.mark_done(row.id);
            ++spec_seen;
        }
    }
    auto ts1 = Clock::now();
    double sp_ms = std::chrono::duration<double,std::milli>(ts1 - ts0).count() + 0.001;

    r.specialist_seen     = spec_seen;
    r.specialist_rate_kps = spec_seen / sp_ms;
    r.delivery_pct = spec_total > 0 ? 100.0 * spec_seen / spec_total : 0.0;
    r.lost_pct     = 100.0 - r.delivery_pct;
    r.avg_latency_us = spec_seen > 0 ? total_lat / spec_seen : -1.0;

    q.close();
    return r;
}

// ── print table ───────────────────────────────────────────────────────────────

static void print_table(const Result& B, const Result& A)
{
    const int W0=44, W1=16, W2=16, W3=18;
    auto line = [&]() {
        printf("+%s+%s+%s+%s+\n",
            std::string(W0,'-').c_str(),
            std::string(W1,'-').c_str(),
            std::string(W2,'-').c_str(),
            std::string(W3,'-').c_str());
    };
    auto row  = [&](const char* m, const char* b, const char* a, const char* d) {
        printf("| %-*s| %*s | %*s | %*s |\n",
            W0-1, m, W1-2, b, W2-2, a, W3-2, d);
    };

    printf("\n");
    printf("  QRrune Cognitive Node — WorkerAgent Routing Fix Benchmark\n");
    printf("  Workload: %d events  (67%% specialized, 33%% worker-owned)\n\n", N_EVENTS);

    line();
    row("Metric", "BEFORE", "AFTER", "Delta / Change");
    line();

    char b[32], a[32], d[32];

    // Enqueue throughput
    snprintf(b,32,"%.1f k/s",B.enqueue_rate_kps);
    snprintf(a,32,"%.1f k/s",A.enqueue_rate_kps);
    snprintf(d,32,"%+.0f%%",100.0*(A.enqueue_rate_kps-B.enqueue_rate_kps)/B.enqueue_rate_kps);
    row("Enqueue throughput", b,a,d);

    // Worker throughput
    snprintf(b,32,"%.1f k/s",B.worker_rate_kps);
    snprintf(a,32,"%.1f k/s",A.worker_rate_kps);
    snprintf(d,32,"%+.0f%%",100.0*(A.worker_rate_kps-B.worker_rate_kps)/B.worker_rate_kps);
    row("Worker processing throughput", b,a,d);

    // Specialist throughput
    snprintf(b,32,"%.2f k/s",B.specialist_rate_kps);
    snprintf(a,32,"%.1f k/s",A.specialist_rate_kps);
    if (B.specialist_rate_kps < 0.001)
        snprintf(d,32,"inf (was 0)");
    else
        snprintf(d,32,"%+.0f%%",100.0*(A.specialist_rate_kps-B.specialist_rate_kps)/B.specialist_rate_kps);
    row("Specialist-agent throughput", b,a,d);

    // Delivery rate
    snprintf(b,32,"%.0f %%",B.delivery_pct);
    snprintf(a,32,"%.0f %%",A.delivery_pct);
    snprintf(d,32,"%+.0f pp",A.delivery_pct-B.delivery_pct);
    row("Specialized-event delivery rate", b,a,d);

    // Loss rate
    snprintf(b,32,"%.0f %%",B.lost_pct);
    snprintf(a,32,"%.0f %%",A.lost_pct);
    snprintf(d,32,"%+.0f pp",A.lost_pct-B.lost_pct);
    row("Event loss rate (silently killed)", b,a,d);

    // Avg latency
    if (B.avg_latency_us < 0) snprintf(b,32,"N/A (0 events)");
    else snprintf(b,32,"%.0f us",B.avg_latency_us);
    snprintf(a,32,"%.0f us",A.avg_latency_us);
    if (B.avg_latency_us < 0) snprintf(d,32,"N/A");
    else snprintf(d,32,"%.0f us",A.avg_latency_us-B.avg_latency_us);
    row("Avg. specialized-event latency", b,a,d);

    // Wasted worker ops
    snprintf(b,32,"%lld",(long long)B.wasted_worker_ops);
    snprintf(a,32,"%lld",(long long)A.wasted_worker_ops);
    snprintf(d,32,"%lld",(long long)(A.wasted_worker_ops-B.wasted_worker_ops));
    row("Worker ops wasted on specialist evts", b,a,d);

    // Specialist events delivered
    snprintf(b,32,"%d / %d",B.specialist_seen,B.spec_total);
    snprintf(a,32,"%d / %d",A.specialist_seen,A.spec_total);
    snprintf(d,32,"+%d",A.specialist_seen-B.specialist_seen);
    row("Specialist events seen by correct agent", b,a,d);

    line();

    printf("\n  Legend: pp=percentage points  k/s=thousand events/s  us=microseconds\n\n");
    printf("  Summary:\n");
    printf("  BEFORE: WorkerAgent consumed %lld specialized events it should not have,\n",
        (long long)B.wasted_worker_ops);
    printf("          leaving %d / %d events undelivered (%.0f%% loss).\n",
        B.spec_total - B.specialist_seen, B.spec_total, B.lost_pct);
    printf("          ChatAgent, WalletAgent, TorrentAgent, EdgeAgent,\n");
    printf("          LibrarianAgent received zero events — completely non-functional.\n\n");
    printf("  AFTER:  WorkerAgent skips all 6 specialized types (kSpecializedTypes).\n");
    printf("          Dedicated agents receive 100%% of their events.\n");
    printf("          Worker throughput drop reflects smaller owned workload (33%% of queue)\n");
    printf("          which is correct — Worker now only does what it owns.\n\n");
}

int main()
{
    std::remove("/tmp/bench_before.db");
    std::remove("/tmp/bench_after.db");

    printf("Running BEFORE scenario (%d events)...\n", N_EVENTS);
    Result bef = run_before();

    printf("Running AFTER scenario  (%d events)...\n\n", N_EVENTS);
    Result aft = run_after();

    print_table(bef, aft);
    return 0;
}
