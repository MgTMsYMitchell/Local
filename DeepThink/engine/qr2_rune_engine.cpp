#include "qr2_rune_engine.hpp"

namespace deepthink {

// ---------- QR2Store ----------

QR2Store::QR2Store()
    : next_id_(1),
      base_ts_(0)
{
    ids_.reserve(1024);
    ts_delta_.reserve(1024);
    tag_ids_.reserve(1024);
    glyph_ids_.reserve(1024);
}

std::vector<uint64_t> QR2Store::append_batch(
    const std::vector<uint32_t>& timestamps,
    const std::vector<uint32_t>& tag_ids,
    const std::vector<uint32_t>& glyph_ids)
{
    const std::size_t n = timestamps.size();
    std::vector<uint64_t> out_ids;
    out_ids.reserve(n);

    if (n == 0) return out_ids;

    std::unique_lock lock(mtx_);

    if (ids_.empty()) {
        base_ts_ = timestamps[0];
    }

    for (std::size_t i = 0; i < n; ++i) {
        uint64_t id = next_id_++;
        ids_.push_back(id);
        ts_delta_.push_back(timestamps[i] - base_ts_);
        tag_ids_.push_back(tag_ids[i]);
        glyph_ids_.push_back(glyph_ids[i]);
        out_ids.push_back(id);
    }

    return out_ids;
}

void QR2Store::fetch_by_ids(const std::vector<uint64_t>& ids,
                            std::vector<Record>& out) const
{
    std::shared_lock lock(mtx_);
    out.clear();
    out.reserve(ids.size());

    for (uint64_t qid : ids) {
        for (std::size_t i = 0; i < ids_.size(); ++i) {
            if (ids_[i] == qid) {
                Record r;
                r.id       = ids_[i];
                r.ts_delta = ts_delta_[i];
                r.tag_id   = tag_ids_[i];
                r.glyph_id = glyph_ids_[i];
                out.push_back(r);
                break;
            }
        }
    }
}

uint64_t QR2Store::size() const {
    std::shared_lock lock(mtx_);
    return ids_.size();
}

// ---------- RuneIndex ----------

RuneIndex::RuneIndex()
{
    ids_.reserve(1024);
    sigils_.reserve(1024);
}

void RuneIndex::insert_batch(const std::vector<uint64_t>& ids,
                             const std::vector<SigilVec>& sigils)
{
    const std::size_t n = ids.size();
    if (n == 0) return;

    std::unique_lock lock(mtx_);
    ids_.insert(ids_.end(), ids.begin(), ids.end());
    sigils_.insert(sigils_.end(), sigils.begin(), sigils.end());
}

float RuneIndex::l2(const SigilVec& a, const SigilVec& b)
{
    float s = 0.f;
    for (int i = 0; i < 14; ++i) {
        float d = a.v[i] - b.v[i];
        s += d * d;
    }
    return s;
}

void RuneIndex::search(const SigilVec& query,
                       std::size_t top_k,
                       std::vector<uint64_t>& out_ids,
                       std::vector<float>& out_scores) const
{
    std::shared_lock lock(mtx_);

    const std::size_t n = ids_.size();
    if (n == 0 || top_k == 0) {
        out_ids.clear();
        out_scores.clear();
        return;
    }

    struct Hit {
        uint64_t id;
        float score;
    };

    std::vector<Hit> hits;
    hits.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        float d = l2(query, sigils_[i]);
        hits.push_back({ ids_[i], d });
    }

    if (top_k > hits.size()) top_k = hits.size();

    std::nth_element(hits.begin(), hits.begin() + top_k, hits.end(),
                     [](const Hit& a, const Hit& b) { return a.score < b.score; });

    hits.resize(top_k);
    std::sort(hits.begin(), hits.end(),
              [](const Hit& a, const Hit& b) { return a.score < b.score; });

    out_ids.clear();
    out_scores.clear();
    out_ids.reserve(top_k);
    out_scores.reserve(top_k);

    for (const auto& h : hits) {
        out_ids.push_back(h.id);
        out_scores.push_back(h.score);
    }
}

uint64_t RuneIndex::size() const {
    std::shared_lock lock(mtx_);
    return ids_.size();
}

// ---------- QR2RuneEngine ----------

QR2RuneEngine::QR2RuneEngine()
    : qr2_(), rune_()
{}

void QR2RuneEngine::ingest_batch(const std::vector<IngestItem>& batch)
{
    const std::size_t n = batch.size();
    if (n == 0) return;

    std::vector<uint32_t> ts(n);
    std::vector<uint32_t> tags(n);
    std::vector<uint32_t> glyph_ids(n);
    std::vector<SigilVec> sigils(n);

    for (std::size_t i = 0; i < n; ++i) {
        ts[i]        = batch[i].timestamp;
        tags[i]      = batch[i].tag_id;
        glyph_ids[i] = static_cast<uint32_t>(i); // placeholder glyph mapping
        sigils[i]    = batch[i].sigil;
    }

    auto ids = qr2_.append_batch(ts, tags, glyph_ids);
    rune_.insert_batch(ids, sigils);
}

void QR2RuneEngine::query(const SigilVec& query_vec,
                          std::size_t top_k,
                          std::vector<Record>& out_records,
                          std::vector<float>& out_scores) const
{
    std::vector<uint64_t> ids;
    rune_.search(query_vec, top_k, ids, out_scores);
    qr2_.fetch_by_ids(ids, out_records);
}

} // namespace deepthink
