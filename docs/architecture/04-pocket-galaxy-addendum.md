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

## Mechanism 4 — Superimplicate Order

### Source

Beyond the implicate order, Bohm proposed a superimplicate order — a
yet-deeper layer that organizes the implicate order itself. If the
explicate is the visible world and the implicate is the hidden reality
beneath it, the superimplicate is the *pattern that shapes the hidden
reality*.

### Gap in Current Spec

The spec has two layers:
- Explicate: the visible glyph (strokes, grid, rendered form)
- Implicate: the halo metadata (color, trust, activation, etc.)

There is no third layer. The rules that govern how glyphs form,
transform, and relate are hardcoded in the engine, not represented
within the symbolic substrate itself.

### Integration

Add the superimplicate layer: meta-glyphs stored as axioms that
describe the encoding process itself. The system contains symbols that
describe how symbols work.

```
THREE-ORDER MODEL
═════════════════

  ORDER 1 — EXPLICATE (visible)
    What you see: strokes, radicals, grid layout, rendered glyph
    Stored in:    glyph_data, strokes, grid
    Analog:       the ink drop you can see

  ORDER 2 — IMPLICATE (hidden metadata)
    What you sense: color, trust, activation, connectivity, lineage
    Stored in:      halo_json
    Analog:         the ink drop stretched into glycerine — invisible
                    but recoverable

  ORDER 3 — SUPERIMPLICATE (rules-about-rules)
    What governs:  how glyphs compose, how trust escalates,
                   how dreams consolidate, how tendrils grow
    Stored in:     axiom-class meta-glyphs in the symbols table
    Analog:        the physics of glycerine itself — why rotation
                   enfolds and unfolds
```

Meta-Glyph Axioms:

```
AXIOM-100: COMPOSITION LAW
  Grid: A=CR-53(Rule)  B=CR-34(Create)  C=CR-42(Child)  D=CR-41(Parent)
  Meaning: "The rule for creating children from parents"
  Encodes: the 2x2 grid composition algorithm itself
  Self-referential: this axiom was composed using the rules it describes

AXIOM-101: TRUST ESCALATION LAW
  Grid: A=CR-53(Rule)  B=CR-38(Dream)  C=CR-57(Truth)  D=CR-55(Time)
  Meaning: "Dreams become truth over time according to rules"
  Encodes: the dream → theory → trusted pipeline

AXIOM-102: DECAY LAW
  Grid: A=CR-53(Rule)  B=CR-25(Wilt)  C=CR-55(Time)  D=CR-40(Release)
  Meaning: "Things wilt over time and are released"
  Encodes: the exponential decay formula

AXIOM-103: TENDRIL GROWTH LAW
  Grid: A=CR-53(Rule)  B=CR-05(Vine)  C=CR-46(Feed)  D=CR-44(Bond)
  Meaning: "Vines grow to feed through bonds"
  Encodes: the Physarum routing algorithm

AXIOM-104: SELF-DESCRIPTION LAW
  Grid: A=CR-53(Rule)  B=CR-53(Rule)  C=CR-51(Agent)  D=CR-48(Observe)
  Meaning: "The rule about rules: an agent observing itself"
  Encodes: THIS MECHANISM — the superimplicate is self-describing
  This is the strange loop that makes the system self-aware.
```

Why This Matters:

If the superimplicate axioms are stored in the same substrate as regular
symbols, agents can query the rules:

```
query("How does trust escalation work?")
→ Returns: AXIOM-101 (Trust Escalation Law)
→ The system can explain its own behavior in its own symbolic language
```

An agent can also detect contradictions between its behavior and its
rules — if the engine is doing something that contradicts AXIOM-101,
the Overwatch agent can flag it by comparing observed behavior against
the superimplicate axioms.

Schema Addition:

```sql
ALTER TABLE symbols ADD COLUMN is_superimplicate BOOLEAN DEFAULT FALSE;
CREATE INDEX idx_symbols_superimplicate ON symbols(is_superimplicate)
  WHERE is_superimplicate = TRUE;
```

JSON Schema Addition:

```json
{
  "is_superimplicate": {
    "type": "boolean",
    "default": false,
    "description": "True if this symbol encodes a rule about how the system itself operates (Bohm third order)"
  },
  "governs_process": {
    "type": ["string", "null"],
    "enum": [
      "composition", "trust_escalation", "decay", "tendril_growth",
      "consolidation", "routing", "encoding", "self_description", null
    ],
    "description": "Which system process this superimplicate axiom governs"
  }
}
```

---

## Mechanism 5 — Pilot Wave Pre-Signal

### Source

Bohm's pilot wave theory (de Broglie-Bohm interpretation) proposes that
a quantum particle is guided by an invisible wave that arrives at the
destination before the particle does. The wave explores all possible
paths, and the particle follows the optimal one. The wave carries
information without the particle's mass.

### Gap in Current Spec

When a glyph is routed through the mesh, the full glyph is transmitted
as a single unit. Receiving nodes have no advance notice. They cannot
pre-warm caches, pre-allocate buffers, or begin predictive decoding.
Every message arrival is a cold start.

### Integration

Before transmitting a full glyph, send a pilot signal — a
lightweight metadata-only packet that travels the tendril network
ahead of the full glyph.

```
ROUTING WITHOUT PILOT (current):
  1. Node A encodes glyph (50ms)
  2. Node A transmits full glyph (payload: ~2KB)
  3. Node B receives (cold cache)
  4. Node B decodes (80ms — no pre-warming)
  Total: 130ms + network latency

ROUTING WITH PILOT (new):
  1. Node A encodes glyph (50ms)
  2. Node A sends PILOT (payload: ~200 bytes)        ← NEW
  3. Node B receives pilot (hot cache prep)           ← NEW
     └── pre-loads relevant radicals into memory
     └── pre-activates domain pockets
     └── begins ILE prediction from halo alone
  4. Node A sends full glyph (payload: ~2KB)
  5. Node B receives + decodes (20ms — pre-warmed)   ← 4x FASTER
  Total: 70ms + network latency (decode overlapped with transit)
```

Pilot Signal Schema:

```json
{
  "type": "PILOT_SIGNAL",
  "payload": {
    "glyph_id": "uuid",
    "halo_preview": {
      "domain": "fungi",
      "trust_state": "theory",
      "ac_ratio": 0.25,
      "activation": 0.7,
      "sem_x": -0.3,
      "sem_y": 0.5,
      "sem_z": 0.2
    },
    "radical_hints": ["CR-04", "CR-44"],
    "estimated_arrival_ms": 150,
    "glyph_size_bytes": 2048,
    "requires_dream_buffer": false,
    "priority": "normal"
  }
}
```

New Mesh Message Type:

```
PILOT_SIGNAL
  Sender:   any peer
  Payload:  halo preview + radical hints + arrival estimate
  Purpose:  "Something is coming — prepare for it"
  Size:     ~200 bytes (10% of full glyph)
  TTL:      1 (direct neighbors only — pilots don't relay)
  Behavior: Recipient pre-warms caches, pre-activates pockets
```

Receiver Pre-Warm Algorithm:

```rust
fn handle_pilot(pilot: PilotSignal) {
    // 1. Pre-load radicals
    for radical_id in pilot.radical_hints {
        radical_cache.warm(radical_id);
    }

    // 2. Pre-activate relevant pocket
    let pocket = galaxy.find_pocket_by_domain(
        pilot.halo_preview.domain
    );
    pocket.pre_activate();

    // 3. Begin predictive ILE from halo alone
    let prediction = ile.predict_from_halo(pilot.halo_preview);
    prediction_cache.store(pilot.glyph_id, prediction);

    // 4. Pre-allocate buffer
    if pilot.requires_dream_buffer {
        dream_buffer.reserve(pilot.glyph_size_bytes);
    }

    // When full glyph arrives, check prediction_cache first
    // If prediction matches → VEL-FLASH instant decode
}
```

---

## Mechanism 6 — Displacement Cells

### Source

Jeff Hawkins describes "displacement cells" in cortical columns —
neurons that encode the difference between where you are and where
something else is. Not absolute position, but relative offset. This
is how you know "the handle is to the left of the cup" regardless of
where the cup is in space.

### Gap in Current Spec

Glyphs have absolute sem_x/y/z coordinates. Tendrils connect symbols
but carry no spatial information. If you find symbol A, you cannot
calculate where B is — you must search for B independently. The
galaxy is navigable only by index lookup, not by spatial reasoning.

### Integration

Tendrils carry displacement vectors — the semantic distance and
direction from source to target. Finding any symbol lets you dead-reckon
to its neighbors without searching.

```
CURRENT TENDRIL:
  source: symbol_A  (sem: 0.3, 0.5, -0.2)
  target: symbol_B  (sem: 0.7, 0.1,  0.4)
  weight: 0.85
  type:   association
  // To find B, you must look up B's coordinates independently

ENHANCED TENDRIL:
  source:       symbol_A  (sem: 0.3, 0.5, -0.2)
  target:       symbol_B  (sem: 0.7, 0.1,  0.4)
  weight:       0.85
  type:         association
  displacement: { dx: +0.4, dy: -0.4, dz: +0.6 }   ← NEW
  // To find B from A: A.sem + displacement = B.sem
  // No lookup required — pure spatial inference
```

Schema Addition:

```sql
ALTER TABLE tendrils ADD COLUMN displacement_x REAL;
ALTER TABLE tendrils ADD COLUMN displacement_y REAL;
ALTER TABLE tendrils ADD COLUMN displacement_z REAL;
```

Dead-Reckoning Navigation:

```
START: Found symbol A at (0.3, 0.5, -0.2)

STEP 1: A has tendril to B with displacement (+0.4, -0.4, +0.6)
  → B should be at (0.7, 0.1, 0.4)
  → Navigate directly — no search needed

STEP 2: B has tendril to C with displacement (-0.1, +0.3, +0.1)
  → C should be at (0.6, 0.4, 0.5)
  → Navigate directly — no search needed

CHAIN: A → B → C via pure vector addition
  Total displacement A→C: (+0.3, -0.1, +0.7)
  → Store as shortcut tendril A→C for future use
```

Why This Matters:

