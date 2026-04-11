# RQ^R2 Pocket-Galaxy Integration Addendum

> Path: docs/architecture/04-pocket-galaxy-addendum.md
> Version: 1.0.0-draft
> Author: Mitchell Turchyniak
> Depends on: 03-rqr2-module.md
> License: Proprietary — All Rights Reserved
> Status: Living Document — Canonical Reference
> Last Updated: 2026-04-11

---

## Overview

This addendum specifies 19 untapped mechanisms from the source texts
(Bohm, Hawkins, Physarum research, Hofstadter, Sheldrake, Simard, Stamets,
ARAS, Shinto ontology, Pribram, cuneiform history) that integrate directly
into the pocket-galaxy model and surrounding subsystems defined in
03-rqr2-module.md.

These are not decorative metaphors. Each mechanism fills a specific
architectural gap with a concrete, implementable pattern.

## Mechanism Index

| #  | Name                           | Source        | Target Subsystem       |
|----|--------------------------------|---------------|------------------------|
|  1 | Ink-Drop Enfoldment            | Bohm          | Pocket storage         |
|  2 | Rheomode / Verb-Pockets        | Bohm          | Pocket types           |
|  3 | Soma-Significance Loop         | Bohm          | ILE feedback           |
|  4 | Superimplicate Order           | Bohm          | Meta-glyph layer       |
|  5 | Pilot Wave Pre-Signal          | Bohm          | Mesh routing           |
|  6 | Displacement Cells             | Hawkins       | Tendril vectors        |
|  7 | Object Compositionality        | Hawkins       | Grid relations         |
|  8 | Tube-Diameter Memory           | Physarum      | Tendril routing        |
|  9 | Anticipatory Behavior          | Physarum      | Consolidation          |
| 10 | Habituation                    | Physarum      | Query ranking          |
| 11 | Typogenetics                   | Hofstadter    | Self-modifying glyphs  |
| 12 | Fluid Analogies / Slipnet      | Hofstadter    | Analogy engine         |
| 13 | Morphic Resonance              | Sheldrake     | Mesh discovery         |
| 14 | Kin Recognition                | Simard        | Lineage routing        |
| 15 | Mycoremediation                | Stamets       | Error correction       |
| 16 | Shadow Pockets                 | ARAS          | Paradox encoding       |
| 17 | Kami Threshold Emergence       | Shinto        | Activation events      |
| 18 | Holographic Fragments          | Pribram       | Fault tolerance        |
| 19 | Token-to-Tablet Compression    | Cuneiform     | Glyph variants         |

---

## Mechanism 1 — Ink-Drop Enfoldment

### Source

David Bohm's glycerine cylinder demonstration. A drop of ink is injected
into clear glycerine. The cylinder is rotated — the ink drop stretches,
thins, and vanishes into apparent uniformity. Rotate backwards the
exact number of turns and the drop reappears precisely where it was.
Multiple drops injected at different rotation counts can be selectively
unfolded without disturbing each other.

### Gap in Current Spec

Pockets in the galaxy model store symbols as flat collections. Retrieving
one symbol from a compressed pocket currently requires decompressing the
entire pocket. There is no mechanism for selective retrieval from
compressed state.

### Integration

Every pocket stores symbols at numbered enfoldment depths. Each depth
is an independent compression layer. Symbols injected at depth 7 can be
retrieved by "rotating back" to depth 7 without touching symbols at
depth 3 or depth 12.

```
ENFOLDMENT STORAGE MODEL
═════════════════════════

  pocket.enfold(symbol_A, depth=3)  // inject at depth 3
  pocket.enfold(symbol_B, depth=7)  // inject at depth 7
  pocket.enfold(symbol_C, depth=3)  // inject at depth 3 (co-located with A)

  pocket.unfold(depth=3)
  // Returns: [symbol_A, symbol_C]
  // symbol_B at depth 7 is UNDISTURBED — not decompressed, not touched

  pocket.unfold(depth=7)
  // Returns: [symbol_B]
```

Depth Assignment Rules:

