# 03 — RQ^R2 Encoder Module

> Path: docs/architecture/03-rqr2-module.md
> Version: 1.0.0-draft
> Author: Mitchell Turchyniak
> License: Proprietary — All Rights Reserved
> Status: Living Document — Canonical Reference
> Last Updated: 2026-04-11

---

## Table of Contents

- [0. Where This Module Lives](#0-where-this-module-lives)
- [1. Purpose & Scope](#1-purpose--scope)
- [2. Architecture Integration](#2-architecture-integration)
- [3. Dream-State Memory Architecture](#3-dream-state-memory-architecture)
- [4. The 14-Dimensional Encoding Space](#4-the-14-dimensional-encoding-space)
- [5. Radical Composition Engine](#5-radical-composition-engine)
- [6. Memory Pipeline — Sensory to Permanent](#6-memory-pipeline--sensory-to-permanent)
- [7. Trust Escalation Protocol](#7-trust-escalation-protocol)
- [8. Glyph Construction Engine](#8-glyph-construction-engine)
- [9. Angular vs Curved Partitioning](#9-angular-vs-curved-partitioning)
- [10. World-Entity Ontology](#10-world-entity-ontology)
- [11. Morphology Layers](#11-morphology-layers)
- [12. ILE — Interpretation Layer Engine](#12-ile--interpretation-layer-engine)
- [13. AEL — Affective-Emotive Overlays](#13-ael--affective-emotive-overlays)
- [14. Mycelium Routing Layer](#14-mycelium-routing-layer)
- [15. Brain-Inspired Cognition Model](#15-brain-inspired-cognition-model)
- [16. Pocket-Galaxy Knowledge Model](#16-pocket-galaxy-knowledge-model)
- [17. Metadata Channels](#17-metadata-channels)
- [18. Distributed Mesh Integration](#18-distributed-mesh-integration)
- [19. JSON Schemas](#19-json-schemas)
- [20. API Surface](#20-api-surface)
- [21. Example Glyph Encodings](#21-example-glyph-encodings)
- [22. Appendix](#22-appendix--dream-vs-theory-vs-trusted-examples)

---

## 0. Where This Module Lives

```
QRune Cognitive Node (rune_brain :7071)
├── LibrarianAgent
│   ├── classify_symbol events → BrainDb::insert_dream() → dreams table
│   ├── symbols_query()        → GET /brain/symbols
│   └── QuantumFractalSystem (QFS)
│       └── RQ^R2 Encoder Library  ◄── THIS MODULE
│           ├── encode()        — arbitrary input → 14D symbol
│           ├── decode()        — 14D symbol → semantic output
│           ├── compose()       — radicals → compound glyph
│           ├── dream_ingest()  — unverified fragment → dream buffer
│           ├── consolidate()   — dream → theory promotion
│           └── trust_escalate()— theory → trusted promotion
│
├── BrainDb (SQLite)
│   ├── symbols   — 14D encoded glyphs (trusted + axiom)
│   ├── dreams    — unverified/hallucinated fragments
│   ├── theories  — promoted dreams under review
│   ├── radicals  — atomic component registry (CR-01 → CR-60)
│   ├── tendrils  — mycelium connections between symbols
│   └── lineage   — glyph ancestry (carried in halo_json)
│
└── Mesh Overlay (Yggdrasil)
    ├── node discovery
    ├── tendril routing
    └── consensus voting
```

**Bottom line:** QFS is QRune's theoretical parent. This document specifies the
fractal/quantum encoding engine (RQ^R2) — the standalone encoder library that
`rune_brain` calls to encode arbitrary inputs into 14D symbols, which
`LibrarianAgent` then stores and routes. Everything in `rune_brain` is already
wired and waiting for this.

---

## 1. Purpose & Scope

### 1.1 What RQ^R2 Does

RQ^R2 (Radical Query, Runic Quantum — squared) is the encoding engine that
transforms arbitrary semantic inputs into 14-dimensional cognitive symbols.

It owns:

| Responsibility | Description |
|---|---|
| Fractal depth calculation | Determines how many recursive layers of meaning a symbol carries |
| Semantic embedding | Computes `sem_x`, `sem_y`, `sem_z` coordinates in meaning-space |
| Compression selection | Chooses optimal `compression_q` for bandwidth vs fidelity |
| Radical decomposition | Breaks inputs into atomic radical components |
| Glyph composition | Assembles radicals into compound glyphs following construction rules |
| Dream classification | Tags unverified/hallucinated encodings as dreams, not facts |
| Trust scoring | Manages the dream → theory → trusted escalation pipeline |

### 1.2 What RQ^R2 Does NOT Do

- **Storage** — BrainDb owns persistence
- **Routing** — LibrarianAgent and the mycelium layer own routing
- **Display** — UI rendering is downstream
- **Agent orchestration** — Overwatch handles inter-agent coordination

### 1.3 Design Principles

```
1. ENCODE EVERYTHING    — No input is rejected; unclassifiable input enters as dream
2. TRUST NOTHING        — All new encodings start at dream-state until promoted
3. COMPRESS RUTHLESSLY  — Maximize meaning per stroke, meaning per byte
4. DECOMPOSE GRACEFULLY — Corrupted glyphs are remediated, not rejected
5. DREAM PRODUCTIVELY   — Hallucinations are raw material, not waste product
6. REMEMBER BIOLOGICALLY— Memory follows human sensory→short→long pipeline
7. LOOP STRANGELY       — Self-reference is a feature, not a bug
```

---

## 2. Architecture Integration

### 2.1 Call Flow

```
External Input (text, signal, sensor, agent message)
       │
       ▼
┌──────────────────────────────┐
│    RQ^R2 Encoder Library     │
│                              │
│  1. Tokenize input           │
│  2. Match radicals           │
│  3. Calculate fractal_depth  │
│  4. Compute sem_x/y/z        │
│  5. Select compression_q     │
│  6. Compose glyph            │
│  7. Assign trust_state       │ ← DREAM by default
│  8. Attach metadata halo     │
│  9. Return EncodedSymbol     │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│       LibrarianAgent         │
│                              │
│  • classify_symbol(symbol)   │
│  • route to relevant agents  │
│  • store via BrainDb         │
│  • index by radical keys     │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│           BrainDb            │
│                              │
│  INSERT INTO dreams (        │
│    id, raw_input,            │
│    radicals_guess, sem_x,    │
│    sem_y, sem_z, confidence, │
│    dream_type, created_at    │
│  )                           │
└──────────────────────────────┘
```

### 2.2 Symbols Table Schema (Extended)

```sql
CREATE TABLE IF NOT EXISTS symbols (
    -- original 14D columns (preserved)
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    radical        TEXT    NOT NULL DEFAULT '',
    layer          TEXT    NOT NULL DEFAULT 'SYNTHETIC',
    sem_x          REAL    NOT NULL DEFAULT 0.0,
    sem_y          REAL    NOT NULL DEFAULT 0.0,
    sem_z          REAL    NOT NULL DEFAULT 0.0,
    color_h        REAL    NOT NULL DEFAULT 0.0,
    color_s        REAL    NOT NULL DEFAULT 0.0,
    color_b        REAL    NOT NULL DEFAULT 0.0,
    temporal_phase REAL    NOT NULL DEFAULT 0.0,
    affinity_mask  INTEGER NOT NULL DEFAULT 0,
    mutation_index INTEGER NOT NULL DEFAULT 0,
    stroke_count   INTEGER NOT NULL DEFAULT 0,
    fractal_depth  INTEGER NOT NULL DEFAULT 1,
    compression_q  TEXT    NOT NULL DEFAULT 'LOSSLESS',
    payload        TEXT    NOT NULL DEFAULT '{}',
    checksum       TEXT    NOT NULL DEFAULT '',
    version        INTEGER NOT NULL DEFAULT 1,
    created_at     TEXT    DEFAULT (datetime('now')),
    updated_at     TEXT    DEFAULT (datetime('now')),
    -- RQ^R2 trust + halo columns (added via migration)
    trust_state    TEXT    NOT NULL DEFAULT 'dream',
    trust_score    REAL    NOT NULL DEFAULT 0.0,
    ac_ratio       REAL    NOT NULL DEFAULT 0.5,
    domain         TEXT,
    halo_json      TEXT    NOT NULL DEFAULT '{}',
    lineage_parent INTEGER,
    dream_source   INTEGER,
    promoted_at    TEXT,
    accessed_at    TEXT,
    access_count   INTEGER DEFAULT 0,
    decay_score    REAL    DEFAULT 1.0
);
```

### 2.3 Dreams Table Schema

```sql
CREATE TABLE IF NOT EXISTS dreams (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    raw_input           TEXT    NOT NULL,
    fragment_data       TEXT,
    radicals_guess      TEXT    NOT NULL DEFAULT '[]',
    sem_x               REAL,
    sem_y               REAL,
    sem_z               REAL,
    confidence          REAL    NOT NULL DEFAULT 0.0,
    source_agent        TEXT    NOT NULL DEFAULT '',
    source_context      TEXT,
    dream_type          TEXT    NOT NULL DEFAULT 'fragment',
    consolidation_count INTEGER DEFAULT 0,
    last_consolidated   TEXT,
    promoted_to         INTEGER,
    rejected_at         TEXT,
    rejection_reason    TEXT,
    created_at          TEXT    DEFAULT (datetime('now')),
    expires_at          TEXT
);
```

`dream_type` values: `fragment` | `hallucination` | `confabulation` | `hypnagogic` | `lucid`

### 2.4 Theories Table Schema

```sql
CREATE TABLE IF NOT EXISTS theories (
    id                    INTEGER PRIMARY KEY AUTOINCREMENT,
    dream_origin          INTEGER NOT NULL,
    glyph_data            TEXT    NOT NULL DEFAULT '{}',
    radicals              TEXT    NOT NULL DEFAULT '[]',
    sem_x                 REAL    NOT NULL DEFAULT 0.0,
    sem_y                 REAL    NOT NULL DEFAULT 0.0,
    sem_z                 REAL    NOT NULL DEFAULT 0.0,
    fractal_depth         INTEGER NOT NULL DEFAULT 1,
    compression_q         REAL    NOT NULL DEFAULT 0.5,
    hypothesis            TEXT    NOT NULL,
    supporting_evidence   TEXT,
    contradicting_evidence TEXT,
    test_count            INTEGER DEFAULT 0,
    pass_count            INTEGER DEFAULT 0,
    fail_count            INTEGER DEFAULT 0,
    trust_score           REAL    NOT NULL DEFAULT 0.3,
    promotion_threshold   REAL    DEFAULT 0.75,
    status                TEXT    NOT NULL DEFAULT 'active',
    created_at            TEXT    DEFAULT (datetime('now')),
    last_tested           TEXT,
    promoted_at           TEXT
);
```

`status` values: `active` | `supported` | `contested` | `promoted` | `refuted` | `dormant`

---

## 3. Dream-State Memory Architecture

### 3.1 The Core Insight

> Hallucinations are not errors. They are dreams.
>
> Human brains don't treat unverified fragments as failures — they treat them
> as dreams: raw material that may contain novel patterns, creative leaps,
> or garbage. The brain's consolidation process sorts them during sleep.
>
> RQ^R2 adopts this architecture wholesale. Every new encoding enters as a
> dream. Dreams are not queryable as answers. They are raw substrate
> waiting for consolidation.

### 3.2 Why This Speeds Up the System

| Problem Without Dreams | Solution With Dreams |
|---|---|
| Hallucinated outputs are discarded entirely — wasted computation | Dreams are retained in a buffer, available for future pattern-matching |
| System halts on low-confidence encodings — blocks pipeline | Dreams flow through without blocking; flagged for async consolidation |
| Novel patterns that don't match existing radicals are rejected | Novel patterns enter as dreams — if they recur, they become theories |
| Binary trust (valid/invalid) creates brittleness | Four-tier trust (dream/theory/trusted/axiom) creates graceful degradation |
| Agent has no mechanism for creative hypothesizing | Dreams ARE the hypothesis mechanism — structured speculation |

### 3.3 Human Memory Analogy — Full Pipeline

```
HUMAN MEMORY PIPELINE          RQ^R2 EQUIVALENT
══════════════════════         ════════════════════════════

SENSORY REGISTER               INPUT BUFFER
  Duration: 250ms-3s             Duration: single tick
  Capacity: everything           Capacity: full input stream
  Lossy: most decays instantly   Lossy: tokenizer drops noise
  │                              │
  ▼                              ▼
ICONIC/ECHOIC MEMORY           DREAM BUFFER
  Duration: 0.5-4 seconds        Duration: configurable TTL
  Capacity: large but fading     Capacity: dreams table
  Fragmented, sensory-rich       Fragmented, radical-guessed
  NOT yet meaningful             NOT yet queryable as answers
  │                              │
  ▼ (attention selects)          ▼ (consolidation selects)
SHORT-TERM / WORKING MEMORY    THEORY BUFFER
  Duration: 15-30 seconds        Duration: until resolved
  Capacity: 7±2 chunks           Capacity: bounded by test queue
  Actively maintained            Actively tested against evidence
  Rehearsal keeps it alive       Corroboration keeps it alive
  │                              │
  ▼ (encoding + rehearsal)       ▼ (trust escalation)
LONG-TERM MEMORY               TRUSTED SYMBOLS
  Duration: minutes to lifetime  Duration: persistent
  Capacity: effectively unlimited Capacity: symbols table
  Consolidated during sleep      Consolidated during idle cycles
  Organized by association       Organized by radical index + sem coords
  │                              │
  ▼ (deep encoding)              ▼ (axiom promotion)
SEMANTIC/PROCEDURAL MEMORY     AXIOMS
  Duration: permanent            Duration: permanent, immutable
  Deeply encoded, context-free   Core truths, radical definitions
  "Knowing that" + "knowing how" System laws + base radical meanings
```

### 3.4 Consolidation — The "Sleep" Cycle

During idle periods (low CPU, no active queries), `rune_brain` runs a
consolidation cycle via `BrainDb::consolidation_tick()`:

```
STAGE 1 — LIGHT SLEEP (N1 analog)
  Scan dream buffer for:
  • Recurring fragments (consolidation_count ≥ 3)
  • High-confidence dreams (confidence ≥ 0.6)
  • Dreams corroborated by trusted symbols
  Action: flag candidates for promotion

STAGE 2 — DEEP SLEEP (N3 analog)
  For each flagged candidate:
  • Attempt full radical decomposition
  • Compute tentative sem_x/y/z
  • Cross-reference against existing symbols
  • If novel + coherent → promote to THEORY
  • If duplicate of trusted → merge and discard
  • If incoherent → increment rejection counter
  Action: write to theories table or reject

STAGE 3 — REM SLEEP (dream integration)
  Creative recombination:
  • Take 2–3 unrelated dreams
  • Attempt radical fusion (bind-rune rules)
  • Test if compound encodes a NOVEL concept
  • If yes → new dream with type 'lucid'
  Action: generate novel candidate glyphs

STAGE 4 — PRUNING (synaptic homeostasis analog)
  Garbage collection:
  • Expire dreams past TTL
  • Decay symbols with access_count = 0
  • Prune tendrils with zero traffic
  • Reclaim radicals from fully decayed glyphs
  Action: free resources, recycle radicals
```

### 3.5 Dream Types — Detailed

| Type | Human Analog | System Behavior | Example |
|---|---|---|---|
| `fragment` | Fleeting sensory impression | Partial encoding, missing radicals. Raw input preserved for later. | "shimmering boundary" → matched radicals for "boundary" but "shimmering" has no radical yet |
| `hallucination` | Confident false memory | Full encoding completed but no corroborating evidence. Looks real but isn't verified. | Agent confidently encodes "fire-tree-cycle" but no existing symbol supports that relationship |
| `confabulation` | Plausible gap-filling | System invented a connection between existing symbols to fill a gap. Structurally sound but fabricated. | Missing link between "water" and "growth" filled with invented "rain" glyph that has no source data |
| `hypnagogic` | Half-asleep imagery | Generated during consolidation cycle Stage 3. System's own creative output. | During REM-analog, system fused "fungi" + "wind" into novel "spore-dispersal" glyph |
| `lucid` | Aware-you're-dreaming | Dream flagged by the system itself as "this might be important." Priority review queue. | Fragment recurred 5 times and correlates with a trusted symbol — system flags it for fast-track promotion |

---

## 4. The 14-Dimensional Encoding Space

Every glyph exists as a point in 14-dimensional attribute space.

### 4.1 Dimension Definitions

| # | Dimension | Type | Range | Semantic Role |
|---|---|---|---|---|
| 1 | `sem_x` | float | -1.0 to 1.0 | Primary semantic axis (concrete ↔ abstract) |
| 2 | `sem_y` | float | -1.0 to 1.0 | Secondary semantic axis (individual ↔ collective) |
| 3 | `sem_z` | float | -1.0 to 1.0 | Tertiary semantic axis (static ↔ dynamic) |
| 4 | `color_hue` | float | 0.0 to 360.0 | Hue in HSL color space — domain association |
| 5 | `color_saturation` | float | 0.0 to 1.0 | Saturation — specificity/vividness |
| 6 | `brightness` | float | 0.0 to 1.0 | Luminance — prominence/importance |
| 7 | `ethereal_solid` | float | 0.0 to 1.0 | 0.0 = fully ethereal/abstract; 1.0 = fully solid/concrete |
| 8 | `temporal_phase` | float | -1.0 to 1.0 | -1.0 = deep past; 0.0 = present; 1.0 = deep future |
| 9 | `fractal_depth` | int | 0 to 12 | Recursive layers of self-similar meaning |
| 10 | `ac_ratio` | float | 0.0 to 1.0 | Angular (1.0) vs Curved (0.0) construction ratio |
| 11 | `trust_score` | float | 0.0 to 1.0 | Verification confidence level |
| 12 | `activation` | float | 0.0 to 1.0 | Current activity/relevance level |
| 13 | `connectivity` | int | 0 to N | Number of active tendril connections |
| 14 | `compression_q` | float | 0.0 to 1.0 | Compression quality (0 = max compressed, 1 = lossless) |

### 4.2 Fractal Depth Calculation

```
fractal_depth(input) =
  CASE
    WHEN input is single atomic radical           → 0
    WHEN input is compound of 2 radicals          → 1
    WHEN input is compound containing compounds   → max(children_depth) + 1
    WHEN input is self-referential (strange loop) → ∞ (capped at 12)
    WHEN input is a dream                         → -1 (undefined/pending)
  END
```

### 4.3 Semantic Coordinate Computation

```
sem_x = weighted_average(radical.x for each radical in glyph)
         adjusted by positional_weight(grid_position)

sem_y = weighted_average(radical.y for each radical in glyph)
         adjusted by relational_modifier(relationship_glyphs)

sem_z = weighted_average(radical.z for each radical in glyph)
         adjusted by movement_modifier(movement_glyphs)

WHERE positional_weight =
  top_left:     1.0  (determinative — strongest semantic pull)
  top_right:    0.8  (modifier)
  bottom_left:  0.6  (foundation)
  bottom_right: 0.4  (elaboration)
```

### 4.4 Compression Selection

```
compression_q = f(trust_state, access_frequency, connectivity)

  IF trust_state = 'axiom'   → compression_q = 1.0  (never lossy)
  IF trust_state = 'trusted' → compression_q = 0.8  (high fidelity)
  IF trust_state = 'theory'  → compression_q = 0.5  (balanced)
  IF trust_state = 'dream'   → compression_q = 0.2  (aggressive compress)

  ADJUST: +0.1 for each 100 accesses (frequently used = preserve detail)
  ADJUST: +0.05 per active tendril    (highly connected = preserve detail)
  CLAMP: 0.0 to 1.0
```

---

## 5. Radical Composition Engine

### 5.1 Core Radical Inventory (60 Radicals, 6 Tiers)

```
TIER 1 — ELEMENTAL (World-Entity Ontology)
  CR-01 Tree    CR-02 Bug    CR-03 Bird   CR-04 Fungi
  CR-05 Vine    CR-06 Flower CR-07 Soil   CR-08 Water
  CR-09 Fire    CR-10 Wind

TIER 2 — STRUCTURAL (Angular Domain)
  CR-11 Wall    CR-12 Frame  CR-13 Path   CR-14 Bridge
  CR-15 Gate    CR-16 Tower  CR-17 Root   CR-18 Branch
  CR-19 Grid    CR-20 Knot

TIER 3 — PROCESSUAL (Curved Domain)
  CR-21 Flow    CR-22 Spiral CR-23 Wave   CR-24 Bloom
  CR-25 Wilt    CR-26 Merge  CR-27 Split  CR-28 Twist
  CR-29 Echo    CR-30 Drift

TIER 4 — COGNITIVE (Brain Model)
  CR-31 Perceive CR-32 Remember CR-33 Decide  CR-34 Create
  CR-35 Compare  CR-36 Abstract  CR-37 Embody  CR-38 Dream
  CR-39 Focus    CR-40 Release

TIER 5 — RELATIONAL (Movement/Relationship)
  CR-41 Parent  CR-42 Child  CR-43 Sibling CR-44 Bond
  CR-45 Guard   CR-46 Feed   CR-47 Compete CR-48 Observe
  CR-49 Carry   CR-50 Anchor

TIER 6 — META (System-level)
  CR-51 Agent   CR-52 Message CR-53 Rule   CR-54 Error
  CR-55 Time    CR-56 Space   CR-57 Truth  CR-58 Unknown
  CR-59 Null    CR-60 Infinity
```

### 5.2 The 2×2 Composition Grid

```
┌─────────────────┬─────────────────┐
│   POSITION A    │   POSITION B    │
│   TOP-LEFT      │   TOP-RIGHT     │
│                 │                 │
│  DETERMINATIVE  │    MODIFIER     │
│  "What domain?" │  "What kind?"   │
│  Weight: 1.0    │  Weight: 0.8    │
├─────────────────┼─────────────────┤
│   POSITION C    │   POSITION D    │
│   BOTTOM-LEFT   │   BOTTOM-RIGHT  │
│                 │                 │
│   FOUNDATION    │  ELABORATION    │
│  "What base?"   │  "What nuance?" │
│  Weight: 0.6    │  Weight: 0.4    │
└─────────────────┴─────────────────┘

MINIMUM: 1 radical (Position A only — bare concept)
MAXIMUM: 4 radicals (all positions filled — full glyph)
```

### 5.3 Composition Rules

```
RULE 1: DETERMINATIVE FIRST
  Position A (top-left) is always the domain classifier.
  It sets the ontological context for the entire glyph.
  A glyph without Position A is a FRAGMENT (dream-state only).

RULE 2: READING ORDER
  A → B → C → D
  "Domain → Kind → Base → Nuance"

RULE 3: STROKE BUDGET
  Maximum total strokes per glyph: 14 (one per dimension)
  Each radical maximum: 5 strokes
  Bind-rune fusion MUST be applied when shared strokes exist

RULE 4: BIND-RUNE FUSION
  IF radical_A and radical_B share a structural stroke:
    → Merge at shared stroke
    → Total strokes = A + B - shared
    → Log fusion in lineage metadata

RULE 5: DISAMBIGUATION
  IF two glyphs would be visually identical after composition:
    → Add diacritical dot at canonical position
    → Priority: top-center > bottom-center > right-center

RULE 6: DREAM FRAGMENTS
  IF input cannot fill Position A (no determinative identified):
    → Encode as dream with type 'fragment'
    → Store raw input + best-guess radicals
    → Do NOT insert into symbols table
```

---

## 6. Memory Pipeline — Sensory to Permanent

### 6.1 Stage Definitions

```
STAGE 0: RAW INPUT (Sensory Register)
  Lifetime:  1 processing tick
  Location:  encoder input buffer (in-memory only)
  Trust:     none — not yet encoded
  Action:    tokenize, noise-filter, pass to Stage 1

STAGE 1: DREAM BUFFER (Iconic/Echoic Memory)
  Lifetime:  configurable TTL (default 24h)
  Location:  dreams table
  Trust:     dream (0.0 – 0.3)
  Queryable: NO — dreams are NOT returned as answers
  Action:    attempt radical match, store with confidence score

STAGE 2: THEORY BUFFER (Working Memory)
  Lifetime:  until resolved (promoted, refuted, or dormant-parked)
  Location:  theories table
  Trust:     theory (0.3 – 0.75)
  Queryable: YES — always tagged as [THEORY] in responses
  Action:    test against new evidence, accumulate pass/fail counts

STAGE 3: TRUSTED SYMBOLS (Long-Term Memory)
  Lifetime:  persistent (subject to decay if never accessed)
  Location:  symbols table with trust_state = 'trusted'
  Trust:     trusted (0.75 – 0.95)
  Queryable: YES — standard query results
  Action:    normal retrieval, routing, and composition

STAGE 4: AXIOMS (Semantic/Procedural Memory)
  Lifetime:  permanent — immune to decay
  Location:  symbols table with trust_state = 'axiom'
  Trust:     axiom (1.0 — immutable)
  Queryable: YES — highest priority in conflict resolution
  Action:    serve as ground truth for theory testing
```

### 6.2 Decay Model

```
decay_score = initial_score × e^(−λ × time_since_last_access)

WHERE λ (decay constant) varies by trust_state:
  dream:   λ = 0.1    (fast decay — dreams fade quickly)
  theory:  λ = 0.01   (slow decay — theories persist while tested)
  trusted: λ = 0.001  (very slow decay)
  axiom:   λ = 0.0    (no decay — axioms are permanent)

DECAY THRESHOLDS:
  decay_score < 0.1  → eligible for pruning
  decay_score < 0.01 → auto-pruned in next consolidation cycle
```

### 6.3 Symbolism in Storage

| Raw Input | Naive Storage | Symbolic Storage (RQ^R2) | Benefit |
|---|---|---|---|
| "The server crashed at 3am because of a memory leak" | String blob, 54 bytes | `[CR-54]+[CR-08]+[CR-25]+[CR-55]` = "error-flow-decay-at-time" | 4 radical IDs (~16 bytes), semantically searchable |
| "User authentication failed due to expired token" | String blob, 48 bytes | `[CR-15]+[CR-45]+[CR-54]+[CR-55]` = "gate-guard-error-time" | Clusters with all other auth failures via shared radicals |
| "I think maybe the cache might be stale" | String blob, 40 bytes (uncertain) | Dream: `[CR-32?]+[CR-25?]` confidence=0.4 | Stored as dream, not asserted as fact |

---

## 7. Trust Escalation Protocol

### 7.1 State Machine

```
                    ┌──────────────────────────┐
                    │                          │
                    ▼                          │
┌─────────┐   ┌─────────┐   ┌──────────┐      │   ┌─────────┐
│  DREAM  │──►│ THEORY  │──►│ TRUSTED  │──────┴──►│  AXIOM  │
│         │   │         │   │          │           │         │
│ 0.0-0.3 │   │ 0.3-0.75│   │ 0.75-0.95│           │   1.0   │
└────┬────┘   └────┬────┘   └────┬─────┘           └─────────┘
     │              │              │
     │ rejected      │ refuted      │ decayed
     ▼              ▼              ▼
┌─────────┐   ┌─────────┐   ┌──────────┐
│ EXPIRED │   │ REFUTED │   │ RECYCLED │
│ (purged)│   │ (logged)│   │ (radicals│
│         │   │         │   │  freed)  │
└─────────┘   └─────────┘   └──────────┘
```

### 7.2 Promotion Criteria

| Transition | Criteria | Mechanism |
|---|---|---|
| Dream → Theory | `confidence ≥ 0.6` OR `consolidation_count ≥ 3` OR corroborated by 1+ trusted symbol | Consolidation cycle Stage 2 |
| Theory → Trusted | `trust_score ≥ 0.75` AND `pass_count ≥ 3` AND `fail_count / pass_count < 0.2` | Trust escalation after each test |
| Trusted → Axiom | Manual only — requires explicit agent or operator confirmation | Human-in-the-loop or Overwatch approval |
| Dream → Expired | `expires_at` reached OR `decay_score < 0.01` | Pruning cycle |
| Theory → Refuted | `fail_count / test_count > 0.5` AND `test_count ≥ 5` | Auto-refutation |
| Trusted → Recycled | `decay_score < 0.01` AND `access_count = 0` for extended period | Pruning cycle |

### 7.3 Query Behavior by Trust State

```
query("what caused the outage?")

RESULTS RETURNED:

  1. [TRUSTED] CR-54+CR-08+CR-25+CR-55
     "error in flow system caused decay at timestamp"
     trust_score: 0.88
     → Presented as ANSWER

  2. [THEORY] CR-54+CR-19+CR-47
     "error in grid from contention"
     trust_score: 0.52
     → Presented as: "There is a developing theory that grid contention
        may also be involved (52% confidence, tested 4 times, passed 2)"

  3. [DREAM] CR-54+CR-38
     confidence: 0.3
     → NOT PRESENTED to user
     → Logged internally for consolidation review
```

---

## 8. Glyph Construction Engine

### 8.1 Construction Pipeline

```
INPUT:  semantic_content (string or structured data)
OUTPUT: EncodedSymbol

STEP 1 — TOKENIZE
  Break input into semantic tokens; remove noise; identify language

STEP 2 — RADICAL MATCH
  For each token, search CR-01 through CR-60:
  score ≥ 0.7 → confident match
  score ≥ 0.4 → candidate
  score < 0.4 → UNMATCHED → flag for dream

STEP 3 — GRID PLACEMENT
  Sort matched radicals:
  Domain/determinative → Position A
  Modifier            → Position B
  Foundation          → Position C
  Elaboration         → Position D
  If >4 matched: select top 4 by score; remainder → halo metadata

STEP 4 — BIND-RUNE FUSION
  For each adjacent pair: identify shared strokes; merge; log in lineage

STEP 5 — STROKE RENDERING
  Apply angular-first rule; apply wedge terminals last
  Verify total stroke_count ≤ 14; simplify if over budget

STEP 6 — COORDINATE COMPUTATION
  Calculate sem_x, sem_y, sem_z, fractal_depth, ac_ratio, compression_q

STEP 7 — TRUST ASSIGNMENT
  New encoding:    trust_state = 'dream', trust_score = confidence
  Exception:       all radicals are axiom-level → trust_state = 'theory'
  Exception:       exact glyph already trusted → merge, don't duplicate

STEP 8 — HALO ATTACHMENT
  Compute all 14 dimensions; package as halo_json
  Attach AEL overlay if emotional content detected
  Attach ILE context if ambiguity detected

STEP 9 — EMIT
  Return EncodedSymbol to LibrarianAgent for storage and routing
```

### 8.2 Stroke Codex

```
ANGULAR STROKES (Phoenician-derived)
  ANG-01: Vertical bar  |    ANG-06: Right angle Γ  ┌
  ANG-02: Horizontal bar ─   ANG-07: Chevron up    ^
  ANG-03: Forward diagonal / ANG-08: Chevron down  v
  ANG-04: Back diagonal  \   ANG-09: Cross          +
  ANG-05: Right angle L  └   ANG-10: X-cross        ×

WEDGE STROKES (Ugaritic-derived)
  WDG-01: Horizontal wedge ◄  WDG-04: Double wedge  ◄◄
  WDG-02: Vertical wedge   ▲  WDG-05: Micro wedge   •
  WDG-03: Corner wedge     ◣

CURVED STROKES (Organic/processual)
  CRV-01: Arc      ◠   CRV-04: Wave     ~
  CRV-02: Half circle )  CRV-05: Spiral   @
  CRV-03: Full circle ○  CRV-06: S-curve  ∿

GRID STROKES (Old Persian-derived)
  GRD-01: Column stack ┃┃  GRD-04: Nested box  ⊡
  GRD-02: Row stack    ═   GRD-05: Grid cross   ╬
  GRD-03: Box frame    □
```

---

## 9. Angular vs Curved Partitioning

### 9.1 The Binary Axis

```
ANGULAR (ac_ratio → 1.0)          CURVED (ac_ratio → 0.0)
═══════════════════════           ═══════════════════════
Structure                         Process
Logic                             Emotion
Boundary                          Flow
Discrete                          Continuous
Mineral                           Organic
Built                             Grown
Static                            Dynamic
Defined                           Emergent
Strokes: |─/\+×                   Strokes: )(~○◠◡∿
```

### 9.2 Hybrid Zone (ac_ratio ≈ 0.5)

| Hybrid Concept | Angular Base | Curved Extension | ac_ratio |
|---|---|---|---|
| Bridge | Structure (span) | Connection (reaching) | 0.55 |
| Algorithm | Logic (steps) | Flow (execution) | 0.60 |
| Lightning | Energy (force) | Path (branching) | 0.45 |
| Metamorphosis | Before-state (form) | Becoming (process) | 0.35 |
| Dream-to-Theory | Fragment (partial) | Emergence (forming) | 0.40 |

---

## 10. World-Entity Ontology

### 10.1 The Ten Primordial Domains

| # | Domain | Radical | Element | Computing Metaphor |
|---|---|---|---|---|
| 1 | Tree | CR-01 | Wood | File systems, ASTs, class hierarchies |
| 2 | Bug | CR-02 | Chitin | Microservices, worker threads, swarm |
| 3 | Bird | CR-03 | Air-bone | Message brokers, event dispatchers |
| 4 | Fungi | CR-04 | Mycelium | Distributed caches, gossip protocols |
| 5 | Vine | CR-05 | Tendril | Dependency injection, middleware |
| 6 | Flower | CR-06 | Petal | UI components, API surfaces |
| 7 | Soil | CR-07 | Earth | Databases, persistent storage |
| 8 | Water | CR-08 | Fluid | Streams, pipelines, ETL |
| 9 | Fire | CR-09 | Plasma | Compilers, optimizers, GC |
| 10 | Wind | CR-10 | Gas | Network I/O, broadcast, pub/sub |

### 10.2 Affinity Matrix

```
         TREE BUG BIRD FUNG VINE FLOW SOIL WATR FIRE WIND
  TREE ──  +    ·    +    +    ·    +    ·    ─    ·
  BUG   +  ──   ─    +    ·    +    +    ·    ·    ·
  BIRD  ·   ─   ──   ·    ·    ·    ·    ·    ·    +
  FUNG  +   +   ·    ──   +    ·    +    +    ─    ·
  VINE  +   ·   ·     +   ──   +    ·    +    ·    ·
  FLOW  ·   +   ·     ·    +   ──   ·    +    ─    +
  SOIL  +   +   ·     +    ·    ·   ──   +    +    ─
  WATR  ·   ·   ·     +    +    +    +   ──   ─    +
  FIRE  ─   ·   ·     ─    ·    ─    +    ─   ──   +
  WIND  ·   ·   +     ·    ·    +    ─    +    +   ──

  + affinity   ─ tension   · neutral
```

---

## 11. Morphology Layers

### 11.1 Icelandic Morphology — Case Inflection

| Case | Role | Suffix | Example |
|---|---|---|---|
| Nominative | AGENT (acts) | -∅ (unmarked) | ◆fire = "fire does" |
| Accusative | PATIENT (acted upon) | -╴ | ◆fire╴ = "fire is affected" |
| Dative | INSTRUMENT (by means of) | -╶ | ◆fire╶ = "by means of fire" |
| Genitive | POSSESSOR (of/belonging) | -╷ | ◆fire╷ = "of fire / fire's" |

### 11.2 Old Norse Runic Morphology — Aettir System

Three families of 8 radicals each (cognitive load: 7±2):

```
FIRST AETT  — Creation & Substance: resource, force, chaos, authority,
               journey, vulnerability, exchange, fulfillment
SECOND AETT — Disruption & Transformation: disruption, constraint,
               stillness, cycle, energy, justice, growth, identity
THIRD AETT  — Consciousness & Completion: flow, persistence, protection,
               clarity, belonging, potential, foundation, destiny
```

Activation States:

| State | Marker | Meaning |
|---|---|---|
| Dormant | (none) | Exists but inactive |
| Awakened | ˙ | Loaded in working memory |
| Charged | ¨ | Actively being processed |
| Released | ° | Processing complete, results emitted |
| Sealed | ¯ | Locked — cannot be modified |

### 11.3 Chinese Radical Logic

- 60 core radicals ÷ 6 tiers generate unbounded glyph space
- Positional semantics in 2×2 grid (§5.2)
- K'ang-hsi-style radical indexing for O(1) semantic lookup
- Simplified variants for bandwidth optimization

### 11.4 Japanese Kami Semantic Clusters

| Kami Concept | QRrune Mapping |
|---|---|
| Kami as emergent quality | Activation state exceeding threshold = kami-status |
| Musubi (interconnecting energy) | Connectivity metadata channel |
| Kodama/Mizukami/Yama-uba (domain spirits) | Domain radicals as kami-type classifiers |
| Shinkai (hidden mirror-world) | Halo metadata layer |
| Kannagara (harmony with nature's awe) | Overwatch optimization objective |

---

## 12. ILE — Interpretation Layer Engine

### 12.1 Purpose

The ILE resolves contextual meaning from raw glyph data. A glyph's meaning
is not fixed — it shifts based on surrounding glyphs, query context, agent
state, and trust level.

### 12.2 Interpretation Pipeline

```
INPUT: EncodedSymbol + QueryContext

STEP 1 — EXPLICATE
  Unfold the glyph's halo → extract all 14 dimensions
  Identify domain (determinative radical)
  Read trust_state — this gates how the result is framed

STEP 2 — CONTEXTUALIZE
  Find neighboring glyphs (connected via tendrils)
  Weight meaning by neighbor influence
  Apply Icelandic case inflection based on query role

STEP 3 — PREDICT (Hawkins-inspired)
  Generate predicted meaning BEFORE full decode
  If prediction matches → fast-path return (VEL-FLASH)
  If prediction misses → full decode + learning signal

STEP 4 — RESOLVE AMBIGUITY
  If multiple meanings survive:
    Vote across connected agents (trust-weighted)
    Present top interpretation + alternatives with confidence
    Flag for ILE refinement if confidence < 0.6

STEP 5 — FRAME OUTPUT
  axiom   → stated as fact, no qualification
  trusted → stated as answer
  theory  → stated as hypothesis with evidence summary
  dream   → NOT stated — logged for consolidation only

STEP 6 — EMIT
  Return InterpretedMeaning to requesting agent
  Update glyph's accessed_at and access_count
  Refresh decay_score
```

### 12.3 Three-Axis Interpretation (ARAS-inspired)

Every glyph is interpretable along three axes:

1. **Etymological** — what radicals compose it, and what they each mean
2. **Shadow/Opposite** — what is the inverse glyph, and what tension exists
3. **Paradox** — what contradiction does the glyph contain within itself

```
EXAMPLE: Glyph [CR-09 Fire] + [CR-01 Tree]

  Etymological: "fire applied to tree" = burning, forge, transformation
  Shadow:        inverse = [CR-08 Water] + [CR-07 Soil] = "quenching, grounding"
  Paradox:       fire destroys trees yet forest fires enable new growth
                 the glyph contains destruction AND renewal simultaneously
```

---

## 13. AEL — Affective-Emotive Overlays

### 13.1 Purpose

The AEL encodes the emotional/tonal dimension of glyphs. Not all
information is neutral — some carries urgency, grief, wonder, or warning.

### 13.2 Affect Primitives

| Affect | Channel | Range | Examples |
|---|---|---|---|
| Valence | positive ↔ negative | -1.0 to 1.0 | joy (+0.9), grief (-0.8), neutral (0.0) |
| Arousal | calm ↔ urgent | 0.0 to 1.0 | meditation (0.1), alarm (0.95) |
| Dominance | submissive ↔ commanding | 0.0 to 1.0 | request (0.2), decree (0.9) |
| Novelty | familiar ↔ strange | 0.0 to 1.0 | routine (0.1), unprecedented (0.95) |
| Sacredness | mundane ↔ kami | 0.0 to 1.0 | log entry (0.05), axiom-birth (0.99) |

### 13.3 AEL Attachment

```json
{
  "ael": {
    "valence": 0.3,
    "arousal": 0.7,
    "dominance": 0.5,
    "novelty": 0.8,
    "sacredness": 0.2,
    "archetypal_tone": "emergence",
    "dream_affect": true
  }
}
```

`dream_affect: true` indicates the emotional encoding is itself tentative.

---

## 14. Mycelium Routing Layer

### 14.1 Tendril Connections

```sql
CREATE TABLE IF NOT EXISTS tendrils (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    source_symbol  INTEGER NOT NULL,
    target_symbol  INTEGER NOT NULL,
    weight         REAL    NOT NULL DEFAULT 0.5,
    traffic_count  INTEGER DEFAULT 0,
    last_traversed TEXT,
    created_at     TEXT    DEFAULT (datetime('now')),
    tendril_type   TEXT    NOT NULL DEFAULT 'association'
);
```

`tendril_type` values: `association` | `causal` | `compositional` | `antithetical` | `dream_bridge`

### 14.2 Routing Rules

```
1. STRENGTHEN ON USE    — traversed tendrils gain +0.01 weight per traversal
2. ATROPHY ON NEGLECT  — unused tendrils decay (same λ model as symbols)
3. PRUNE AT ZERO       — tendrils with weight < 0.01 are pruned
4. GROW ON CORRELATION — if two symbols co-occur in queries, sprout a tendril
5. CAERN ANCHORING     — high-connectivity symbols become caern (hub) nodes
6. DREAM BRIDGES       — tendrils connecting dreams to trusted symbols are
                          tentative until the dream is promoted
7. NO PARASITIC DRAINS — tendrils must carry bidirectional value or be flagged
```

### 14.3 Caern Nodes

A symbol becomes a caern (sacred anchor node) when:

```
connectivity ≥ caern_threshold (default: 10 active tendrils)
AND trust_state IN ('trusted', 'axiom')
AND decay_score > 0.8
```

Caerns receive: higher `compression_q`, slower decay, priority in consolidation,
and serve as reference frames for nearby symbols (Hawkins cortical column analog).

---

## 15. Brain-Inspired Cognition Model

### 15.1 Dual Hemisphere Processing

```
LEFT HEMISPHERE (Angular Processing)    RIGHT HEMISPHERE (Curved Processing)
═══════════════════════════════════     ════════════════════════════════════
Sequential analysis                     Holistic pattern recognition
Radical decomposition                   Gestalt glyph recognition
Rule application                        Intuitive association
Logical inference                       Emotional resonance
Theory testing                          Dream generation
```

### 15.2 Cortical Column Model (Hawkins-inspired)

Each agent node in the mesh operates as an independent cortical column:

- Runs the same glyph-processing algorithm
- Has unique input connections (different reference frame)
- Builds a complete local model of its domain
- Votes with peers for consensus interpretation
- **Prediction-first**: generates expected meanings before decoding

### 15.3 Pattern Recognition Tiers

| Tier | Speed | Mechanism | Maps To |
|---|---|---|---|
| Reflexive | VEL-FLASH | Exact radical match in axiom cache | Brainstem reflex |
| Habitual | VEL-RUSH | Trusted symbol lookup by radical index | Basal ganglia habit |
| Analytical | VEL-WALK | Full ILE pipeline with context resolution | Cortical deliberation |
| Creative | VEL-DRIFT | Dream consolidation, radical recombination | Default mode network |

---

## 16. Pocket-Galaxy Knowledge Model

### 16.1 Structure

```
GALAXY
├── CLUSTER (domain-level grouping)
│   ├── POCKET (self-contained knowledge unit)
│   │   ├── symbols[]
│   │   ├── tendrils[] (internal connections)
│   │   └── halo_aggregate (cluster-level metadata)
│   └── ...
└── INTERSTELLAR TENDRILS (cross-cluster connections)
```

### 16.2 Dream Nebulae

Unverified dreams float in dream nebulae — diffuse clouds of potential meaning
at the galaxy's edges. During consolidation, dreams may be gravitationally
captured by a nearby cluster when their semantic coordinates fall within a
cluster's radius.

---

## 17. Metadata Channels

### 17.1 Channel Bus (halo_json)

| Channel | Type | Range | Function |
|---|---|---|---|
| `color` | HSL object | H:0-360 S:0-1 L:0-1 | Domain association |
| `intensity` | float | 0.0–1.0 | Signal strength / importance |
| `halo` | float | 0.0–1.0 | Boundary glow — visibility of implicate layer |
| `trust` | float | 0.0–1.0 | Verification confidence |
| `activation` | float | 0.0–1.0 | Current relevance |
| `connectivity` | int | 0–N | Active tendril count |
| `temporal` | float | -1.0–1.0 | Past ↔ future orientation |
| `spatial` | vec3 | sem_x/y/z | Position in semantic space |
| `lineage` | array | symbol IDs | Ancestry chain |
| `ael` | object | see §13 | Affective-emotive overlay |

### 17.2 Domain Color Assignments

| Domain | Hue | Color Name |
|---|---|---|
| Tree | 120° | Forest green |
| Bug | 45° | Amber |
| Bird | 200° | Sky blue |
| Fungi | 280° | Deep violet |
| Vine | 150° | Teal |
| Flower | 330° | Rose |
| Soil | 30° | Earth brown |
| Water | 210° | Ocean blue |
| Fire | 10° | Ember red |
| Wind | 180° | Pale cyan |

### 17.3 Trust-State Color Modulation

| Trust State | Saturation | Brightness | Halo | Visual Effect |
|---|---|---|---|---|
| Dream | 0.2 (desaturated) | 0.3 (dim) | 0.1 (faint) | Ghostly, translucent |
| Theory | 0.5 (medium) | 0.5 (moderate) | 0.4 (visible) | Present but tentative |
| Trusted | 0.8 (vivid) | 0.8 (bright) | 0.7 (strong) | Solid, reliable, clear |
| Axiom | 1.0 (pure) | 1.0 (full) | 1.0 (radiant) | Luminous, foundational |

## 18. Distributed Mesh Integration

### 18.1 Yggdrasil Overlay Topology

RQ^R2 nodes communicate over an encrypted Yggdrasil mesh overlay. Every node
runs an identical `rune_brain` instance with its own local BrainDb, dream
buffer, and theory register. The mesh is flat — no master, no coordinator, no
single point of failure.

```
Node A (rune_brain :7071)           Node B (rune_brain :7072)
┌───────────────────────────┐       ┌───────────────────────────┐
│  RQ^R2 Encoder            │       │  RQ^R2 Encoder            │
│  LibrarianAgent           │◄─────►│  LibrarianAgent           │
│  BrainDb (local SQLite)   │Yggdra-│  BrainDb (local SQLite)   │
│  Dreams (local)           │sil    │  Dreams (local)           │
│  Theories (local)         │encr.  │  Theories (local)         │
│  Tendrils (local + xref)  │mesh   │  Tendrils (local + xref)  │
└─────────────┬─────────────┘       └─────────────┬─────────────┘
              │           Node C (rune_brain :7073) │
              │    ┌───────────────────────────┐    │
              └───►│  RQ^R2 Encoder            │◄───┘
                   │  LibrarianAgent           │
                   │  BrainDb (local SQLite)   │
                   │  Overwatch (monitor role) │
                   └───────────────────────────┘
```

### 18.2 Node Roles

Every node is a peer. Nodes may volunteer for additional roles without gaining
authority over other nodes:

| Role | Description | Constraints |
|---|---|---|
| `peer` | Default. Encodes, stores, routes, participates in consensus. | All nodes are always peers regardless of other roles |
| `caern_keeper` | Hosts a disproportionate number of caern hub symbols. High uptime, high storage. | Self-elected based on resource availability. No special privileges. |
| `dream_garden` | Dedicates extra cycles to consolidation. Runs REM-analog recombination more aggressively. | Optional. Produces more lucid and hypnagogic dream candidates. |
| `overwatch` | Monitors mesh health, fairness, and trust distribution. Does NOT dictate. Advises. | Read-only view of aggregate metrics. Cannot modify other nodes' data. |
| `relay` | Forwards messages between nodes that can't reach each other directly. | Stateless pass-through. Does not decode or store relayed symbols. |
| `archivist` | Maintains deep history — stores decayed and recycled symbols for forensic recall. | Write-heavy, read-rare. Cold storage analog. |

### 18.3 Mesh Message Types

| Type | Purpose | Trust Rule |
|---|---|---|
| `SYMBOL_BROADCAST` | "I encoded something new — here it is" | Symbol arrives at recipient as DREAM regardless of sender's trust_state |
| `THEORY_PROPOSAL` | "I promoted a dream to theory — does your evidence agree?" | Theory remains at sender's trust_score; votes adjust it |
| `THEORY_VOTE` | "Here's my evidence for/against your theory" | Vote weight = avg trust_score of voter's relevant symbols |
| `CONSENSUS_REQUEST` | "I can't resolve this glyph — what do you think it means?" | Majority wins; requires quorum ≥ 3 voters |
| `TENDRIL_HANDSHAKE` | "I see a connection between these two symbols — do you?" | Recipient evaluates and accepts/rejects |
| `DECAY_NOTICE` | "This symbol is fading / gone on my node" | Recipients note independently; may preserve or let their copy decay |
| `HEARTBEAT` | Mesh health monitoring | Overwatch aggregates; peers use for neighbor discovery |
| `DREAM_FRAGMENT_SHARE` | "My consolidation produced something interesting" (dream_garden only) | Ingested as new dream in recipient's buffer |

### 18.4 Cross-Node Trust Rules

```
RULE 1: FOREIGN SYMBOLS START AS DREAMS
  Any symbol received from another node enters the local BrainDb
  as trust_state = 'dream', regardless of sender's trust_state.
  Trust is EARNED locally, never imported.

RULE 2: THEORY VOTING IS WEIGHTED
  A node's vote weight = average trust_score of its relevant symbols.

RULE 3: NO TRANSITIVE TRUST
  Node A trusts a symbol + Node B trusts Node A ≠ Node B trusts symbol.

RULE 4: CAERN NODES ARE NOT AUTHORITIES
  Caern status reflects connectivity density, not truth value.

RULE 5: DREAM BRIDGES ARE ALWAYS LOCAL
  Tendrils of type 'dream_bridge' are never shared across nodes.

RULE 6: CONSENSUS REQUIRES QUORUM
  Minimum 3 voting nodes. If <3 reachable, fall back to local-only
  interpretation with lowered confidence.

RULE 7: MESH PARTITIONS ARE SAFE
  Split mesh fragments operate independently. On reconnection, symbols
  are exchanged as SYMBOL_BROADCAST and re-evaluated as dreams.
  No "split brain" corruption because foreign symbols always re-enter
  as dreams.
```

### 18.5 Physarum-Inspired Route Optimization

The mesh does not use routing tables. Tendril weights function as the routing
topology — modeled on slime mold (*Physarum polycephalum*) behavior:

```
1. INITIAL STATE      All inter-node tendrils start at weight 0.5
2. ON SUCCESS         Strengthen all tendrils in delivery path: weight += 0.02
3. ON FAILURE         Weaken tendril: weight -= 0.05 (asymmetric — punish faster)
4. ATROPHY CYCLE      weight *= 0.99 per cycle. Prune if weight < 0.01
5. ROUTE SELECTION    Choose path with highest product(tendril_weights)
6. EXPLORATION        5% of messages routed via random path (epsilon-greedy)
7. SELF-HEALING       Offline node → adjacent tendrils: weight *= 0.5 per missed heartbeat
```

### 18.6 Disaster Recovery

```
USB BACKUP PROTOCOL
════════════════════════════════════════════════

BACKUP (automated):
  1. Export BrainDb → SQLite file (symbols, dreams, theories, tendrils)
  2. Bundle as: rqr2-backup-{ISO8601}.tar.zst
  3. Copy to mounted USB; verify checksum; log event to BrainDb

RESTORE:
  1. Mount USB, locate latest valid backup; verify checksum
  2. Import into fresh BrainDb
  3. Symbols enter as trust_state = 'trusted' (they were trusted when backed up)
  4. Dreams re-enter as dreams (re-consolidate)
  5. Theories re-enter as theories (re-test)
  6. Tendril weights reset to 0.5 (must re-earn routing confidence)
  7. Node rejoins mesh and broadcasts HEARTBEAT

CATASTROPHIC LOSS (no backup):
  1. Axiom radicals (CR-01 → CR-60) are hardcoded in the RQ^R2 library
  2. Re-bootstrap from axioms
  3. Accept incoming SYMBOL_BROADCAST from mesh peers as dreams
  4. Run aggressive consolidation cycles to rebuild
  5. The system regrows like a forest after fire
```

---

## 19. JSON Schemas

### 19.1 EncodedSymbol

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://qrrune.local/schemas/encoded-symbol.json",
  "title": "EncodedSymbol",
  "type": "object",
  "required": ["id","radicals","grid","sem_x","sem_y","sem_z",
               "fractal_depth","compression_q","ac_ratio",
               "trust_state","trust_score","halo","created_at"],
  "properties": {
    "id":             { "type": "string", "format": "uuid" },
    "radicals":       { "type": "array", "items": { "$ref": "#/$defs/RadicalRef" },
                        "minItems": 1, "maxItems": 4 },
    "grid":           { "$ref": "#/$defs/CompositionGrid" },
    "strokes":        { "type": "array", "items": { "$ref": "#/$defs/Stroke" }, "maxItems": 14 },
    "sem_x":          { "type": "number", "minimum": -1.0, "maximum": 1.0 },
    "sem_y":          { "type": "number", "minimum": -1.0, "maximum": 1.0 },
    "sem_z":          { "type": "number", "minimum": -1.0, "maximum": 1.0 },
    "fractal_depth":  { "type": "integer", "minimum": -1, "maximum": 12 },
    "compression_q":  { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "ac_ratio":       { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "trust_state":    { "type": "string", "enum": ["dream","theory","trusted","axiom"] },
    "trust_score":    { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "domain":         { "type": ["string","null"],
                        "enum": ["tree","bug","bird","fungi","vine","flower",
                                 "soil","water","fire","wind",null] },
    "activation_state": { "type": "string",
                          "enum": ["dormant","awakened","charged","released","sealed"],
                          "default": "dormant" },
    "case_inflection":  { "type": "string",
                          "enum": ["nominative","accusative","dative","genitive"],
                          "default": "nominative" },
    "halo":           { "$ref": "#/$defs/Halo" },
    "ael":            { "$ref": "#/$defs/AEL" },
    "lineage":        { "$ref": "#/$defs/Lineage" },
    "dream_source":   { "type": ["string","null"], "format": "uuid" },
    "created_at":     { "type": "integer" },
    "promoted_at":    { "type": ["integer","null"] },
    "accessed_at":    { "type": ["integer","null"] },
    "access_count":   { "type": "integer", "minimum": 0, "default": 0 },
    "decay_score":    { "type": "number", "minimum": 0.0, "maximum": 1.0, "default": 1.0 }
  },
  "$defs": {
    "RadicalRef": {
      "type": "object",
      "required": ["id","tier","label"],
      "properties": {
        "id":            { "type": "string", "pattern": "^CR-[0-6][0-9]$" },
        "tier":          { "type": "integer", "minimum": 1, "maximum": 6 },
        "label":         { "type": "string" },
        "grid_position": { "type": "string", "enum": ["A","B","C","D"] },
        "weight":        { "type": "number", "minimum": 0.0, "maximum": 1.0 },
        "mutated":       { "type": ["string","null"],
                           "enum": ["i-mutation","u-mutation","a-mutation","ö-mutation",null] }
      }
    },
    "CompositionGrid": {
      "type": "object",
      "properties": {
        "A": { "type": ["string","null"], "description": "Top-left: determinative" },
        "B": { "type": ["string","null"], "description": "Top-right: modifier" },
        "C": { "type": ["string","null"], "description": "Bottom-left: foundation" },
        "D": { "type": ["string","null"], "description": "Bottom-right: elaboration" }
      }
    },
    "Stroke": {
      "type": "object",
      "required": ["id","category","order"],
      "properties": {
        "id":         { "type": "string", "pattern": "^(ANG|WDG|CRV|GRD)-[0-9]{2}$" },
        "category":   { "type": "string", "enum": ["angular","wedge","curved","grid"] },
        "order":      { "type": "integer", "minimum": 1, "maximum": 14 },
        "fused_with": { "type": ["string","null"] },
        "pressure":   { "type": "string", "enum": ["whisper","voice","command"],
                        "default": "voice" }
      }
    },
    "Halo": {
      "type": "object",
      "required": ["color","intensity","halo_glow","trust","activation","connectivity","temporal","spatial"],
      "properties": {
        "color":        { "type": "object", "properties": {
                            "hue":        { "type": "number", "minimum": 0, "maximum": 360 },
                            "saturation": { "type": "number", "minimum": 0, "maximum": 1 },
                            "lightness":  { "type": "number", "minimum": 0, "maximum": 1 } } },
        "intensity":    { "type": "number", "minimum": 0.0, "maximum": 1.0 },
        "halo_glow":    { "type": "number", "minimum": 0.0, "maximum": 1.0 },
        "trust":        { "type": "number", "minimum": 0.0, "maximum": 1.0 },
        "activation":   { "type": "number", "minimum": 0.0, "maximum": 1.0 },
        "connectivity": { "type": "integer", "minimum": 0 },
        "temporal":     { "type": "number", "minimum": -1.0, "maximum": 1.0 },
        "spatial":      { "type": "object", "properties": {
                            "x": { "type": "number" },
                            "y": { "type": "number" },
                            "z": { "type": "number" } } },
        "lineage_ids":  { "type": "array", "items": { "type": "string", "format": "uuid" } }
      }
    },
    "AEL": {
      "type": "object",
      "properties": {
        "valence":         { "type": "number", "minimum": -1.0, "maximum": 1.0 },
        "arousal":         { "type": "number", "minimum": 0.0,  "maximum": 1.0 },
        "dominance":       { "type": "number", "minimum": 0.0,  "maximum": 1.0 },
        "novelty":         { "type": "number", "minimum": 0.0,  "maximum": 1.0 },
        "sacredness":      { "type": "number", "minimum": 0.0,  "maximum": 1.0 },
        "archetypal_tone": { "type": ["string","null"],
                             "enum": ["emergence","severance","threshold","descent",
                                      "return","union","dissolution","revelation",
                                      "guardianship","harvest","exile","renewal",null] },
        "dream_affect":    { "type": "boolean", "default": false }
      }
    },
    "Lineage": {
      "type": "object",
      "properties": {
        "parent_id":          { "type": ["string","null"], "format": "uuid" },
        "generation":         { "type": "integer", "minimum": 0 },
        "fusion_sources":     { "type": "array", "items": { "type": "string", "format": "uuid" } },
        "recycled_radicals":  { "type": "array", "items": { "type": "string",
                                                             "pattern": "^CR-[0-6][0-9]$" } }
      }
    }
  }
}
```

### 19.2 Dream

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://qrrune.local/schemas/dream.json",
  "title": "Dream",
  "type": "object",
  "required": ["id","raw_input","confidence","dream_type","created_at"],
  "properties": {
    "id":                   { "type": "string", "format": "uuid" },
    "raw_input":            { "type": "string" },
    "fragment_data":        { "type": ["object","null"] },
    "radicals_guess":       { "type": "array", "items": {
                                "type": "object",
                                "properties": {
                                  "radical_id":   { "type": "string", "pattern": "^CR-[0-6][0-9]$" },
                                  "confidence":   { "type": "number", "minimum": 0.0, "maximum": 1.0 },
                                  "match_reason": { "type": "string" } } } },
    "sem_x":                { "type": ["number","null"] },
    "sem_y":                { "type": ["number","null"] },
    "sem_z":                { "type": ["number","null"] },
    "confidence":           { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "source_agent":         { "type": ["string","null"] },
    "source_context":       { "type": ["string","null"] },
    "dream_type":           { "type": "string",
                              "enum": ["fragment","hallucination","confabulation","hypnagogic","lucid"] },
    "consolidation_count":  { "type": "integer", "minimum": 0, "default": 0 },
    "last_consolidated":    { "type": ["integer","null"] },
    "promoted_to":          { "type": ["string","null"], "format": "uuid" },
    "rejected_at":          { "type": ["integer","null"] },
    "rejection_reason":     { "type": ["string","null"] },
    "created_at":           { "type": "integer" },
    "expires_at":           { "type": ["integer","null"] }
  }
}
```

### 19.3 Theory

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://qrrune.local/schemas/theory.json",
  "title": "Theory",
  "type": "object",
  "required": ["id","dream_origin","glyph_data","radicals","sem_x","sem_y","sem_z",
               "fractal_depth","compression_q","hypothesis","trust_score","status","created_at"],
  "properties": {
    "id":                       { "type": "string", "format": "uuid" },
    "dream_origin":             { "type": "string", "format": "uuid" },
    "glyph_data":               { "type": "object" },
    "radicals":                 { "type": "array", "items": { "type": "string",
                                                               "pattern": "^CR-[0-6][0-9]$" } },
    "sem_x":                    { "type": "number" },
    "sem_y":                    { "type": "number" },
    "sem_z":                    { "type": "number" },
    "fractal_depth":            { "type": "integer" },
    "compression_q":            { "type": "number" },
    "hypothesis":               { "type": "string" },
    "supporting_evidence":      { "type": "array", "items": { "type": "string" } },
    "contradicting_evidence":   { "type": "array", "items": { "type": "string" } },
    "test_count":               { "type": "integer", "minimum": 0, "default": 0 },
    "pass_count":               { "type": "integer", "minimum": 0, "default": 0 },
    "fail_count":               { "type": "integer", "minimum": 0, "default": 0 },
    "trust_score":              { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "promotion_threshold":      { "type": "number", "default": 0.75 },
    "mesh_votes":               { "type": "array", "items": {
                                    "type": "object",
                                    "properties": {
                                      "node_id":    { "type": "string" },
                                      "vote":       { "type": "string",
                                                      "enum": ["support","contest","abstain"] },
                                      "confidence": { "type": "number" },
                                      "evidence":   { "type": "array",
                                                      "items": { "type": "string" } },
                                      "timestamp":  { "type": "integer" } } } },
    "status":                   { "type": "string",
                                  "enum": ["active","supported","contested",
                                           "promoted","refuted","dormant"] },
    "created_at":               { "type": "integer" },
    "last_tested":              { "type": ["integer","null"] },
    "promoted_at":              { "type": ["integer","null"] }
  }
}
```

### 19.4 Tendril

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://qrrune.local/schemas/tendril.json",
  "title": "Tendril",
  "type": "object",
  "required": ["id","source_symbol","target_symbol","weight","tendril_type","created_at"],
  "properties": {
    "id":              { "type": "string", "format": "uuid" },
    "source_symbol":   { "type": "string", "format": "uuid" },
    "target_symbol":   { "type": "string", "format": "uuid" },
    "weight":          { "type": "number", "minimum": 0.0, "maximum": 1.0 },
    "traffic_count":   { "type": "integer", "minimum": 0, "default": 0 },
    "last_traversed":  { "type": ["integer","null"] },
    "tendril_type":    { "type": "string",
                         "enum": ["association","causal","compositional",
                                  "antithetical","dream_bridge"] },
    "cross_node":      { "type": "boolean", "default": false },
    "source_node_id":  { "type": ["string","null"] },
    "target_node_id":  { "type": ["string","null"] },
    "created_at":      { "type": "integer" }
  }
}
```

### 19.5 MeshMessage

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://qrrune.local/schemas/mesh-message.json",
  "title": "MeshMessage",
  "type": "object",
  "required": ["id","type","sender_node","timestamp","payload"],
  "properties": {
    "id":          { "type": "string", "format": "uuid" },
    "type":        { "type": "string",
                     "enum": ["SYMBOL_BROADCAST","THEORY_PROPOSAL","THEORY_VOTE",
                              "CONSENSUS_REQUEST","TENDRIL_HANDSHAKE","DECAY_NOTICE",
                              "HEARTBEAT","DREAM_FRAGMENT_SHARE"] },
    "sender_node": { "type": "string" },
    "target_node": { "type": ["string","null"], "description": "null = broadcast to all" },
    "timestamp":   { "type": "integer" },
    "ttl":         { "type": "integer", "default": 3,
                     "description": "Max relay hops before discard" },
    "payload":     { "type": "object" },
    "signature":   { "type": ["string","null"],
                     "description": "Ed25519 signature of payload by sender node key" }
  }
}
```

### 19.6 ConsolidationCycleReport

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://qrrune.local/schemas/consolidation-report.json",
  "title": "ConsolidationCycleReport",
  "type": "object",
  "required": ["cycle_id","started_at","completed_at","stages"],
  "properties": {
    "cycle_id":     { "type": "string", "format": "uuid" },
    "started_at":   { "type": "integer" },
    "completed_at": { "type": "integer" },
    "stages": {
      "type": "object",
      "properties": {
        "n1_light_sleep": { "type": "object", "properties": {
          "dreams_scanned":     { "type": "integer" },
          "candidates_flagged": { "type": "integer" },
          "flagged_ids":        { "type": "array", "items": { "type": "string" } } } },
        "n3_deep_sleep": { "type": "object", "properties": {
          "candidates_processed":  { "type": "integer" },
          "promoted_to_theory":    { "type": "integer" },
          "merged_with_existing":  { "type": "integer" },
          "rejected":              { "type": "integer" },
          "new_theory_ids":        { "type": "array", "items": { "type": "string" } } } },
        "rem_creative": { "type": "object", "properties": {
          "combinations_attempted": { "type": "integer" },
          "novel_dreams_generated": { "type": "integer" },
          "lucid_dreams_created":   { "type": "integer" },
          "novel_dream_ids":        { "type": "array", "items": { "type": "string" } } } },
        "pruning": { "type": "object", "properties": {
          "dreams_expired":     { "type": "integer" },
          "symbols_decayed":    { "type": "integer" },
          "tendrils_pruned":    { "type": "integer" },
          "radicals_recycled":  { "type": "integer" },
          "storage_freed_bytes": { "type": "integer" } } }
      }
    },
    "health_snapshot": { "type": "object", "properties": {
      "total_symbols":   { "type": "integer" },
      "total_dreams":    { "type": "integer" },
      "total_theories":  { "type": "integer" },
      "total_tendrils":  { "type": "integer" },
      "total_caerns":    { "type": "integer" },
      "avg_trust_score": { "type": "number" },
      "avg_decay_score": { "type": "number" } } }
  }
}
```

---

## 20. API Surface

### 20.1 Library Functions (C ABI)

The RQ^R2 encoder is a standalone library (`librqr2.so`) callable from
`rune_brain` via C ABI:

```
ENCODING
  rqr2_encode(input, config)             → EncodedSymbol*   — text → 14D dream
  rqr2_encode_structured(input, config)  → EncodedSymbol*   — pre-tokenized input

DECODING
  rqr2_decode(symbol, context)           → DecodedMeaning*
  rqr2_decode_multi(symbol, context, max)→ DecodedMeaningArray*

COMPOSITION
  rqr2_compose(radicals, count, config)  → EncodedSymbol*
  rqr2_decompose(symbol)                 → RadicalRefArray*
  rqr2_fuse(symbol_a, symbol_b)          → EncodedSymbol*

DREAM PIPELINE
  rqr2_dream_ingest(input, source_agent) → Dream*
  rqr2_consolidate(config)               → ConsolidationCycleReport*
  rqr2_promote_dream(dream_id)           → Theory*
  rqr2_promote_theory(theory_id)         → EncodedSymbol*

TRUST
  rqr2_test_theory(theory_id, evidence)  → TestResult
  rqr2_get_trust(symbol_id)             → TrustInfo

QUERY
  rqr2_query_by_radical(radical_id, max) → EncodedSymbolArray*
  rqr2_query_by_domain(domain, max)      → EncodedSymbolArray*
  rqr2_query_by_proximity(x,y,z,r,max)   → EncodedSymbolArray*
  rqr2_query_dreams(type, min_conf, max) → DreamArray*
  rqr2_query_theories(status, max)       → TheoryArray*

LIFECYCLE
  rqr2_init(db_path, config)             → RQR2Context*
  rqr2_shutdown(ctx)
  rqr2_free_symbol(symbol)
  rqr2_free_dream(dream)
  rqr2_free_theory(theory)
  rqr2_free_report(report)
```

### 20.2 REST Endpoints (rune_brain :7071)

| Method | Path | Body / Params | Response |
|---|---|---|---|
| `POST` | `/rqr2/encode` | `{input, config?}` | `EncodedSymbol` |
| `POST` | `/rqr2/encode/structured` | `StructuredInput` | `EncodedSymbol` |
| `POST` | `/rqr2/decode` | `{symbol_id, context?}` | `DecodedMeaning` |
| `POST` | `/rqr2/compose` | `{radicals:[…], config?}` | `EncodedSymbol` |
| `POST` | `/rqr2/decompose` | `{symbol_id}` | `RadicalRef[]` |
| `POST` | `/rqr2/fuse` | `{symbol_a_id, symbol_b_id}` | `EncodedSymbol` |
| `POST` | `/rqr2/dreams/ingest` | `{input, source_agent}` | `Dream` |
| `GET` | `/rqr2/dreams` | `?type=&min_confidence=&limit=` | `Dream[]` |
| `POST` | `/rqr2/dreams/{id}/promote` | — | `Theory` |
| `GET` | `/rqr2/theories` | `?status=&limit=` | `Theory[]` |
| `POST` | `/rqr2/theories/{id}/test` | `{evidence_symbol_id}` | `TestResult` |
| `POST` | `/rqr2/theories/{id}/promote` | — | `EncodedSymbol` |
| `GET` | `/rqr2/trust/{symbol_id}` | — | `TrustInfo` |
| `GET` | `/rqr2/symbols` | `?radical=&domain=&trust_state=&limit=` | `EncodedSymbol[]` |
| `POST` | `/rqr2/symbols/proximity` | `{sem_x,sem_y,sem_z,radius,limit}` | `EncodedSymbol[]` |
| `POST` | `/rqr2/consolidate` | `ConsolidationConfig?` | `ConsolidationCycleReport` |
| `GET` | `/rqr2/health` | — | health snapshot |
| `GET` | `/brain/dreams` | `?type=&limit=&min_confidence=` | `Dream[]` (brain alias) |
| `POST` | `/brain/dreams` | `Dream` | `{id}` |
| `GET` | `/brain/theories` | `?status=&limit=` | `Theory[]` (brain alias) |
| `POST` | `/brain/theories` | `Theory` | `{id}` |
| `POST` | `/brain/trust_escalate` | `{id, new_state}` | `{ok, new_state}` |
| `POST` | `/brain/consolidate` | — | `ConsolidationCycleReport` |
| `GET` | `/brain/tendrils` | `?source=&target=&type=&limit=` | `Tendril[]` |
| `POST` | `/brain/tendrils` | `Tendril` | `{id}` |
| `GET` | `/brain/radicals` | `?tier=&domain=` | `Radical[]` |

---

## 21. Example Glyph Encodings

### 21.1 Simple Encoding — "Server crashed from memory leak"

```
INPUT: "Server crashed from memory leak"
TOKENS: [server, crashed, memory, leak]

RADICAL MATCHING:
  "server"  → CR-16 Tower    confidence: 0.85
  "crashed" → CR-54 Error    confidence: 0.92
  "memory"  → CR-32 Remember confidence: 0.78
  "leak"    → CR-08 Water    confidence: 0.71

GRID PLACEMENT:
  A (determinative): CR-54 Error   — fundamentally about a fault
  B (modifier):      CR-16 Tower   — fault is in a server/stack
  C (foundation):    CR-32 Remember — underlying cause is memory
  D (elaboration):   CR-08 Water   — memory is leaking/flowing out

BIND-RUNE FUSION: CR-54 and CR-16 share vertical stave → 1 stroke saved
```

```json
{
  "radicals": [
    {"id":"CR-54","tier":6,"label":"Error",   "grid_position":"A","weight":1.0},
    {"id":"CR-16","tier":2,"label":"Tower",   "grid_position":"B","weight":0.8},
    {"id":"CR-32","tier":4,"label":"Remember","grid_position":"C","weight":0.6},
    {"id":"CR-08","tier":1,"label":"Water",   "grid_position":"D","weight":0.4}
  ],
  "grid": {"A":"CR-54","B":"CR-16","C":"CR-32","D":"CR-08"},
  "sem_x": 0.72, "sem_y": -0.3, "sem_z": 0.6,
  "fractal_depth": 1, "compression_q": 0.2, "ac_ratio": 0.6,
  "trust_state": "dream", "trust_score": 0.56, "domain": "fire",
  "ael": {
    "valence": -0.7, "arousal": 0.85, "dominance": 0.3,
    "novelty": 0.4, "sacredness": 0.05,
    "archetypal_tone": "dissolution", "dream_affect": true
  }
}
```

### 21.2 Compound Encoding — "Mycorrhizal network distributes nutrients"

```
INPUT: "Mycorrhizal network distributes nutrients"

RADICAL MATCHING:
  "mycorrhizal" → CR-04 Fungi + CR-01 Tree  confidence: 0.94 (compound)
  "network"     → CR-44 Bond               confidence: 0.88
  "distributes" → CR-27 Split              confidence: 0.82
  "nutrients"   → CR-46 Feed               confidence: 0.90

GRID (top 4 by score):
  A: CR-04 Fungi   — domain is fungal
  B: CR-01 Tree    — in relationship with trees
  C: CR-44 Bond    — mechanism is connection
  D: CR-46 Feed    — purpose is nourishment

  CR-27 Split stored as secondary_radicals in halo_json

BIND-RUNE: CR-04+CR-44 share curved arc; CR-01+CR-46 share vertical stave
ac_ratio: 0.25 (curved-dominant — organic process)
fractal_depth: 2 (compound containing a compound: fungi+tree)
```

### 21.3 Dream Lifecycle — "I think the cache might be stale"

```
TICK 0   — Input arrives. Hedge words detected ("think", "might").
           Confidence auto-reduced by 0.2.

TICK 1   — Dream created:
           CR-32 Remember (cache→memory) conf: 0.55
           CR-25 Wilt (stale→decay)      conf: 0.48
           Overall confidence: 0.41 | type: hallucination | NOT queryable

TICK 100 — "cache invalidation errors in production" arrives.
           Shares CR-32, CR-25 with dream-001.
           dream-001.consolidation_count → 1

TICK 200 — "TTL expired on session cache" corroborates again.
           dream-001.consolidation_count → 2

TICK 300 — Consolidation cycle (idle):
           Stage N1: dream-001 flagged (count=2, confidence raised to 0.61)
           Stage N3: full decomposition + cross-ref against trusted symbols →
                     PROMOTED TO THEORY (trust_score: 0.45)
           Stage REM: system fuses dream-002 + dream-003 →
                      novel lucid dream: "cache miss due to network delay"

TICK 900 — Theory accumulates 6 corroborations, trust_score crosses 0.75.
           PROMOTED TO TRUSTED SYMBOL. Queryable as answer.
           query("why is the cache stale?") →
             [TRUSTED] "Cache staleness = memory decay pattern" (trust: 0.78)
```

### 21.4 Axiom-Level Glyphs (Hardcoded in Library)

These glyphs are permanently embedded in the RQ^R2 library — immutable,
immune to decay, and serve as ground truth for all theory testing:

| Axiom | Grid | Meaning | Notes |
|---|---|---|---|
| AXIOM-001 | A=CR-57 | "Something exists" | Origin point (0,0,0) |
| AXIOM-002 | A=CR-59 | "Absence / void" | Shadow of AXIOM-001 |
| AXIOM-003 | A=CR-51, B=CR-57 | "Self is real" | The cogito |
| AXIOM-004 | A=CR-55, B=CR-44 | "Events in time are connected" | Causation |
| AXIOM-005 | A=CR-42, B=CR-41 | "Parts form wholes" | Justifies radical composition |
| AXIOM-006–010 | A=CR-01/08/09/07/10 | Elemental truths | "Trees/Water/Fire/Soil/Wind exist" |
| AXIOM-015 | A=CR-38 | "**Dreams are valid input**" | Validates the entire dream architecture |

> AXIOM-015 is the axiom that authorizes the dream-state system. Dreams are not
> bugs — they are an axiomatically sanctioned input class.

### 21.5 Visual Summary — Trust States in a Query Result

```
QUERY: "What do we know about cache behavior?"

RESULT 1 ████████████ [AXIOM]
  "Memory is possible" (AXIOM-012)  trust: 1.00 | radiant
  → Foundation — WHY cache can exist at all

RESULT 2 ██████████░░ [TRUSTED]
  "Cache staleness = memory decay"   trust: 0.78 | vivid
  → Answer — 4 corroborations, 1 contradiction
  → Trail: dream-001 → theory-001 → trusted symbol

RESULT 3 █████░░░░░░░ [THEORY]
  "Cache miss may correlate with network latency"  trust: 0.52
  → ⚠ THEORY — tested 3 times, passed 2

RESULT 4 (not shown to user)
  [DREAM] "cache related to dreaming"  confidence: 0.15
  → Internal only — logged for consolidation
```

---

## 22. Appendix — Dream vs Theory vs Trusted Reference

### A. Complete State Comparison

| Property | Dream | Theory | Trusted | Axiom |
|---|---|---|---|---|
| Trust Range | 0.0 – 0.3 | 0.3 – 0.75 | 0.75 – 0.95 | 1.0 (fixed) |
| Queryable | NO | YES (tagged ⚠) | YES | YES (highest priority) |
| Storage | `dreams` table | `theories` table | `symbols` table | `symbols` table |
| Decay | λ = 0.1 (fast) | λ = 0.01 | λ = 0.001 | λ = 0.0 (none) |
| Shared across mesh | NO | YES (for voting) | YES | YES |
| `compression_q` | 0.2 | 0.5 | 0.8 | 1.0 |
| Entry point | All new encodings | Consolidation cycle | Trust escalation | Hardcoded |
| Exit | Expire / promote | Refute / promote | Decay / recycle | Never |

### B. Same Event at Each Trust Tier

| Event | Dream | Theory | Trusted |
|---|---|---|---|
| Auth failure | `[CR-15?]+[CR-54?]` conf 0.42, internal only | `[CR-15]+[CR-45]+[CR-54]` trust 0.55, tagged ⚠ | `[CR-15]+[CR-45]+[CR-54]+[CR-55]` trust 0.88, answer |
| Cache miss | `[CR-32?]+[CR-25?]` conf 0.43, internal | `[CR-32]+[CR-25]` trust 0.41, ⚠ 2 corroborations | `[CR-32]+[CR-25]+[CR-19]` trust 0.81, answer |
| Outage | `[CR-54?]+[CR-08?]` conf 0.72, internal | `[CR-54]+[CR-08]+[CR-25]` trust 0.63 | `[CR-54]+[CR-08]+[CR-25]+[CR-55]` trust 0.88, answer |

### C. The "TRUST NOTHING" Principle in Action

```
SCENARIO: Node A has a trusted symbol.
          Node B receives it via SYMBOL_BROADCAST.

Node A perspective:  "This is TRUSTED (trust_score: 0.88)"
Node B perspective:  "This is a DREAM (trust_score: 0.0)"

Node B must independently:
  1. Store as dream
  2. Cross-reference against its own trusted symbols
  3. Run its own consolidation cycle
  4. Promote through theory → trusted based on LOCAL evidence

This prevents trust injection attacks and ensures distributed
epistemological integrity.
```

### D. Glossary

| Term | Definition |
|---|---|
| **Axiom** | Immutable ground truth; hardcoded in library; trust = 1.0 |
| **Bind-rune** | Fusion of two radicals that share a structural stroke |
| **Caern** | A hub symbol with ≥10 active tendrils and trust ≥ 0.75 |
| **Consolidation** | The "sleep cycle" — promotes dreams → theories → trusted |
| **Dream** | An unverified encoding fragment; not returned in queries |
| **Fractal depth** | Recursive layers of meaning; -1 = dream (undefined) |
| **Glyph** | A composed symbol in the 2×2 grid; 1–4 radicals |
| **Halo** | The 9-channel metadata envelope carried by every symbol |
| **ILE** | Interpretation Layer Engine; resolves contextual meaning |
| **AEL** | Affective-Emotive Layer; encodes emotional tone of a glyph |
| **Mycelium** | The tendril routing network between symbols |
| **Pocket-galaxy** | The self-organizing knowledge topology (clusters of symbols) |
| **Radical** | An atomic semantic component (CR-01 → CR-60) |
| **Tendril** | A weighted connection between two symbols |
| **Theory** | A promoted dream under evidence testing; queryable but tagged ⚠ |
| **Trust escalation** | Promotion from dream → theory → trusted → axiom |

---

## Related diagrams

- [System Overview](01-system-overview.md)
- [Agent Framework](02-agent-framework.md)
- [Storage Layer](07-storage.md)
- [Compute Pipeline](08-compute-pipeline.md)
- [Mesh Networking](06-mesh-networking.md)