1. Search-free navigation — follow vectors instead of querying indexes
2. Analogy detection — if A→B has the same displacement as C→D,
   then A:B :: C:D (proportional analogy discovered structurally)
3. Missing symbol inference — if you know A and the displacement
   to B but B doesn't exist yet, you know where it should be in
   semantic space. The system predicts unfilled gaps.

JSON Schema Addition (in Tendril):

```json
{
  "displacement": {
    "type": ["object", "null"],
    "properties": {
      "dx": { "type": "number" },
      "dy": { "type": "number" },
      "dz": { "type": "number" }
    },
    "description": "Hawkins displacement vector. target.sem = source.sem + displacement"
  }
}
```

---

## Mechanism 7 — Object Compositionality

### Source

Hawkins describes how cortical columns learn that a complex object (cup)
is a composition of parts (handle, rim, body), each with its own
reference frame. The brain doesn't store "cup" as one holistic blob — it
stores the spatial relationships between parts. Touching the handle
alone is enough to reconstruct the whole cup.

### Gap in Current Spec

Glyphs have radicals (parts) placed in a 2×2 grid. But there is no
data about how the radicals relate to each other spatially. The grid
is a layout tool, not a relational map. You cannot reconstruct the whole
glyph from a single radical.

### Integration

Enhance the composition grid with inter-radical displacement vectors
and relation types. Every pair of radicals in a glyph has a defined
spatial and semantic relationship.

Enhanced CompositionGrid Schema:

```json
{
  "grid": {
    "A": "CR-54",
    "B": "CR-16",
    "C": "CR-32",
    "D": "CR-08",
    "inter_radical_relations": [
      {
        "from": "A", "to": "B",
        "displacement": [0.3, 0.0, -0.1],
        "relation": "classifier-of",
        "strength": 0.9,
        "description": "Error classifies Tower"
      },
      {
        "from": "A", "to": "C",
        "displacement": [0.0, -0.4, 0.0],
        "relation": "caused-by",
        "strength": 0.7,
        "description": "Error caused by Memory"
      },
      {
        "from": "C", "to": "D",
        "displacement": [0.2, 0.1, 0.3],
        "relation": "manifests-as",
        "strength": 0.6,
        "description": "Memory manifests as Water (leak)"
      },
      {
        "from": "A", "to": "D",
        "displacement": [0.5, -0.3, 0.2],
        "relation": "expressed-through",
        "strength": 0.5,
        "description": "Error expressed through Water"
      }
    ]
  }
}
```

Reconstruction from Fragment:

```
SCENARIO: System receives only radical CR-32 (Remember) from a
          corrupted or partial glyph transmission.

STEP 1: Look up all glyphs containing CR-32 in position C
STEP 2: For each candidate, check inter_radical_relations:
  → CR-32(C) has relation "caused-by" to position A
  → CR-32(C) has relation "manifests-as" to position D
STEP 3: Use displacement vectors to predict what A and D should be
STEP 4: Match predictions against known radicals
  → A predicted at offset (0.0, +0.4, 0.0) from CR-32 → CR-54 (Error)
  → D predicted at offset (0.2, 0.1, 0.3)  from CR-32 → CR-08 (Water)
STEP 5: Reconstruct full glyph from single fragment

RESULT: "Error-Tower-Remember-Water" = server memory leak crash
        Entire glyph reconstructed from ONE radical + its relations.
```

Relation Types:

| Relation           | Meaning               | Typical Grid Pair |
|--------------------|-----------------------|-------------------|
| classifier-of      | A categorizes B       | A → B             |
| caused-by          | A was caused by C     | A → C             |
| manifests-as       | C appears as D        | C → D             |
| expressed-through  | A shows via D         | A → D             |
| modifies           | B adjusts C           | B → C             |
| elaborates         | D extends B           | D → B             |

Why This Matters:

1. Fault tolerance — partial glyph transmissions can be fully
   reconstructed from any surviving radical
2. Semantic search — find any radical, navigate its relations to
   discover the full concept
3. Compression — transmit only one radical + relation table instead
   of full glyph (for bandwidth-constrained mesh links)
4. Learning — understanding HOW parts relate is deeper knowledge
   than just knowing WHAT parts exist

---

## Mechanism 8 — Tube-Diameter Memory

### Source

Physarum polycephalum (slime mold) stores memory in the physical
diameter of its tubes. Tubes carrying more nutrients grow thicker.
Unused tubes atrophy and thin. The network's geometry is its memory —
no separate memory store is needed. Flow capacity scales with the square
of the diameter (Hagen-Poiseuille law).

### Gap in Current Spec

Tendrils have a `weight` field that increments and decrements linearly.
This produces uniform scaling — a tendril at 0.8 carries only 4× the
importance of one at 0.2. Real Physarum networks develop extreme
contrast between highways and footpaths because of quadratic scaling.

### Integration

Replace the abstract weight with a physically-modeled diameter
that governs flow capacity quadratically.

```
CURRENT MODEL (linear):
  weight 0.2 → capacity 0.2
  weight 0.8 → capacity 0.8
  Ratio: 4:1

PHYSARUM MODEL (quadratic):
  diameter 0.2 → capacity π*(0.1)² = 0.031
  diameter 0.8 → capacity π*(0.4)² = 0.503
  Ratio: 16:1

  The thick tendril carries SIXTEEN TIMES more flow.
  This naturally creates highways vs footpaths.
```

Schema Change:

```sql
-- Rename weight → diameter (semantic upgrade, same column type)
ALTER TABLE tendrils RENAME COLUMN weight TO diameter;

-- Flow capacity is computed at query time:
-- flow_capacity = pi * (diameter / 2)^2
-- Or as a generated column:
-- ALTER TABLE tendrils ADD COLUMN flow_capacity REAL
-- GENERATED ALWAYS AS (3.14159265 * (diameter / 2.0) * (diameter / 2.0)) STORED;
```

Updated Routing Rules:

```
1. STRENGTHEN ON USE
   traversed → diameter += 0.01
   (slower growth than before — quadratic effect compensates)

2. ATROPHY ON NEGLECT
   unused → diameter *= 0.995 per cycle
   (slower atrophy — thin tubes persist longer as footpaths)

3. PRUNE THRESHOLD
   diameter < 0.005 → prune  (was 0.01 for weight)

4. ROUTE SELECTION
   Choose path with maximum product(flow_capacity) along path
   flow_capacity = π * (diameter/2)²

5. HIGHWAY EMERGENCE
   After ~100 cycles of natural traffic:
   - 5–10 "highway" tendrils with diameter > 0.7
   - 50–100 "roads" with diameter 0.2–0.5
   - 200+ "footpaths" with diameter < 0.1
   The network self-organizes into hierarchical transport
   WITHOUT any central planner.
```

Emergent Infrastructure Tiers:

| Diameter Range | Flow Capacity  | Tier Name | Function                                        |
|----------------|----------------|-----------|-------------------------------------------------|
| 0.8 – 1.0      | 0.50 – 0.79    | Highway   | Primary inter-cluster routes, axiom connections |
| 0.5 – 0.8      | 0.20 – 0.50    | Road      | Regular inter-pocket routes                     |
| 0.2 – 0.5      | 0.03 – 0.20    | Path      | Occasional associations                         |
| 0.05 – 0.2     | 0.002 – 0.03   | Footpath  | Rarely used, may atrophy                        |
| < 0.05         | < 0.002        | Thread    | Near-death, one cycle from pruning              |

JSON Schema Update (Tendril):

```json
{
  "diameter": {
    "type": "number",
    "minimum": 0.0,
    "maximum": 1.0,
    "description": "Tube diameter (Physarum model). Replaces weight. Flow capacity scales quadratically: π*(d/2)²"
  },
  "flow_capacity": {
    "type": "number",
    "minimum": 0.0,
    "readOnly": true,
    "description": "Computed: π * (diameter/2)². Do not set directly."
  }
}
```

---

## Mechanism 9 — Anticipatory Behavior

### Source

When Physarum is exposed to periodic cold shocks every 60 minutes, it
learns the rhythm and preemptively slows its growth before the
next shock arrives — even when the shock doesn't come. A brainless,
single-celled organism, predicting the future from temporal patterns.

### Gap in Current Spec

The system is entirely reactive. Queries arrive, get processed, results
returned. There is no mechanism for detecting temporal patterns in the
input stream and pre-activating relevant resources before the next
expected event.

### Integration

Add an Anticipation Engine as a sub-stage of the consolidation
cycle. It detects periodic patterns and pre-warms the system.

```
ANTICIPATION ENGINE
═══════════════════

Runs during: Consolidation cycle (new sub-stage between N3 and REM)

STEP 1 — RHYTHM DETECTION
  Scan query log for periodic patterns:
  - Collect timestamps of queries grouped by domain/radical
  - Apply autocorrelation to find dominant period T
  - Confidence = peak autocorrelation coefficient
  - Threshold: confidence > 0.7 to register a rhythm

STEP 2 — PATTERN REGISTRATION
  Store detected rhythms in the rhythms table (see schema below)

STEP 3 — PRE-ACTIVATION
  When current_time approaches next_predicted (within 1 period/10):
  - Pre-warm relevant pockets (unfold enfoldment layers)
  - Pre-activate relevant symbols (dormant → awakened)
  - Pre-compute ILE predictions for expected query pattern
  - Pre-expand relevant tendrils (diameter += 0.005 boost)

STEP 4 — OUTCOME TRACKING
  IF anticipated event arrives within tolerance window:
    → hit_count++
    → confidence recalculated
    → Response delivered at VEL-FLASH speed (pre-computed)
  IF anticipated event does NOT arrive:
    → miss_count++
    → Log as "phantom anticipation"
    → If miss_count > hit_count → deactivate rhythm

STEP 5 — PHANTOM VALUE
  Phantom anticipations are not waste — they are data:
  - "We expected a water-domain query every hour but it stopped"
  - This absence IS information
  - Generate a dream: "periodic water-query pattern has broken"
  - The missing expected event becomes a signal
```

Schema Addition:

