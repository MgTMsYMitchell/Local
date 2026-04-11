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

## Related documents

- [03 — RQ^R2 Encoder Module](03-rqr2-module.md)
- [02 — Agent Framework](02-agent-framework.md)
- [07 — Storage Layer](07-storage.md)
- [06 — Mesh Networking](06-mesh-networking.md)
