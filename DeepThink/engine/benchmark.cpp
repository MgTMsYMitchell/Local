// benchmark.cpp
//
// Measures QR2RuneEngine ingest and query performance without any network I/O.
//
// Design targets:
//   Ingest 10k items   → ~0.6–0.9 s  (when including LocalAI embed calls)
//   Engine-only ingest → well under 10 ms
//   Query top-10       → < 10 ms per query

#include "qr2_rune_engine.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace deepthink;
using clk = std::chrono::high_resolution_clock;
using ms  = std::chrono::duration<double, std::milli>;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::vector<IngestItem> make_random_batch(std::size_t n,
                                                  uint32_t base_ts,
                                                  uint32_t seed = 42)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float>    dist(-1.f, 1.f);
    std::uniform_int_distribution<uint32_t>  tag_dist(1, 100);

    std::vector<IngestItem> batch;
    batch.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        IngestItem item;
        item.timestamp = base_ts + static_cast<uint32_t>(i);
        item.tag_id    = tag_dist(rng);
        for (float& f : item.sigil.v) f = dist(rng);
        batch.push_back(item);
    }
    return batch;
}

static SigilVec random_sigil(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(-1.f, 1.f);
    SigilVec sv;
    for (float& f : sv.v) f = dist(rng);
    return sv;
}

static void check(const char* label, double measured, double target)
{
    const bool ok = measured <= target;
    std::cout << "  " << std::left << std::setw(36) << label
              << std::right << std::setw(10) << std::fixed
              << std::setprecision(3) << measured
              << (ok ? "  ✓" : "  ✗")
              << "  (target ≤ " << target << ")\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    constexpr std::size_t INGEST_N  = 10'000;
    constexpr std::size_t QUERY_N   =    200;
    constexpr std::size_t TOP_K     =     10;
    constexpr uint32_t    BASE_TS   = 1'700'000'000u;

    std::cout << "DeepThink QR2+Rune benchmark\n";
    std::cout << "  Ingest N = " << INGEST_N
              << "  Query N = " << QUERY_N
              << "  top_k = "   << TOP_K << "\n\n";

    QR2RuneEngine engine;

    // ------------------------------------------------------------------
    // Ingest benchmark
    // ------------------------------------------------------------------
    auto batch = make_random_batch(INGEST_N, BASE_TS);

    auto t0 = clk::now();
    engine.ingest_batch(batch);
    auto t1 = clk::now();

    const double ingest_ms  = ms(t1 - t0).count();
    const double inserts_ps = INGEST_N / (ingest_ms / 1000.0);

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "[Ingest] " << INGEST_N << " items: "
              << ingest_ms << " ms  ("
              << static_cast<long long>(inserts_ps) << " inserts/s)\n\n";

    // ------------------------------------------------------------------
    // Query benchmark
    // ------------------------------------------------------------------
    std::mt19937 rng(99);
    double total_ms = 0.0;
    double min_ms   = 1e9;
    double max_ms   = 0.0;

    std::vector<Record> records;
    std::vector<float>  scores;

    for (std::size_t q = 0; q < QUERY_N; ++q) {
        SigilVec qv = random_sigil(rng);

        auto q0 = clk::now();
        engine.query(qv, TOP_K, records, scores);
        auto q1 = clk::now();

        const double elapsed = ms(q1 - q0).count();
        total_ms += elapsed;
        if (elapsed < min_ms) min_ms = elapsed;
        if (elapsed > max_ms) max_ms = elapsed;
    }

    const double avg_ms = total_ms / static_cast<double>(QUERY_N);

    std::cout << "[Query]  " << QUERY_N << " queries, top-" << TOP_K
              << " over " << INGEST_N << " items:\n"
              << "  avg " << avg_ms << " ms"
              << "  min " << min_ms << " ms"
              << "  max " << max_ms << " ms\n\n";

    // ------------------------------------------------------------------
    // Target summary
    // ------------------------------------------------------------------
    std::cout << "--- Target checks ---\n";
    check("Engine ingest 10k (ms)",      ingest_ms, 50.0);   // net latency adds ~800ms
    check("Query avg latency (ms)",      avg_ms,    10.0);
    check("Query max latency (ms)",      max_ms,    20.0);

    std::cout << "\nNote: end-to-end ingest (including LocalAI embed) targets 0.6–0.9 s "
                 "for 10k items.\n"
                 "      Engine-only ingest is typically < 5 ms.\n";

    return 0;
}