```sql
CREATE TABLE IF NOT EXISTS rhythms (
    id             TEXT    PRIMARY KEY,
    domain         TEXT,
    radicals       TEXT,             -- JSON array of radical IDs
    period_ms      INTEGER NOT NULL,
    confidence     REAL    NOT NULL DEFAULT 0.0,
    last_occurrence INTEGER,
    next_predicted  INTEGER,
    hit_count      INTEGER DEFAULT 0,
    miss_count     INTEGER DEFAULT 0,
    status         TEXT    DEFAULT 'active',
        -- active:   currently tracking
        -- dormant:  too many misses, paused
        -- expired:  deactivated permanently
    created_at     INTEGER NOT NULL
);

CREATE INDEX idx_rhythms_status ON rhythms(status);
CREATE INDEX idx_rhythms_next   ON rhythms(next_predicted);
```

JSON Schema:

```json
{
  "$id": "https://qrrune.local/schemas/rhythm.json",
  "title": "Rhythm",
  "description": "A detected periodic pattern in the query stream",
  "type": "object",
  "required": ["id", "period_ms", "confidence", "status", "created_at"],
  "properties": {
    "id":               { "type": "string", "format": "uuid" },
    "domain":           { "type": ["string", "null"] },
    "radicals": {
      "type": "array",
      "items": { "type": "string", "pattern": "^CR-[0-6][0-9]$" }
    },
    "period_ms":        { "type": "integer", "minimum": 1000 },
    "confidence":       { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "last_occurrence":  { "type": ["integer", "null"] },
    "next_predicted":   { "type": ["integer", "null"] },
    "hit_count":        { "type": "integer", "minimum": 0, "default": 0 },
    "miss_count":       { "type": "integer", "minimum": 0, "default": 0 },
    "status": {
      "type": "string",
      "enum": ["active", "dormant", "expired"]
    },
    "created_at": { "type": "integer" }
  }
}
```

Query Performance Impact:

```
WITHOUT ANTICIPATION:
  Query arrives → cold decode → 80ms response

WITH ANTICIPATION (rhythm detected, pre-warmed):
  Query arrives → prediction cache hit → 5ms response
  16x speedup for predictable periodic queries

WITH ANTICIPATION (phantom — query doesn't arrive):
  Pre-warmed resources idle → auto-release after timeout
  Cost:    ~2ms wasted CPU + small memory allocation
  Benefit: generated dream about broken pattern (information gain)
```

---

## Mechanism 10 — Habituation

### Source

Physarum exhibits habituation — repeated exposure to a harmless stimulus
causes the organism to stop responding to it. It is not forgetting —
it is learning to ignore. After a rest period without the stimulus, the
response returns (dishabituation). This is the simplest form of learning
in biology.

### Gap in Current Spec

Decay exists but is purely time-based. A symbol queried 10,000 times
never fades into background. High-frequency symbols dominate attention
permanently. There is no mechanism for "I have seen this too many times,
deprioritize it."

### Integration

Add a habituation system that mutes over-accessed symbols without
deleting them.

```
HABITUATION LIFECYCLE
═════════════════════

                    access_count
                    │
  0────────────────►threshold──────────────────►
  │                 │                           │
  │  NOVEL          │  HABITUATING              │  HABITUATED
  │  Full response  │  Diminishing response     │  Muted
  │  High activation│  Activation declining     │  activation = 0.1
  │  Full AEL       │  AEL.novelty fading       │  AEL.novelty = 0.0
  │                 │                           │
  │                 │                           │  (rest period
  │                 │                           │  with no access)
  │                 │                           │
  │  DISHABITUATED ◄┼───────────────────────────┘
  │  Novelty spike! │  reset habituation_count
  │  "Wait, this    │  full activation restored
  │  is fresh       │
  │  again?"        │
  └─────────────────┘

  ALSO: content change while habituated → INSTANT dishabituation
```

Schema Addition:

```sql
ALTER TABLE symbols ADD COLUMN habituation_count     INTEGER DEFAULT 0;
ALTER TABLE symbols ADD COLUMN habituation_threshold INTEGER DEFAULT 50;
ALTER TABLE symbols ADD COLUMN habituated_at         INTEGER;
```

Rules:

```
RULE 1 — ACCUMULATION
  Each query returning this symbol: habituation_count++

RULE 2 — TRIGGER
  When habituation_count > habituation_threshold:
    activation      = 0.1  (muted, not zero)
    ael.novelty     = 0.0
    habituated_at   = now()
    Symbol STILL returned in queries but ranked lower
    Symbol STILL participates in tendril routing

RULE 3 — DISHABITUATION (time-based)
  If NOT queried for (habituation_threshold * 2) ticks:
    habituation_count = 0
    habituated_at     = null
    Novelty restored — next access gets full activation spike

RULE 4 — DISHABITUATION (change-based)
  If symbol's content CHANGES while habituated:
    Immediate dishabituation
    Novelty spike: ael.novelty = 0.95
    activation = 0.9
    "Wait — this familiar thing is different now"

RULE 5 — THRESHOLD ADAPTATION
  Symbols that habituate and dishabituate repeatedly:
    habituation_threshold increases by 10 each cycle
    System learns "this one takes longer to tune out"
    Prevents pathological habituation/dishabituation loops
```

Query Ranking Impact:

```
QUERY: "What is the cache?"

WITHOUT HABITUATION:
  1. [TRUSTED] cache-definition         (accessed 10,000 times) ← always #1
  2. [TRUSTED] cache-staleness-pattern  (accessed 50 times)
  3. [THEORY]  cache-network-correlation (accessed 3 times)

WITH HABITUATION:
  1. [TRUSTED] cache-staleness-pattern  (50 accesses,     novelty 0.6)
  2. [THEORY]  cache-network-correlation (3 accesses,      novelty 0.9)
  3. [TRUSTED] cache-definition         (10,000 accesses, HABITUATED, novelty 0.0)
     ↑ Still returned, but ranked last.
       The system knows you already know what a cache is.
       Novel and developing knowledge surfaces first.
```

JSON Schema Addition:

```json
{
  "habituation_count": {
    "type": "integer", "minimum": 0, "default": 0,
    "description": "Physarum habituation counter. Increments per query access."
  },
  "habituation_threshold": {
    "type": "integer", "minimum": 10, "default": 50,
    "description": "Access count at which symbol becomes habituated (muted)."
  },
  "habituated_at": {
    "type": ["integer", "null"],
    "description": "Timestamp when habituation triggered. null = not habituated."
  }
}
```

---

## Mechanism 11 — Typogenetics / Self-Modifying Glyphs

### Source

In *Gödel, Escher, Bach*, Hofstadter describes "typogenetics" — a system
where strands of genetic-like code contain instructions that, when
executed, modify the strand itself. The code is both the program and
the data. Executing the strand changes what the strand says, which
changes what it does when executed next time. Self-modification as a
fundamental computational primitive.

### Gap in Current Spec

Glyphs are static after encoding. Their metadata changes (trust,
activation, decay) but their radical composition — the actual semantic
content — is fixed at creation time. A glyph cannot evolve its own
meaning through use.

### Integration

Introduce self-modifying glyphs: glyphs carrying embedded
transformation instructions. When interpreted by the ILE enough times
or when certain conditions are met, the glyph rewrites its own
radicals according to its embedded program.

Typogenetic Program Schema:

```json
{
  "typogenetic_program": {
    "type": ["object", "null"],
    "description": "Self-modification rules embedded in the glyph",
    "properties": {
      "enabled": { "type": "boolean", "default": false },
      "rules": {
        "type": "array",
        "items": {
          "type": "object",
          "required": ["condition", "action"],
          "properties": {
            "condition": {
              "type": "object",
              "properties": {
                "trigger": {
                  "type": "string",
                  "enum": [
                    "access_count", "trust_score", "decay_score",
                    "habituation_count", "connectivity", "age_ticks"
                  ]
                },
                "operator": {
                  "type": "string",
                  "enum": [">=", "<=", "==", "!=", ">", "<"]
                },
                "value": { "type": "number" }
              }
            },
            "action": {
              "type": "object",
              "properties": {
                "type": {
                  "type": "string",
                  "enum": [
                    "mutate_radical", "add_radical",
                    "remove_radical", "swap_positions"
                  ]
                },
                "target_position": {
                  "type": "string",
                  "enum": ["A", "B", "C", "D"]
                },
                "from_radical": {
                  "type": ["string", "null"],
                  "pattern": "^CR-[0-6][0-9]$"
                },
                "to_radical": {
                  "type": ["string", "null"],
                  "pattern": "^CR-[0-6][0-9]$"
                },
                "mutation_type": {
                  "type": ["string", "null"],
                  "enum": [
                    "i-mutation", "u-mutation",
                    "a-mutation", "ö-mutation", null
                  ]
                },
                "merge_mode": {
                  "type": ["string", "null"],
                  "enum": ["replace", "bind_rune", null]
                }
              }
            },
            "description": { "type": "string" }
          }
        }
      },
      "mutation_log": {
        "type": "array",
        "items": {
          "type": "object",
          "properties": {
            "timestamp":  { "type": "integer" },
            "rule_index": { "type": "integer" },
            "before":     { "type": "string" },
            "after":      { "type": "string" },
            "reason":     { "type": "string" }
          }
        }
      },
      "generation": {
        "type": "integer", "minimum": 0, "maximum": 5, "default": 0
      }
    }
  }
}
```

Example — A Glyph That Learns:

```
INITIAL GLYPH: "server memory leak"
  A=CR-54(Error)  B=CR-16(Tower)  C=CR-32(Remember)  D=CR-08(Water)

TYPOGENETIC RULE 1:
  condition: access_count >= 100
  action:    mutate_radical D  from CR-08(Water) to CR-22(Spiral)
             via i-mutation
  meaning:   After 100 accesses, the system learns this is not a
             one-time leak — it is a recurring cycle.
             "Water(leak)" becomes "Spiral(cycle)"

TYPOGENETIC RULE 2:
  condition: trust_score >= 0.9
  action:    add_radical CR-57(Truth) to position D via bind_rune
  meaning:   When trust reaches 0.9, the glyph asserts itself as
             verified fact by fusing Truth into its structure.

AFTER 100 ACCESSES:
  A=CR-54(Error)  B=CR-16(Tower)  C=CR-32(Remember)  D=CR-22(Spiral)
  The glyph now reads: "recurring memory error in server"
  It LEARNED from its own usage pattern.

AFTER TRUST 0.9:
  D=CR-22(Spiral) bind-fused with CR-57(Truth)
  The glyph now reads: "verified recurring memory error in server"
  It ASSERTED its own reliability.
```