| Enfoldment Depth | Trust State | Rationale                                      |
|------------------|-------------|------------------------------------------------|
| 0                | Axiom       | Surface level — always immediately accessible  |
| 1–3              | Trusted     | Shallow enfoldment — fast retrieval            |
| 4–7              | Theory      | Medium enfoldment — requires some rotation     |
| 8–11             | Dream       | Deep enfoldment — archived, slow retrieval     |
| 12               | Sealed      | Maximum depth — requires explicit unlock       |

Performance:

```
CURRENT (flat pocket):
  Retrieve 1 symbol from pocket of 1000 = decompress all 1000
  Cost: O(n)

WITH ENFOLDMENT:
  Retrieve 1 symbol from pocket of 1000 = unfold only its depth layer
  Typical layer size: ~80 symbols (1000 / 12 depths)
  Cost: O(n/d) where d = number of depth layers

  For axioms at depth 0: O(1) — instant, no decompression
```

Schema Addition:

```sql
ALTER TABLE symbols ADD COLUMN enfoldment_depth INTEGER DEFAULT 0;
CREATE INDEX idx_symbols_enfoldment ON symbols(enfoldment_depth);
```

JSON Schema Addition (in EncodedSymbol):

```json
{
  "enfoldment_depth": {
    "type": "integer",
    "minimum": 0,
    "maximum": 12,
    "default": 0,
    "description": "Bohm ink-drop layer. 0=surface/axiom, 12=deep sealed."
  }
}
```

---

## Mechanism 2 — Rheomode / Verb-Pockets

### Source

Bohm proposed the "rheomode" — a restructuring of language where verbs
are primary and nouns are derived. Reality is fundamentally movement,
not things. He invented words like "relevate" (to lift into relevance)
instead of using the static adjective "relevant."

### Gap in Current Spec

All pockets store symbol objects (nouns). Movement glyphs exist as
annotations on symbols but have no first-class home. The galaxy can
answer "what is X?" but cannot natively answer "what is happening?"

### Integration

Introduce verb-pockets as a distinct pocket type alongside the
existing noun-pockets. Verb-pockets store transformation sequences —
ordered chains of movement and relationship glyphs that represent
ongoing processes.

```
POCKET TYPES
════════════

  NOUN-POCKET (existing):
    Contains: EncodedSymbol objects
    Answers:  "What is X?"
    Indexed:  by radical, by sem_x/y/z, by domain
    Decays:   by access frequency (unused → stale)
    Example:  { CR-32 Remember, CR-16 Tower } = "cache server"

  VERB-POCKET (new):
    Contains: TransformationSequence objects
    Answers:  "What is happening to X?"
    Indexed:  by movement type, by participant symbols, by temporal phase
    Decays:   by process recency (finished process → archive)
    Example:  [ MOV-SE + TRN-DECAY + CR-32 ] = "cache is decaying"
```

TransformationSequence Schema:

```json
{
  "$id": "https://qrrune.local/schemas/transformation-sequence.json",
  "title": "TransformationSequence",
  "description": "A verb-pocket entry representing an ordered process",
  "type": "object",
  "required": ["id", "steps", "participants", "status", "created_at"],
  "properties": {
    "id": { "type": "string", "format": "uuid" },
    "steps": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "order":           { "type": "integer", "minimum": 0 },
          "movement_glyph":  { "type": "string" },
          "velocity":        { "type": "string",
                               "enum": ["still","drift","walk","rush","flash"] },
          "manner":          { "type": "string",
                               "enum": ["smooth","pulse","stagger","spiral"] },
          "subject_symbol":  { "type": "string", "format": "uuid" },
          "object_symbol":   { "type": ["string","null"], "format": "uuid" }
        }
      },
      "minItems": 1
    },
    "participants": {
      "type": "array",
      "items": { "type": "string", "format": "uuid" }
    },
    "status": {
      "type": "string",
      "enum": ["active", "completed", "suspended", "archived"]
    },
    "temporal_phase": {
      "type": "number", "minimum": -1.0, "maximum": 1.0,
      "description": "-1=past process, 0=current, +1=anticipated"
    },
    "trust_state": {
      "type": "string",
      "enum": ["dream", "theory", "trusted", "axiom"]
    },
    "created_at":  { "type": "integer" },
    "last_active": { "type": ["integer", "null"] }
  }
}
```

Query Examples:

