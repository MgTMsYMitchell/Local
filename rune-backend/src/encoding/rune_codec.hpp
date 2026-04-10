#pragma once
#include <array>
#include <optional>
#include <string>
#include <string_view>

// ── Elder Futhark codec ───────────────────────────────────────────────────────
// Provides a lookup table for all 24 Elder Futhark runes and lightweight
// helpers for name/glyph lookup and Latin→rune phoneme encoding.

namespace rune_codec {

struct RuneEntry {
    std::string_view name;
    std::string_view glyph;    // UTF-8 encoded Unicode glyph
    std::string_view phoneme;  // primary Latin phoneme (1-2 chars)
    std::string_view meaning;
    std::string_view element;
};

// Complete 24-rune Elder Futhark table (Aett order).
inline constexpr std::array<RuneEntry, 24> ELDER_FUTHARK = {{
    // First Aett (Freyr's)
    {"Fehu",     "\xe1\x9a\xa0", "f",  "Cattle, wealth, abundance",              "fire" },
    {"Uruz",     "\xe1\x9a\xa2", "u",  "Aurochs, strength, endurance",           "earth"},
    {"Thurisaz", "\xe1\x9a\xa6", "th", "Giant, thorn, chaos, defence",           "fire" },
    {"Ansuz",    "\xe1\x9a\xa8", "a",  "God, mouth, wisdom, communication",      "air"  },
    {"Raidho",   "\xe1\x9a\xb1", "r",  "Ride, journey, rhythm, right action",    "air"  },
    {"Kenaz",    "\xe1\x9a\xb2", "k",  "Torch, knowledge, enlightenment",        "fire" },
    {"Gebo",     "\xe1\x9a\xb7", "g",  "Gift, exchange, balance, partnership",   "air"  },
    {"Wunjo",    "\xe1\x9a\xb9", "w",  "Joy, fellowship, harmony, clan",         "earth"},
    // Second Aett (Hagal's)
    {"Hagalaz",  "\xe1\x9a\xba", "h",  "Hail, disruption, transformation",      "ice"  },
    {"Nauthiz",  "\xe1\x9a\xbe", "n",  "Need, necessity, constraint, survival",  "fire" },
    {"Isa",      "\xe1\x9b\x81", "i",  "Ice, stillness, stasis, clarity",        "ice"  },
    {"Jera",     "\xe1\x9b\x83", "j",  "Year, harvest, cycle, earned reward",    "earth"},
    {"Eihwaz",   "\xe1\x9b\x87", "ei", "Yew tree, death-rebirth, axis mundi",   "all"  },
    {"Perthro",  "\xe1\x9b\x88", "p",  "Fate, mystery, hidden things, womb",     "water"},
    {"Algiz",    "\xe1\x9b\x89", "z",  "Elk, protection, higher self, defence",  "air"  },
    {"Sowilo",   "\xe1\x9b\x8a", "s",  "Sun, victory, wholeness, life-force",    "fire" },
    // Third Aett (Tyr's)
    {"Tiwaz",    "\xe1\x9b\x8f", "t",  "Tyr, justice, honor, sacrifice",         "air"  },
    {"Berkano",  "\xe1\x9b\x92", "b",  "Birch, growth, fertility, new beginnings","earth"},
    {"Ehwaz",    "\xe1\x9b\x96", "e",  "Horse, trust, movement, partnership",    "earth"},
    {"Mannaz",   "\xe1\x9b\x97", "m",  "Man, humanity, self, social order",      "air"  },
    {"Laguz",    "\xe1\x9b\x9a", "l",  "Water, lake, flow, intuition, the deep", "water"},
    {"Ingwaz",   "\xe1\x9b\x9c", "ng", "Ing, fertility, completion, potential",  "earth"},
    {"Dagaz",    "\xe1\x9b\x9e", "d",  "Day, dawn, breakthrough, balance",       "fire" },
    {"Othalan",  "\xe1\x9b\x9f", "o",  "Ancestral land, heritage, estate, home", "earth"},
}};

// Lookup by name (exact, case-sensitive).
inline std::optional<RuneEntry> by_name(std::string_view name)
{
    for (const auto& r : ELDER_FUTHARK)
        if (r.name == name) return r;
    return std::nullopt;
}

// Lookup by glyph (UTF-8 sequence).
inline std::optional<RuneEntry> by_glyph(std::string_view glyph)
{
    for (const auto& r : ELDER_FUTHARK)
        if (r.glyph == glyph) return r;
    return std::nullopt;
}

// Best-effort encode a lowercase Latin string to rune glyphs.
// Two-character phonemes (th, ei, ng) are matched greedily before singles.
inline std::string encode(std::string_view text)
{
        // Build a lower-case copy for matching.
    std::string lower(text);
    for (auto& c : lower) {
        c = static_cast<char>(
            static_cast<unsigned char>(
                std::tolower(static_cast<unsigned char>(c))));
    }

    std::string out;
    out.reserve(lower.size() * 3);

    for (size_t i = 0; i < lower.size(); ) {
        bool matched = false;

        // Try 2-char phoneme first.
        if (i + 1 < lower.size()) {
            std::string_view bi{lower.data() + i, 2};
            for (const auto& r : ELDER_FUTHARK) {
                if (r.phoneme.size() == 2 && r.phoneme == bi) {
                    out += r.glyph;
                    i += 2;
                    matched = true;
                    break;
                }
            }
        }
        // Try 1-char phoneme.
        if (!matched) {
            std::string_view uni{lower.data() + i, 1};
            for (const auto& r : ELDER_FUTHARK) {
                if (r.phoneme.size() == 1 && r.phoneme == uni) {
                    out += r.glyph;
                    matched = true;
                    break;
                }
            }
            if (!matched) out += lower[i]; // pass-through unknown chars
            ++i;
        }
    }
    return out;
}

} // namespace rune_codec