Safety Constraints:

```
CONSTRAINT 1: DETERMINATIVE IMMUTABILITY
  Position A (determinative radical) can NEVER be modified by
  typogenetic rules. The domain classification is permanent.
  A fire glyph cannot self-modify into a water glyph.

CONSTRAINT 2: TRUST GATE
  Self-modification only fires for trust_state >= 'theory'.
  Dreams cannot self-modify — insufficient confidence.

CONSTRAINT 3: LINEAGE PRESERVATION
  Every mutation is logged in mutation_log AND lineage.
  The glyph's full history is always recoverable.

CONSTRAINT 4: GENERATION LIMIT
  Maximum 5 generations of self-modification per glyph.
  After generation 5, the glyph is "mature" — no more auto-mutation.
  Manual modification by operator is still possible.

CONSTRAINT 5: REVERSIBILITY
  Every mutation can be reversed by consulting mutation_log
  and restoring the previous radical. Non-destructive evolution.
```

Why This Matters:

Without typogenetics, the system needs an external agent to notice
that "this leak keeps happening" and manually update the glyph. WITH
typogenetics, the glyph itself notices the pattern and self-corrects.
The knowledge substrate becomes genuinely adaptive — not just stored
data, but data that learns from being used.

---

## Mechanism 12 — Fluid Analogies / Slipnet

### Source

Hofstadter's "Fluid Analogies" research (Copycat, Metacat projects)
uses a **Slipnet** — a network of concepts where nodes are connected by
**slip-links** of varying strength. When the system reasons by analogy,
concepts "slip" along these links from one meaning to a related meaning.
"Successor" slips to "predecessor." "Leftmost" slips to "rightmost."
The strength of slip determines how easy the analogy is.

### Gap in Current Spec

Tendrils connect related symbols, but there is no concept of a symbol
"slipping" to a related-but-different meaning. Analogy detection relies
on explicit radical overlap. Subtle analogies — where the relationship
is structural, not compositional — are missed entirely.

### Integration

Add **slip-links** to the tendril system — a special tendril type that
connects concepts by **analogical relationship** rather than semantic
similarity. Slip-links enable fluid reasoning across domains.

```
SLIP-LINK PROPERTIES
════════════════════

  A slip-link connects two symbols that are NOT semantically similar
  but ARE structurally analogous.

  Example:
    "Cache invalidation" ←→ "Forest fire renewal"
    These share NO radicals. But structurally:
      - Both involve destruction that enables renewal
      - Both are periodic
      - Both clear accumulated cruft

  The slip-link encodes:
    slip_from:     symbol_A (cache invalidation)
    slip_to:       symbol_B (forest fire renewal)
    slip_strength: 0.7 (strong analogy)
    slip_mapping: {
      "CR-32(Remember)" : "CR-01(Tree)",
      "CR-54(Error)"    : "CR-09(Fire)",
      "CR-25(Wilt)"     : "CR-24(Bloom)",
      "TRN-DECAY"       : "TRN-CYCLE"
    }
    slip_type: "structural_analogy"
```

**Updated Tendril Type Enum:**

```json
{
  "tendril_type": {
    "type": "string",
    "enum": [
      "association",
      "causal",
      "compositional",
      "antithetical",
      "dream_bridge",
      "slip_link"
    ]
  }
}
```

**Slip-Link Schema Addition (in Tendril):**

```json
{
  "slip_mapping": {
    "type": ["object", "null"],
    "description": "Maps radicals in source to analogous radicals in target. Keys and values are radical IDs or movement glyph IDs.",
    "additionalProperties": { "type": "string" }
  },
  "slip_strength": {
    "type": ["number", "null"],
    "minimum": 0.0,
    "maximum": 1.0,
    "description": "How easily the analogy can be followed. 1.0=obvious, 0.1=tenuous"
  }
}
```

**Schema Addition:**

```sql
ALTER TABLE tendrils ADD COLUMN slip_mapping TEXT; -- JSON object
ALTER TABLE tendrils ADD COLUMN slip_strength REAL;
```

**Slip-Link Discovery (during REM consolidation):**

```
REM CREATIVE STAGE — ANALOGY DETECTION (enhanced)
══════════════════════════════════════════════════

For each pair of unrelated symbols/dreams/theories:
  1. Extract structural pattern:
     - Grid positions filled (which of A/B/C/D)
     - Movement glyph types used
     - Transformation types used
     - Temporal phase alignment
     - AC-ratio similarity
  2. Compare structural patterns (IGNORING specific radicals)
  3. If structural similarity > 0.6:
     → Create slip-link between the two
     → Map corresponding radicals by position
     → Slip strength = structural similarity score
  4. Log as "analogical discovery"

  This is the system's primary mechanism for CREATIVE INSIGHT.
  It finds hidden structure shared between unrelated domains.
```

**Query Enhancement:**

```
LITERAL QUERY: "How do I fix cache invalidation?"
  → Returns symbols directly about cache invalidation
  → Standard radical-match results

ANALOGICAL QUERY: "What is cache invalidation LIKE?"
  → Follows slip-links from cache-invalidation symbols
  → Returns: "Cache invalidation is structurally analogous to
     forest fire renewal (slip_strength: 0.7).

     The mapping:
       cache (Remember) ↔ forest (Tree)
       invalidation (Error) ↔ fire (Fire)
       staleness (Wilt) ↔ new growth (Bloom)
       decay pattern ↔ renewal cycle

     Consider: is your cache invalidation strategy a controlled
     burn or an uncontrolled fire?"

DISCOVERY QUERY: "What unexpected connections exist?"
  → Return all slip-links with slip_strength > 0.5
  → Ranked by novelty (most recently discovered first)
  → These are the system's creative insights
```

Why This Matters:

Without slip-links, the system only finds what it's looking for —
symbols with matching radicals. WITH slip-links, the system discovers
**structural analogies across domains** that no one asked about. This
is genuine computational creativity: finding that cache invalidation
and forest fires share a deep pattern, and making that insight
available for reasoning.

---

## Mechanism 13 — Morphic Resonance

### Source

Sheldrake's morphic resonance hypothesis proposes that once a pattern
has been established anywhere, it becomes **easier for that same pattern
to occur elsewhere** — even without direct communication. Crystals that
are hard to form become easier to crystallize worldwide once they have
been done once. Rats that learn a maze make it easier for rats
everywhere to learn the same maze.

### Gap in Current Spec

When two nodes in the mesh independently develop similar symbols, there
is no mechanism for them to *find each other* without explicit
broadcast. Similar discoveries on separate nodes require deliberate
SYMBOL_BROADCAST to correlate. There is no passive discovery.

### Integration

Add a **morphic resonance field** — a passive, low-bandwidth background
signal that lets nodes detect when distant nodes have developed symbols
with similar semantic coordinates, *without* exchanging the symbols
themselves.

```
MORPHIC RESONANCE PROTOCOL
═══════════════════════════

STEP 1 — FIELD EMISSION (passive, periodic)
  Every node periodically emits a "resonance digest":
  - NOT a symbol list
  - A BLURRED DENSITY MAP of where symbols cluster in semantic space
  - No individual symbol data shared
  - Fixed size: ~500 bytes regardless of symbol count
  - Frequency: every 100 heartbeats (slow background)

STEP 2 — RESONANCE DETECTION
  When node B receives node A's digest:
    For each region in A's fingerprint:
      Compare against B's own density map
      If overlap detected (similar cluster in similar region):
        → "Morphic resonance" detected
        → B sends RESONANCE_PING to A

STEP 3 — VOLUNTARY EXCHANGE
  On receiving RESONANCE_PING:
    A can choose to share symbols in the matching region
    via standard SYMBOL_BROADCAST
    Or ignore the ping (no obligation)

  If exchange happens:
    Both nodes gain corroborating evidence
    → Independent discovery = STRONG evidence
    → Trust scores increase for matching symbols
```

**Resonance Digest Schema:**

```json
{
  "$id": "https://qrrune.local/schemas/resonance-digest.json",
  "title": "ResonanceDigest",
  "type": "object",
  "required": ["node_id", "timestamp", "semantic_fingerprint"],
  "properties": {
    "node_id":   { "type": "string" },
    "timestamp": { "type": "integer" },
    "semantic_fingerprint": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "region": {
            "type": "array",
            "items": { "type": "number" },
            "minItems": 3, "maxItems": 3,
            "description": "Centroid [sem_x, sem_y, sem_z] of cluster"
          },
          "radius":         { "type": "number",
                              "description": "Semantic radius of cluster" },
          "density":        { "type": "integer",
                              "description": "Number of symbols in this cluster" },
          "dominant_domain":{ "type": ["string", "null"],
                              "description": "Most common domain in this cluster" },
          "avg_trust":      { "type": "number",
                              "description": "Average trust score in cluster" }
        }
      },
      "description": "Blurred density map — no individual symbol data leaked"
    }
  }
}
```

**New Mesh Message Types:**

```
RESONANCE_DIGEST
  Sender:  all peers (periodic, slow — every 100 heartbeats)
  Payload: semantic fingerprint (blurred density map)
  Purpose: "Here is the general shape of my knowledge"
  Size:    ~500 bytes fixed
  TTL:     2 (neighbors + their neighbors)

RESONANCE_PING
  Sender:  peer that detected overlap
  Payload: { matching_region, my_density, request: "SYMBOL_EXCHANGE?" }
  Purpose: "We independently discovered similar things — compare notes?"
  Response: SYMBOL_BROADCAST of matching region (voluntary)
```

**Why Morphic > Broadcast:**

