#pragma once
#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>

#include "db/database.hpp"

// ── RuneNode ──────────────────────────────────────────────────────────────────
// A background worker that runs a heartbeat loop until `shutdown` is set.
// Multiple RuneNode instances can share a single Database (it is thread-safe).

class RuneNode {
public:
    RuneNode(int id, std::atomic<bool>& shutdown, Database& db)
        : id_(id), shutdown_(shutdown), db_(db)
    {}

    // Movable so it can be captured in a lambda for std::thread.
    RuneNode(RuneNode&&)            = default;
    RuneNode& operator=(RuneNode&&) = default;

    RuneNode(const RuneNode&)            = delete;
    RuneNode& operator=(const RuneNode&) = delete;

    int id() const { return id_; }

    void run()
    {
        log("[node %d] started\n", id_);
        while (!shutdown_.load(std::memory_order_relaxed)) {
            for (int i = 0; i < 50 && !shutdown_.load(std::memory_order_relaxed); ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (shutdown_.load(std::memory_order_relaxed)) break;

            // Single SQL round-trip for all counts instead of three separate
            // COUNT(*) queries (each of which would acquire the mutex in turn).
            auto hc = db_.health_counts();
            log("[node %d] heartbeat — runes=%d  stories=%d  mythic=%d\n",
                id_, hc.runes, hc.stories, hc.mythic_moments);
        }
        log("[node %d] stopped\n", id_);
    }

private:
    int                id_;
    std::atomic<bool>& shutdown_;
    Database&          db_;

    static std::mutex& log_mutex()
    {
        static std::mutex m;
        return m;
    }

    template <typename... Args>
    static void log(const char* fmt, Args&&... args)
    {
        std::lock_guard<std::mutex> lk(log_mutex());
        std::printf(fmt, std::forward<Args>(args)...);
        std::fflush(stdout);
    }
};
