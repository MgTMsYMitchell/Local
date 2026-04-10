#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <shared_mutex>

namespace deepthink {

struct Record {
    uint64_t id;
    uint32_t ts_delta;   // delta from base_ts (compression)
    uint32_t tag_id;
    uint32_t glyph_id;
};

struct SigilVec {
    // 14D vector
    std::array<float, 14> v;
};

class QR2Store {
public:
    QR2Store();

    std::vector<uint64_t> append_batch(
        const std::vector<uint32_t>& timestamps,
        const std::vector<uint32_t>& tag_ids,
        const std::vector<uint32_t>& glyph_ids);

    void fetch_by_ids(const std::vector<uint64_t>& ids,
                      std::vector<Record>& out) const;

    uint64_t size() const;

private:
    mutable std::shared_mutex mtx_;

    uint64_t next_id_;
    uint32_t base_ts_;

    std::vector<uint64_t> ids_;
    std::vector<uint32_t> ts_delta_;
    std::vector<uint32_t> tag_ids_;
    std::vector<uint32_t> glyph_ids_;
};

class RuneIndex {
public:
    RuneIndex();

    void insert_batch(const std::vector<uint64_t>& ids,
                      const std::vector<SigilVec>& sigils);

    void search(const SigilVec& query,
                std::size_t top_k,
                std::vector<uint64_t>& out_ids,
                std::vector<float>& out_scores) const;

    uint64_t size() const;

private:
    mutable std::shared_mutex mtx_;

    std::vector<uint64_t> ids_;
    std::vector<SigilVec> sigils_;

    static float l2(const SigilVec& a, const SigilVec& b);
};

struct IngestItem {
    uint32_t timestamp;
    uint32_t tag_id;
    SigilVec sigil;
};

class QR2RuneEngine {
public:
    QR2RuneEngine();

    void ingest_batch(const std::vector<IngestItem>& batch);

    void query(const SigilVec& query_vec,
               std::size_t top_k,
               std::vector<Record>& out_records,
               std::vector<float>& out_scores) const;

    const QR2Store& qr2() const { return qr2_; }
    const RuneIndex& rune() const { return rune_; }

private:
    QR2Store qr2_;
    RuneIndex rune_;
};

} // namespace deepthink