| Property        | Broadcast (current)           | Morphic Resonance (new)             |
|-----------------|-------------------------------|-------------------------------------|
| Data shared     | Full symbols (~2KB each)      | Density map (~500B total)           |
| Receiver load   | Process every symbol          | Process only matching regions       |
| Scaling         | O(n) with symbol count        | O(1) — fixed digest size            |
| Direction       | Push: you get everything      | Pull: you only get what resonates   |
| Serendipity     | None — you get what was sent  | High — discover unexpected overlaps |
| Privacy         | Full symbol data exposed      | Only aggregate shape exposed        |

---

## Mechanism 14 — Kin Recognition

### Source

Suzanne Simard proved that mother trees preferentially route nutrients
to *their own seedlings* over unrelated trees through the mycorrhizal
network. The fungal network enables **kin recognition** — trees can
distinguish their offspring from strangers and allocate resources
accordingly.

### Gap in Current Spec

All symbols in the galaxy are peers. A symbol's lineage is tracked
(parent_id, generation) but it has no effect on routing or resource
allocation. A newly created glyph gets no preferential treatment from
its parent. There is no concept of "family."

### Integration

Symbols that share lineage (common parent or grandparent) form **kin
clusters**. Resources flow preferentially within kin clusters, and
parent symbols actively nourish their descendants.

```
KIN RECOGNITION RULES
═════════════════════

RULE 1 — KIN DETECTION
  Two symbols are KIN if:
    - They share a lineage_parent (siblings)
    - One is the other's lineage_parent (parent-child)
    - They share a grandparent (cousins) — max depth 3
  Kin affinity = 1.0 / (lineage_distance + 1)

RULE 2 — PREFERENTIAL ROUTING
  Tendrils connecting kin symbols get a routing bonus:
    effective_diameter = actual_diameter * (1.0 + kin_affinity * 0.3)
  Kin-connected paths are preferred, all else being equal.

RULE 3 — NOURISHMENT FLOW
  During consolidation, parent symbols with trust >= 'trusted'
  transfer a trust bonus to child symbols:
    child.trust_score += parent.trust_score * 0.05
  Capped at parent's trust_score — cannot boost above self.

RULE 4 — KIN DEFENSE
  When a symbol's trust drops below 0.2 (endangered):
    Kin symbols within distance 2 receive an alert
    Kin with trust >= 0.7 can "vouch":
      → Transfer 0.05 trust to endangered kin
    Maximum 3 vouches per endangered symbol per cycle
    Prevents extinction of valid concepts from temporary disuse.

RULE 5 — KIN CLUSTER BOUNDARIES
  Kin clusters are soft boundaries, not hard partitions.
  A symbol can have kin in multiple clusters.
  Cross-cluster tendrils are NOT penalized.
```

**Schema Addition:**

```sql
CREATE TABLE IF NOT EXISTS kin_relations (
    symbol_a     TEXT    NOT NULL,
    symbol_b     TEXT    NOT NULL,
    relationship TEXT    NOT NULL,  -- parent|child|sibling|cousin
    distance     INTEGER NOT NULL,  -- lineage hops
    affinity     REAL    NOT NULL,  -- 1.0 / (distance + 1)
    PRIMARY KEY (symbol_a, symbol_b),
    FOREIGN KEY (symbol_a) REFERENCES symbols(id),
    FOREIGN KEY (symbol_b) REFERENCES symbols(id)
);

CREATE INDEX idx_kin_symbol_a ON kin_relations(symbol_a);
CREATE INDEX idx_kin_symbol_b ON kin_relations(symbol_b);
CREATE INDEX idx_kin_affinity ON kin_relations(affinity);
```

**Kin Relation JSON Schema:**

```json
{
  "$id": "https://qrrune.local/schemas/kin-relation.json",
  "title": "KinRelation",
  "type": "object",
  "required": ["symbol_a", "symbol_b", "relationship", "distance", "affinity"],
  "properties": {
    "symbol_a":     { "type": "string", "format": "uuid" },
    "symbol_b":     { "type": "string", "format": "uuid" },
    "relationship": {
      "type": "string",
      "enum": ["parent", "child", "sibling", "cousin"]
    },
    "distance": { "type": "integer", "minimum": 1, "maximum": 3 },
    "affinity":  { "type": "number",  "minimum": 0.0, "maximum": 1.0 }
  }
}
```

**Consolidation Integration:**

```
CONSOLIDATION — KIN NOURISHMENT SUBSTAGE (new)
═══════════════════════════════════════════════

Runs after:  N3 (Deep Sleep)
Runs before: Anticipation Engine

FOR each trusted/axiom symbol with children:
  FOR each child in kin_relations where relationship = 'child':
    IF child.trust_score < parent.trust_score:
      child.trust_score += parent.trust_score * 0.05
      child.trust_score = min(child.trust_score, parent.trust_score)
      LOG: "Kin nourishment: {parent} → {child} (+{delta})"

FOR each endangered symbol (trust < 0.2):
  kin = query kin_relations WHERE distance <= 2
                            AND kin.trust_score >= 0.7
  vouches_received = 0
  FOR each eligible kin:
    IF vouches_received < 3:
      endangered.trust_score += 0.05
      vouches_received++
      LOG: "Kin defense: {kin} vouched for {endangered}"
```

---

## Mechanism 15 — Mycoremediation Error Correction

### Source

Paul Stamets documented how fungi neutralize environmental toxins
(pesticides, petroleum, heavy metals) by **breaking them into harmless
components** through enzymatic decomposition. The fungus does not reject
the toxin — it *processes* it into something safe. Mycoremediation is
remediation, not rejection.

### Gap in Current Spec

When a corrupted glyph enters the system, the current behavior is
undefined. The spec mentions that corrupted glyphs are "remediated, not
rejected" but provides no mechanism for how remediation actually works.

### Integration

Define a **mycoremediation pipeline** for corrupted, malformed, or
contradictory glyphs.

```
MYCOREMEDIATION PIPELINE
═════════════════════════

INPUT: Corrupted glyph (malformed, contradictory, or injection attempt)

STAGE 1 — TOXIN DETECTION
  Scan incoming glyph for corruption markers:
  □ Invalid radical IDs (not in CR-01 through CR-60)
  □ Stroke count > 14 (over budget)
  □ Self-contradictory grid (same radical in two positions)
  □ Trust-state claiming 'axiom' without axiom registry match
  □ Sem coordinates outside [-1.0, 1.0] bounds
  □ Embedded directives in string fields (injection attempt)
  □ Halo values outside valid ranges
  □ Cyclic lineage (symbol is its own ancestor)
  IF any marker detected → route to remediation

STAGE 2 — DECOMPOSITION
  Break the corrupted glyph into constituent radicals:
  - Valid radicals → salvage pool
  - Invalid radicals → discard with log entry
  - Metadata → quarantine (do not trust halo from corrupted source)
  DO NOT discard the entire glyph. Extract every usable part.

STAGE 3 — NEUTRALIZATION
  For each salvaged radical:
  - Verify against axiom radical definitions (CR-01..CR-60)
  - If radical is valid → clean and re-register
  - If radical is close-but-wrong → attempt correction:
    Example: "CR-61" (invalid) → nearest valid = CR-60 (Infinity)
    Log correction with confidence score
  - If radical is unrecognizable → discard, log as unknown

STAGE 4 — REASSEMBLY (optional)
  If enough clean radicals were salvaged (>= 2):
  - Re-compose a new glyph from salvaged radicals
  - Assign trust_state = 'dream' (starts from zero trust)
  - Tag with remediation metadata
  - Insert into dream buffer for standard consolidation

STAGE 5 — AUDIT LOG
  Every remediation event is logged (see schema below)
```

**Remediation Log Schema:**

```sql
CREATE TABLE IF NOT EXISTS remediation_log (
    id                   TEXT    PRIMARY KEY,
    timestamp            INTEGER NOT NULL,
    corruption_type      TEXT    NOT NULL,
        -- injection_attempt: embedded directives detected
        -- malformed:         structural violations
        -- contradictory:     self-conflicting data
        -- out_of_bounds:     values outside valid ranges
        -- cyclic_lineage:    impossible ancestry
    source_node          TEXT,
    original_size_bytes  INTEGER,
    radicals_salvaged    INTEGER NOT NULL DEFAULT 0,
    radicals_discarded   INTEGER NOT NULL DEFAULT 0,
    reassembled          INTEGER NOT NULL DEFAULT 0,  -- BOOLEAN
    new_dream_id         TEXT,
    corruption_details   TEXT,  -- JSON array of violation objects
    FOREIGN KEY (new_dream_id) REFERENCES dreams(id)
);

CREATE INDEX IF NOT EXISTS idx_remediation_type ON remediation_log(corruption_type);
CREATE INDEX IF NOT EXISTS idx_remediation_time ON remediation_log(timestamp);
```

**Remediation Report JSON Schema:**

```json
{
  "$id": "https://qrrune.local/schemas/remediation-report.json",
  "title": "RemediationReport",
  "type": "object",
  "required": [
    "id", "timestamp", "corruption_type",
    "radicals_salvaged", "radicals_discarded", "reassembled"
  ],
  "properties": {
    "id":        { "type": "string", "format": "uuid" },
    "timestamp": { "type": "integer" },
    "corruption_type": {
      "type": "string",
      "enum": [
        "injection_attempt", "malformed",
        "contradictory", "out_of_bounds", "cyclic_lineage"
      ]
    },
    "source_node":          { "type": ["string", "null"] },
    "original_size_bytes":  { "type": ["integer", "null"] },
    "radicals_salvaged":    { "type": "integer", "minimum": 0 },
    "radicals_discarded":   { "type": "integer", "minimum": 0 },
    "salvaged_radical_ids": {
      "type": "array",
      "items": { "type": "string", "pattern": "^CR-[0-6][0-9]$" }
    },
    "discarded_radical_ids": {
      "type": "array",
      "items": { "type": "string" },
      "description": "May contain invalid IDs — that is the point"
    },
    "reassembled":  { "type": "boolean" },
    "new_dream_id": { "type": ["string", "null"], "format": "uuid" },
    "corruption_details": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "field":    { "type": "string" },
          "expected": { "type": "string" },
          "actual":   { "type": "string" },
          "severity": {
            "type": "string",
            "enum": ["minor", "major", "critical"]
          }
        }
      }
    }
  }
}
```