```
NOUN QUERY: "What is the cache?"
  → Search noun-pockets by radical CR-32
  → Returns: symbol definitions

VERB QUERY: "What is happening to the cache?"
  → Search verb-pockets by participant containing CR-32 symbols
  → Returns: active transformation sequences
  → Example: "Cache is undergoing TRN-DECAY at VEL-DRIFT"

COMBINED: "Why is the cache stale?"
  → Noun: "cache = memory store"         (what it IS)
  → Verb: "cache undergoing decay process" (what is HAPPENING)
  → Combined answer is richer than either alone
```

---

## Mechanism 3 — Soma-Significance Bidirectional Loop

### Source

Bohm proposed that meaning flows in two directions simultaneously:
soma → significance (physical form generates meaning upward) and
significance → soma (meaning reshapes physical form downward).

Seeing a shadow → interpreting "threat" → adrenaline → physical change.
The interpretation physically altered the body. Reading is not passive.

### Gap in Current Spec

When the ILE interprets a glyph, it reads the halo, produces meaning,
and emits a result. The glyph itself is unchanged by the act of being
read. Metadata is effectively read-only during interpretation. The
system is one-directional: soma → significance only.

### Integration

Every ILE interpretation is a bidirectional event. The act of reading
a glyph writes back to the glyph's halo. The pocket-galaxy is not a
library — it is a living system where observation changes the observed.

```
CURRENT ILE PIPELINE (one-way):
  Read glyph → produce meaning → emit result
  Glyph: unchanged

ENHANCED ILE PIPELINE (bidirectional):
  Read glyph → produce meaning → emit result
       └──────── SIMULTANEOUSLY ────────┐
                                        ▼
                               Write back to glyph:
                                 activation   ↑  (woke it up)
                                 brightness   ↑  (made it visible)
                                 decay_score  = 1.0  (refresh)
                                 access_count++
                                 accessed_at  = now()
                                 halo_glow    ↑  (implicate visible)
                                 trust_score  ±δ (context match)
```

Feedback Rules:

| ILE Outcome                               | Soma Feedback                          | Magnitude           |
|-------------------------------------------|----------------------------------------|---------------------|
| Prediction matched (fast path)            | trust += 0.01, activation += 0.1      | Small positive      |
| Prediction missed, resolved               | trust unchanged, activation += 0.2    | Neutral awakening   |
| Ambiguity unresolved                      | trust -= 0.02, halo_glow += 0.1       | Slight doubt        |
| Contradicted by context                   | trust -= 0.05, brightness -= 0.1      | Weakening           |
| Confirmed by multiple neighbors           | trust += 0.03, connectivity += 1      | Strong reinforcement|
| Dream accessed in consolidation           | activation += 0.3, consol_count++     | Dream becomes visible|

Implementation:

```rust
fn ile_interpret(
    symbol: &mut EncodedSymbol,
    context: &QueryContext
) -> InterpretedMeaning {
    // SOMA → SIGNIFICANCE (existing)
    let meaning = decode(symbol, context);

    // SIGNIFICANCE → SOMA (new bidirectional feedback)
    symbol.accessed_at   = now();
    symbol.access_count += 1;
    symbol.decay_score   = 1.0;
    symbol.halo.activation = min(1.0, symbol.halo.activation + 0.1);

    match meaning.confidence_vs_prediction {
        PredictionMatch  => symbol.trust_score += 0.01,
        PredictionMiss   => { /* no change */ },
        Contradiction    => symbol.trust_score -= 0.05,
        MultiConfirm     => symbol.trust_score += 0.03,
    }

    symbol.halo.brightness = trust_to_brightness(symbol.trust_score);
    persist(symbol); // write changes back to BrainDb

    meaning
}
```

Why This Matters:

Without bidirectional feedback, the knowledge graph is a static
database that happens to have pretty metadata. WITH it, every query
reshapes the knowledge landscape. Frequently accessed symbols grow
brighter and more trusted. Contradicted symbols dim. The galaxy is
alive — shaped by the act of being observed.

---

## Related documents

- [03 — RQ^R2 Encoder Module](03-rqr2-module.md)
- [02 — Agent Framework](02-agent-framework.md)
- [07 — Storage Layer](07-storage.md)
- [06 — Mesh Networking](06-mesh-networking.md)
