// QuantumFractalSystem — Quantum-inspired compressor implementation

#include "compression/QuantumInspiredCompressor.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace qfs {

// ── encode ───────────────────────────────────────────────────────────────────
// Uses a simple DCT-like projection: partition the input into blocks, compute
// the average amplitude per block, and keep only the top-K blocks by energy.

QState QuantumInspiredCompressor::encode(const std::vector<float>& input) const {
    if (input.empty()) return {};

    const int n = static_cast<int>(input.size());
    const int blocks = std::min(basis_size_, n);
    const int block_size = std::max(1, n / blocks);

    // Compute per-block amplitudes (RMS energy per block)
    std::vector<std::pair<float, int>> block_energy; // (amplitude, index)
    block_energy.reserve(blocks);

    for (int b = 0; b < blocks; ++b) {
        int start = b * block_size;
        int end   = (b == blocks - 1) ? n : start + block_size;
        float sum_sq = 0.0f;
        for (int i = start; i < end; ++i)
            sum_sq += input[i] * input[i];
        float rms = std::sqrt(sum_sq / static_cast<float>(end - start));
        block_energy.emplace_back(rms, b);
    }

    // Sort by descending energy and keep top-K
    std::sort(block_energy.begin(), block_energy.end(),
              [](auto& a, auto& b) { return a.first > b.first; });

    // Determine how many to keep (skip near-zero blocks)
    int keep = 0;
    for (auto& [amp, _] : block_energy) {
        if (amp < 1e-8f) break;
        ++keep;
    }

    QState state;
    state.amplitudes.reserve(keep);
    state.basis.reserve(keep);

    // Normalise amplitudes so total probability ≈ 1.0
    float total_energy = 0.0f;
    for (int i = 0; i < keep; ++i)
        total_energy += block_energy[i].first * block_energy[i].first;
    float norm = (total_energy > 1e-12f) ? std::sqrt(total_energy) : 1.0f;

    for (int i = 0; i < keep; ++i) {
        state.amplitudes.push_back(block_energy[i].first / norm);
        state.basis.push_back(block_energy[i].second);
    }

    return state;
}

// ── decode ───────────────────────────────────────────────────────────────────
// Reconstruct by distributing each amplitude uniformly across its block.

std::vector<float> QuantumInspiredCompressor::decode(const QState& state,
                                                      std::size_t output_size) const {
    if (output_size == 0) return {};

    const int n = static_cast<int>(output_size);
    const int blocks = std::min(basis_size_, n);
    const int block_size = std::max(1, n / blocks);

    std::vector<float> output(output_size, 0.0f);

    for (std::size_t k = 0; k < state.basis.size(); ++k) {
        int b     = state.basis[k];
        float amp = state.amplitudes[k];
        int start = b * block_size;
        int end   = (b == blocks - 1) ? n : start + block_size;
        float val = amp / std::sqrt(static_cast<float>(end - start));
        for (int i = start; i < end; ++i)
            output[i] = val;
    }

    return output;
}

// ── entangle ─────────────────────────────────────────────────────────────────
// Combine two states by concatenating their basis representations and
// re-normalising the amplitudes.

QState QuantumInspiredCompressor::entangle(const QState& a, const QState& b) {
    QState result;
    result.amplitudes.reserve(a.amplitudes.size() + b.amplitudes.size());
    result.basis.reserve(a.basis.size() + b.basis.size());

    result.amplitudes.insert(result.amplitudes.end(),
                             a.amplitudes.begin(), a.amplitudes.end());
    result.basis.insert(result.basis.end(), a.basis.begin(), a.basis.end());

    // Offset b's basis indices to avoid collision
    int offset = a.basis.empty() ? 0 : *std::max_element(a.basis.begin(), a.basis.end()) + 1;
    for (std::size_t i = 0; i < b.basis.size(); ++i) {
        result.amplitudes.push_back(b.amplitudes[i]);
        result.basis.push_back(b.basis[i] + offset);
    }

    // Re-normalise
    float total = 0.0f;
    for (float amp : result.amplitudes) total += amp * amp;
    if (total > 1e-12f) {
        float inv = 1.0f / std::sqrt(total);
        for (float& amp : result.amplitudes) amp *= inv;
    }

    return result;
}

// ── collapse ─────────────────────────────────────────────────────────────────
// "Measure" the state: keep only the highest-amplitude basis element.

QState QuantumInspiredCompressor::collapse(const QState& state) {
    if (state.amplitudes.empty()) return {};

    auto it = std::max_element(state.amplitudes.begin(), state.amplitudes.end());
    auto idx = static_cast<std::size_t>(std::distance(state.amplitudes.begin(), it));

    QState collapsed;
    collapsed.amplitudes = {1.0f};
    collapsed.basis = {state.basis[idx]};
    return collapsed;
}

} // namespace qfs