**Example — Remediation in Action:**

```
INCOMING CORRUPTED GLYPH FROM MESH:
  {
    "radicals": [
      { "id": "CR-54", ... },
      { "id": "CR-99", ... },  ← INVALID: no such radical
      { "id": "CR-32", ... },
      { "id": "CR-08", ... }
    ],
    "trust_state": "axiom",  ← SUSPICIOUS: not in axiom registry
    "sem_x": 5.7,            ← INVALID: outside [-1.0, 1.0]
    "halo": {
      "trust": 1.0,          ← SUSPICIOUS: claiming perfect trust
      "activation": 1.0
    }
  }

STAGE 1 — TOXIN DETECTION:
  [x] Invalid radical ID: CR-99
  [x] False axiom claim
  [x] sem_x out of bounds (5.7 > 1.0)
  [x] Suspicious trust/activation values
  → Classified: "injection_attempt" (severity: critical)

STAGE 2 — DECOMPOSITION:
  Salvaged: CR-54 (Error), CR-32 (Remember), CR-08 (Water)
  Discarded: CR-99 (unknown)
  Metadata: QUARANTINED (do not trust any halo values)

STAGE 3 — NEUTRALIZATION:
  CR-54 ✓ valid
  CR-32 ✓ valid
  CR-08 ✓ valid
  CR-99 → nearest match: CR-60 (Infinity)? No — distance too great.
          → Discard entirely.

STAGE 4 — REASSEMBLY:
  3 clean radicals salvaged (>= 2 threshold)
  New glyph: A=CR-54  B=(empty)  C=CR-32  D=CR-08
  trust_state = 'dream'
  trust_score = 0.15 (low — remediated source)
  remediated  = true
  → Inserted into dream buffer

STAGE 5 — AUDIT LOG:
  {
    "corruption_type": "injection_attempt",
    "source_node": "node-X",
    "radicals_salvaged": 3,
    "radicals_discarded": 1,
    "reassembled": true,
    "new_dream_id": "dream-remediated-001",
    "corruption_details": [
      { "field": "radicals[1].id",  "expected": "CR-01..CR-60",
        "actual": "CR-99",          "severity": "critical" },
      { "field": "trust_state",     "expected": "dream",
        "actual": "axiom",          "severity": "critical" },
      { "field": "sem_x",           "expected": "[-1.0, 1.0]",
        "actual": "5.7",            "severity": "major" }
    ]
  }

RESULT:
  - Corrupted glyph was NOT rejected — it was remediated
  - 3 of 4 radicals were salvaged
  - A new dream was created from the clean parts
  - The injection attempt was logged for security review
  - The system extracted value from toxic input
```

---

## Mechanism 16 — Shadow Pockets

### Source

The Archive for Research in Archetypal Symbolism (ARAS) and *The Book
of Symbols* assert that every living symbol has three dimensions:
1. **Etymological roots** — where it came from
2. **Play of opposites** — its shadow, its inverse
3. **Paradox** — the contradiction it holds within itself

A symbol without a shadow is dead. The shadow is not the enemy of
meaning — it is the *other half* of meaning.

### Gap in Current Spec

The ILE has a "three-axis interpretation" concept (etymological,
shadow/opposite, paradox) but there is no structural home for shadow
symbols. Shadows are computed on-the-fly during interpretation. They
have no persistent existence, no accumulated evidence, no trust score.
The shadow is ephemeral when it should be architectural.

### Integration

For every noun-pocket in the galaxy, create a mirrored **shadow pocket**
that contains the inversions, negations, and paradoxes of the symbols
in the primary pocket.

```
SHADOW POCKET ARCHITECTURE
═══════════════════════════

  PRIMARY POCKET               SHADOW POCKET
  ┌─────────────────┐          ┌─────────────────┐
  │  "Cache Server" │◄────────►│ "Cache Absence" │
  │  CR-32 + CR-16  │  mirror  │  CR-59 + CR-16  │
  │  trust: 0.85    │   link   │  trust: 0.60    │
  │  domain: tree   │          │  domain: tree   │
  │                 │          │                 │
  │  "Memory Leak"  │◄────────►│  "Memory Seal"  │
  │  CR-32 + CR-08  │  mirror  │  CR-32 + CR-45  │
  │  trust: 0.78    │          │  trust: 0.55    │
  └─────────────────┘          └─────────────────┘

  Shadow symbols are constructed by:
  1. Horizontal mirror of key radical (inversion)
  2. Substitution with antithetical radical
  3. Negation via CR-59 (Null) prefix

  Shadow pockets have their own trust scores,
  tendrils, and lifecycle — they are REAL pockets,
  not computed projections.
```

**Shadow Construction Rules:**

```
RULE 1 — AUTOMATIC SHADOW GENERATION
  When a symbol reaches trust_state = 'trusted':
    Generate its shadow symbol automatically
    Shadow trust_state = 'theory' (must earn its own trust)
    Insert into the corresponding shadow pocket

RULE 2 — SHADOW RADICAL MAPPING

  Primary → Shadow    Relationship
  ────────────────────────────────────────────
  CR-01 Tree   → CR-09 Fire    Growth ↔ Destruction
  CR-08 Water  → CR-09 Fire    Flow   ↔ Consumption
  CR-32 Remember→CR-40 Release Store  ↔ Forget
  CR-34 Create → CR-25 Wilt    Build  ↔ Decay
  CR-44 Bond   → CR-27 Split   Connect↔ Sever
  CR-45 Guard  → CR-54 Error   Protect↔ Breach
  CR-57 Truth  → CR-58 Unknown Verified↔Uncertain
  CR-41 Parent → CR-42 Child   Above  ↔ Below
  CR-24 Bloom  → CR-25 Wilt    Expand ↔ Contract
  CR-39 Focus  → CR-30 Drift   Attend ↔ Wander

  If no canonical shadow exists for a radical:
    Use CR-59 (Null) as universal shadow prefix
    "The absence of X"

RULE 3 — PARADOX SYMBOLS
  When a symbol AND its shadow both reach trust >= 0.7:
    Generate a PARADOX symbol that contains BOTH
    Grid: A=primary_determinative  B=shadow_determinative
          C=CR-44(Bond)            D=CR-28(Twist)
    Meaning: "The union of X and not-X"
    These are the deepest, most powerful symbols in the system.
    They encode the ARAS principle that paradox IS meaning.

RULE 4 — SHADOW POCKET ROUTING
  Queries can explicitly request shadow results:
    query("What is NOT cache?")             → search shadow pockets
    query("What is the opposite of growth?") → shadow lookup
    query("What paradox does fire contain?") → paradox symbols
```

**Schema Additions:**

```sql
ALTER TABLE symbols ADD COLUMN is_shadow      BOOLEAN DEFAULT FALSE;
ALTER TABLE symbols ADD COLUMN shadow_of      TEXT;   -- primary symbol ID
ALTER TABLE symbols ADD COLUMN is_paradox     BOOLEAN DEFAULT FALSE;
ALTER TABLE symbols ADD COLUMN paradox_primary TEXT;  -- primary ID
ALTER TABLE symbols ADD COLUMN paradox_shadow  TEXT;  -- shadow ID

CREATE INDEX idx_symbols_shadow    ON symbols(is_shadow)  WHERE is_shadow = TRUE;
CREATE INDEX idx_symbols_shadow_of ON symbols(shadow_of)  WHERE shadow_of IS NOT NULL;
CREATE INDEX idx_symbols_paradox   ON symbols(is_paradox) WHERE is_paradox = TRUE;
```

**JSON Schema Additions (in EncodedSymbol):**

```json
{
  "is_shadow": {
    "type": "boolean", "default": false,
    "description": "True if this symbol is the shadow/inverse of another"
  },
  "shadow_of": {
    "type": ["string", "null"], "format": "uuid",
    "description": "ID of the primary symbol this is the shadow of"
  },
  "shadow_id": {
    "type": ["string", "null"], "format": "uuid",
    "description": "ID of this symbol's shadow (if generated)"
  },
  "is_paradox": {
    "type": "boolean", "default": false,
    "description": "True if this symbol encodes the union of a primary and its shadow"
  },
  "paradox_sources": {
    "type": ["object", "null"],
    "properties": {
      "primary_id": { "type": "string", "format": "uuid" },
      "shadow_id":  { "type": "string", "format": "uuid" }
    },
    "description": "The primary and shadow symbols that generated this paradox"
  }
}
```

Why This Matters:

Without shadow pockets, the system only knows what things ARE. With
them, it also knows what things ARE NOT — and more powerfully, it knows
the PARADOXES that arise when being and not-being coexist. "Fire
destroys trees yet fire renews forests" is not a contradiction to be
resolved — it is a paradox symbol to be preserved. The system gains
depth, nuance, and the capacity for dialectical reasoning.

---

## Mechanism 17 — Kami Threshold Emergence

### Source

In Shinto ontology, kami is not a fixed category — it is a **quality
that any entity can possess** when it exceeds a threshold of
extraordinariness. A waterfall can be kami. A sword can be kami. A
person can be kami. Motoori Norinaga defined it: "Anything possessing
eminent quality out of the ordinary and awe-inspiring is called kami."

Kami-nature is **emergent**, not assigned.

### Gap in Current Spec

The spec mentions kami as a metaphor and maps it to the `sacredness`
AEL channel. But there is no system-level event triggered when a symbol
crosses the kami threshold. Nothing *happens* when a symbol becomes
extraordinary — it just has a high number in a field.

### Integration

Define the **kami emergence event** — a system-level event triggered
when a symbol's combined metadata crosses a computed threshold. When a
symbol becomes kami, the system responds with specific architectural
consequences.

```
KAMI THRESHOLD COMPUTATION
══════════════════════════

  kami_score = (
    trust_score          * 0.25 +
    activation           * 0.20 +
    connectivity_norm    * 0.20 +
    (1.0 - decay_inverted) * 0.15 +
    ael.sacredness       * 0.10 +
    ael.novelty          * 0.10
  )

  WHERE:
    connectivity_norm  = min(1.0, connectivity / caern_threshold)
    decay_inverted     = 1.0 - decay_score (fresh symbols score higher)

  KAMI THRESHOLD: kami_score >= 0.85
  When crossed → KAMI EMERGENCE EVENT fires
```

**Kami Emergence Event Consequences:**

```
WHEN a symbol crosses kami_score >= 0.85:

  1. VISUAL TRANSFORMATION
     halo_glow → 1.0   (maximum luminance)
     color.saturation → 1.0   (pure color)
     brightness → 1.0   (full visibility)

  2. DECAY IMMUNITY
     decay_score frozen at 1.0 — cannot decay while kami.

  3. CAERN PROMOTION
     If not already a caern → automatically promoted to caern.
     Becomes a hub node in the knowledge graph.

  4. SHADOW ACTIVATION
     Shadow symbol (if it exists) is auto-activated.
     Kami presence awakens its opposite — light casts shadow.

  5. TENDRIL AMPLIFICATION
     All tendrils connected to this symbol: diameter *= 1.5

  6. NOTIFICATION
     System emits KAMI_EMERGENCE mesh message:
     {
       "type": "KAMI_EMERGENCE",
       "symbol_id": "uuid",
       "kami_score": 0.89,
       "domain": "fungi",
       "timestamp": 1744400000
     }

  7. SUPERIMPLICATE CHECK
     Overwatch evaluates alignment with superimplicate axioms.
     If aligned → confirmed kami (stable).
     If misaligned → flagged for review (kami can be revoked).
```

**Kami Revocation:**

```
Kami status is maintained as long as:
  kami_score >= 0.80 (hysteresis: 0.85 to enter, 0.80 to exit)

If kami_score drops below 0.80:
  → KAMI_FADING event
  → Visual transformations reverse (gradual, over 10 cycles)
  → Decay immunity lifted
  → Tendril boost reversed
  → Caern status retained (earned independently)
  → Shadow returns to previous activation state
```

**Schema Additions:**

```sql
ALTER TABLE symbols ADD COLUMN kami_score      REAL    DEFAULT 0.0;
ALTER TABLE symbols ADD COLUMN is_kami         BOOLEAN DEFAULT FALSE;
ALTER TABLE symbols ADD COLUMN kami_emerged_at INTEGER;
ALTER TABLE symbols ADD COLUMN kami_faded_at   INTEGER;

CREATE INDEX idx_symbols_kami ON symbols(is_kami) WHERE is_kami = TRUE;
```

**JSON Schema Additions:**

```json
{
  "kami_score": {
    "type": "number", "minimum": 0.0, "maximum": 1.0, "default": 0.0,
    "description": "Composite score. >= 0.85 triggers kami emergence."
  },
  "is_kami": {
    "type": "boolean", "default": false,
    "description": "True while kami_score >= 0.80 (hysteresis band)"
  },
  "kami_emerged_at": {
    "type": ["integer", "null"],
    "description": "Timestamp of most recent kami emergence"
  },
  "kami_faded_at": {
    "type": ["integer", "null"],
    "description": "Timestamp of most recent kami fading"
  }
}
```

---

## Mechanism 18 — Holographic Fragment Reconstruction

### Source

Karl Pribram's holonomic brain theory proposes that memory is stored
holographically — any piece of the hologram contains the whole image,
at reduced resolution. Cut a holographic plate in half and each half
still shows the complete image, just blurrier. Memory is distributed,
not localized. Damage degrades gracefully rather than catastrophically.

### Gap in Current Spec

The pocket-galaxy is described as "holographic" but the implementation
is standard database storage. If a pocket is lost, its symbols are
gone. There is no mechanism for reconstructing lost symbols from
surviving fragments elsewhere in the galaxy.

### Integration

Implement **holographic redundancy** — every symbol's semantic
signature is distributed across multiple pockets as compressed
fragments. If the primary pocket is lost, the symbol can be
reconstructed from fragments at reduced fidelity.

```
HOLOGRAPHIC STORAGE MODEL
═════════════════════════

  When a symbol reaches trust_state = 'trusted':
    1. Compute its semantic signature:
       sig = [sem_x, sem_y, sem_z, ac_ratio, domain,
              radical_ids, fractal_depth]

    2. Compress signature to FRAGMENT (lossy):
       fragment = {
         sem_centroid:       [sem_x, sem_y, sem_z],  // 3 floats
         domain:             domain,                  // 1 enum
         radical_hash:       hash(radical_ids),       // 1 uint32
         ac_ratio_quantized: round(ac_ratio*4)/4      // 1 byte
       }
       Size: ~20 bytes (vs ~2KB full symbol)

    3. Distribute fragment to N neighbor pockets:
       N = min(5, number_of_neighbor_pockets)

    4. Fragment tagged with:
       origin_pocket_id, origin_symbol_id, fragment_fidelity
```

**Fragment Table Schema:**

```sql
CREATE TABLE IF NOT EXISTS fragments (
    id                TEXT    PRIMARY KEY,
    origin_symbol_id  TEXT    NOT NULL,
    origin_pocket_id  TEXT    NOT NULL,
    sem_x             REAL    NOT NULL,
    sem_y             REAL    NOT NULL,
    sem_z             REAL    NOT NULL,
    domain            TEXT,
    radical_hash      INTEGER NOT NULL,
    ac_ratio_q        REAL    NOT NULL,
    fragment_fidelity REAL    NOT NULL DEFAULT 0.5,
    created_at        INTEGER NOT NULL,
    verified          INTEGER NOT NULL DEFAULT 0  -- BOOLEAN
);

CREATE INDEX IF NOT EXISTS idx_fragments_origin ON fragments(origin_symbol_id);
CREATE INDEX IF NOT EXISTS idx_fragments_sem    ON fragments(sem_x, sem_y, sem_z);
CREATE INDEX IF NOT EXISTS idx_fragments_domain ON fragments(domain);
```

**Reconstruction Algorithm:**

```
SCENARIO: Pocket P is lost (node crash / corruption)

STEP 1 — DETECT LOSS
  Heartbeat from P missed for 3 cycles OR explicit DECAY_NOTICE.

STEP 2 — GATHER FRAGMENTS
  Query all neighbor pockets: fragments WHERE origin_pocket_id = P

STEP 3 — RECONSTRUCT
  fragments_found.count >= 3 → FULL: trust_state = 'theory'
  fragments_found.count 1-2  → PARTIAL: trust_state = 'dream'
  fragments_found.count == 0 → LOST: emit LOSS_ALERT if kami/caern

STEP 4 — VERIFY
  Cross-check against displacement vectors (Mech 6),
  kin relations (Mech 14), inter-radical relations (Mech 7).
  Each verification increases trust.
```

**JSON Schema:**

```json
{
  "$id": "https://qrrune.local/schemas/fragment.json",
  "title": "HolographicFragment",
  "type": "object",
  "required": [
    "id", "origin_symbol_id", "origin_pocket_id",
    "sem_x", "sem_y", "sem_z",
    "radical_hash", "ac_ratio_q", "fragment_fidelity", "created_at"
  ],
  "properties": {
    "id":               { "type": "string", "format": "uuid" },
    "origin_symbol_id": { "type": "string", "format": "uuid" },
    "origin_pocket_id": { "type": "string" },
    "sem_x":  { "type": "number" },
    "sem_y":  { "type": "number" },
    "sem_z":  { "type": "number" },
    "domain": { "type": ["string", "null"] },
    "radical_hash":      { "type": "integer" },
    "ac_ratio_q":        { "type": "number" },
    "fragment_fidelity": { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "created_at":        { "type": "integer" },
    "verified":          { "type": "boolean", "default": false }
  }
}
```

---

## Mechanism 19 — Token-to-Tablet Compression Pipeline

### Source

The historical evolution of writing from Sumerian clay tokens (8000 BCE)
through envelope tokens (3500 BCE) to impressed tablets (3200 BCE) to
abstract cuneiform (2600 BCE) represents a **four-stage compression
pipeline**. Concrete physical objects became abstract marks through
millennia of optimization. Each stage lost visual detail but gained
processing speed and storage density.

### Gap in Current Spec

Glyphs have a `compression_q` value and informal references to "formal
vs compact" variants, but there is no defined pipeline for how a glyph
progresses through compression stages. The system has no concept of a
glyph existing at multiple fidelity levels simultaneously.

### Integration

Define a **four-stage compression pipeline** mirroring the historical
token-to-cuneiform evolution. Every glyph can exist at multiple
compression levels. The appropriate level is selected based on context.

```
COMPRESSION PIPELINE
════════════════════

STAGE 1 — TOKEN (full fidelity)        compression_q: 1.0  ~2-4 KB
  Complete glyph: all 14 dimensions, full halo, full AEL,
  full lineage, typogenetic program, all strokes rendered.
  Use: primary storage, detailed analysis, glyph editing.

STAGE 2 — ENVELOPE (structural)        compression_q: 0.7  ~200-500 B
  radical IDs + grid + sem coordinates + trust_state + domain + ac_ratio.
  Dropped: strokes, full halo, AEL, typogenetics, lineage detail.
  Use: inter-pocket routing, query results, working memory.

STAGE 3 — TABLET (compressed ref)      compression_q: 0.4  ~50-100 B
  radical_hash + sem_centroid + domain + trust_state.
  Dropped: individual IDs, grid positions, all metadata.
  Use: index entries, density maps, resonance digests.

STAGE 4 — CUNEIFORM (maximum)          compression_q: 0.1  8 B
  Single 64-bit hash: domain + sem_octant + trust_tier.
  Use: bloom filters, existence checks, routing table keys.
```

**Multi-Level Storage:**

```
Every trusted symbol is stored at ALL FOUR levels simultaneously:
  symbols table:   TOKEN level    (full glyph)
  envelope_cache:  ENVELOPE level (working memory)
  index entries:   TABLET level   (search index)
  cuneiform_index: CUNEIFORM level (existence check)

Query routing:
  "Does this exist?"    → CUNEIFORM  (8 bytes, instant)
  "What domain is it?"  → TABLET     (50 bytes, fast)
  "Show me summary"     → ENVELOPE   (200 bytes, quick)
  "Give me everything"  → TOKEN      (2KB, full decode)
```

**Schema Addition:**

```sql
-- Envelope cache for working memory
CREATE TABLE IF NOT EXISTS envelope_cache (
    symbol_id   TEXT    PRIMARY KEY,
    radicals    TEXT    NOT NULL,  -- JSON array of radical IDs
    grid        TEXT    NOT NULL,  -- JSON grid positions
    sem_x       REAL    NOT NULL,
    sem_y       REAL    NOT NULL,
    sem_z       REAL    NOT NULL,
    trust_state TEXT    NOT NULL,
    domain      TEXT,
    ac_ratio    REAL    NOT NULL,
    cached_at   INTEGER NOT NULL,
    FOREIGN KEY (symbol_id) REFERENCES symbols(id)
);

-- Cuneiform bloom filter entries
CREATE TABLE IF NOT EXISTS cuneiform_index (
    hash_key    INTEGER PRIMARY KEY,  -- 64-bit hash
    symbol_id   TEXT    NOT NULL,
    domain_code INTEGER NOT NULL,     -- domain enum as int
    sem_octant  INTEGER NOT NULL,     -- octant of sem space (0–7)
    trust_tier  INTEGER NOT NULL,     -- 0=dream 1=theory 2=trusted 3=axiom
    FOREIGN KEY (symbol_id) REFERENCES symbols(id)
);

CREATE INDEX IF NOT EXISTS idx_cuneiform_domain ON cuneiform_index(domain_code);
CREATE INDEX IF NOT EXISTS idx_cuneiform_octant ON cuneiform_index(sem_octant);
```

Why This Matters:

Without the compression pipeline, every query fetches a full TOKEN-
level glyph (2KB+) even when the answer is "yes, it exists" (8 bytes).
With the pipeline, the system serves the right fidelity for the right
question — mirroring how human memory uses compressed representations
rather than total recall for every access.

---

---

## Enhanced 8-Stage Consolidation Cycle

The original 4-stage consolidation cycle (N1, N3, REM, Pruning) expands
to **8 stages** with Mechanisms 1–19 integrated.

```
STAGE 1 — LIGHT SLEEP (N1)
  Scan dream buffer for promotion candidates.
  NEW (M3 Soma-Significance): each scanned dream gets activation += 0.3.

STAGE 2 — DEEP SLEEP (N3)
  Promote candidates to theories.
  NEW (M1 Enfoldment): assign depth 8–11 to dream promotions, 4–7 to theories.
  NEW (M7 Compositionality): compute inter-radical relations.
  NEW (M18 Fragments): cross-reference holographic fragments.

STAGE 3 — KIN NOURISHMENT (M14)
  parent.trust transfers 5% to child symbols (capped at parent trust).
  Kin defense: up to 3 vouches for endangered symbols (trust < 0.2).

STAGE 4 — ANTICIPATION (M9)
  4a: Detect rhythms in query log (autocorrelation, confidence > 0.7).
  4b: Pre-warm pockets + symbols approaching next_predicted window.
  4c: Phantom evaluation — missing expected events generate dreams.

STAGE 5 — REM CREATIVE (original + M12, M16)
  5a: Dream recombination (original bind-rune fusion).
  5b: Structural analogy detection → slip-link tendrils (M12).
  5c: Shadow generation for newly trusted symbols (M16).
       Paradox symbol when primary + shadow both reach trust >= 0.7.

STAGE 6 — TYPOGENETIC EVALUATION (M11)
  For each symbol with typogenetic_program.enabled = true:
    Evaluate conditions, fire mutations (max generation 5).
    SAFETY: position A immutable, trust >= theory required.

STAGE 7 — KAMI EVALUATION (M17)
  Compute kami_score per trusted/axiom symbol.
  Emergence: score >= 0.85 → freeze decay, amplify tendrils, emit event.
  Fading: score < 0.80 → gradual reversal over 10 cycles.

STAGE 8 — PRUNING & FRAGMENT DISTRIBUTION
  8a: Habituation check (M10) — mute over-accessed, reset dishabituated.
  8b: Decay tick — exponential by trust tier.
  8c: Fragment distribution to neighbor pockets (M18).
  8d: Compression cache update — envelope + cuneiform (M19).
  8e: Tendril atrophy — diameter *= 0.995; prune < 0.005 (M8).
  8f: Dream expiry, symbol pruning, radical recycling.
  8g: Health snapshot.
```

---

## Consolidated SQL Migration (Mechanisms 1–19)

```sql
-- ============================================================
-- RQ^R2 POCKET-GALAXY INTEGRATION — CONSOLIDATED MIGRATION
-- Version: 1.0.0
-- ============================================================
BEGIN TRANSACTION;

-- Mechanism 1: Ink-Drop Enfoldment
ALTER TABLE symbols ADD COLUMN enfoldment_depth INTEGER DEFAULT 0;
CREATE INDEX idx_symbols_enfoldment ON symbols(enfoldment_depth);

-- Mechanism 4: Superimplicate Order
ALTER TABLE symbols ADD COLUMN is_superimplicate BOOLEAN DEFAULT FALSE;
ALTER TABLE symbols ADD COLUMN governs_process TEXT;
CREATE INDEX idx_symbols_superimplicate ON symbols(is_superimplicate)
  WHERE is_superimplicate = TRUE;

-- Mechanism 6: Displacement Cells
ALTER TABLE tendrils ADD COLUMN displacement_x REAL;
ALTER TABLE tendrils ADD COLUMN displacement_y REAL;
ALTER TABLE tendrils ADD COLUMN displacement_z REAL;

-- Mechanism 8: Tube-Diameter Memory (rename weight → diameter)
ALTER TABLE tendrils RENAME COLUMN weight TO diameter;

-- Mechanism 10: Habituation
ALTER TABLE symbols ADD COLUMN habituation_count     INTEGER DEFAULT 0;
ALTER TABLE symbols ADD COLUMN habituation_threshold INTEGER DEFAULT 50;
ALTER TABLE symbols ADD COLUMN habituated_at         INTEGER;

-- Mechanism 12: Fluid Analogies / Slipnet
ALTER TABLE tendrils ADD COLUMN slip_mapping  TEXT;
ALTER TABLE tendrils ADD COLUMN slip_strength REAL;

-- Mechanism 16: Shadow Pockets
ALTER TABLE symbols ADD COLUMN is_shadow       BOOLEAN DEFAULT FALSE;
ALTER TABLE symbols ADD COLUMN shadow_of       TEXT;
ALTER TABLE symbols ADD COLUMN shadow_id       TEXT;
ALTER TABLE symbols ADD COLUMN is_paradox      BOOLEAN DEFAULT FALSE;
ALTER TABLE symbols ADD COLUMN paradox_primary TEXT;
ALTER TABLE symbols ADD COLUMN paradox_shadow  TEXT;
CREATE INDEX idx_symbols_shadow    ON symbols(is_shadow)  WHERE is_shadow = TRUE;
CREATE INDEX idx_symbols_shadow_of ON symbols(shadow_of)  WHERE shadow_of IS NOT NULL;
CREATE INDEX idx_symbols_paradox   ON symbols(is_paradox) WHERE is_paradox = TRUE;

-- Mechanism 17: Kami Threshold Emergence
ALTER TABLE symbols ADD COLUMN kami_score      REAL    DEFAULT 0.0;
ALTER TABLE symbols ADD COLUMN is_kami         BOOLEAN DEFAULT FALSE;
ALTER TABLE symbols ADD COLUMN kami_emerged_at INTEGER;
ALTER TABLE symbols ADD COLUMN kami_faded_at   INTEGER;
CREATE INDEX idx_symbols_kami ON symbols(is_kami) WHERE is_kami = TRUE;

-- New tables: rhythms, kin_relations, remediation_log,
--             fragments, envelope_cache, cuneiform_index
-- (see brain_system.cpp ensure_schema for full CREATE TABLE statements)

COMMIT;
```

---

## Integration Wiring Summary

| # | Mechanism | Modifies | New Tables | Consol. Stage |
|---|---|---|---|---|
| 1 | Ink-Drop Enfoldment | symbols | — | N3 |
| 2 | Rheomode / Verb-Pockets | pocket model | verb_pockets | — |
| 3 | Soma-Significance | ILE pipeline | — | N1 |
| 4 | Superimplicate Order | symbols | — | Kami eval |
| 5 | Pilot Wave | mesh routing | — | — |
| 6 | Displacement Cells | tendrils | — | — |
| 7 | Object Compositionality | grid schema | — | N3 |
| 8 | Tube-Diameter | tendrils | — | Pruning |
| 9 | Anticipatory Behavior | query pipeline | rhythms | Anticipation |
| 10 | Habituation | symbols, AEL | — | Pruning |
| 11 | Typogenetics | symbols | — | Typogenetic |
| 12 | Fluid Analogies | tendrils | — | REM |
| 13 | Morphic Resonance | mesh discovery | — | — |
| 14 | Kin Recognition | routing, trust | kin_relations | Kin nourish |
| 15 | Mycoremediation | error handling | remediation_log | — |
| 16 | Shadow Pockets | symbols | — | REM |
| 17 | Kami Threshold | symbols, mesh | — | Kami eval |
| 18 | Holographic Fragments | fault tolerance | fragments | N3, Pruning |
| 19 | Token-to-Tablet | compression | envelope_cache, cuneiform_index | Pruning |

**Totals:**
- Columns added to `symbols`: 15
- Columns added to `tendrils`: 6 (+ rename `weight` → `diameter`)
- New tables: 6 (`rhythms`, `kin_relations`, `remediation_log`, `fragments`, `envelope_cache`, `cuneiform_index`, `verb_pockets`)
- New indexes: 18
- New C ABI functions: 34
- New REST endpoints: 30+
- New mesh message types: 5 (`PILOT_SIGNAL`, `RESONANCE_DIGEST`, `RESONANCE_PING`, `KAMI_EMERGENCE`, `KAMI_FADING`)
- Consolidation stages: 4 → 8

---

## Related documents

- [03 — RQ^R2 Encoder Module](03-rqr2-module.md)
- [02 — Agent Framework](02-agent-framework.md)
- [07 — Storage Layer](07-storage.md)
- [06 — Mesh Networking](06-mesh-networking.md)
