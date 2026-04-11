# QRrune / RQ² Cognitive Encoding System — Master Specification

> **Version:** 1.0.0-draft
> **Date:** 2026-04-11
> **Author:** Mitchell Turchyniak
> **License:** Proprietary — All Rights Reserved
> **Status:** Living Document — Canonical Reference

---

## Table of Contents

1. [Introduction & Design Philosophy](#1-introduction--design-philosophy)
2. [Hybrid Ancient-Inspired Glyph System](#2-hybrid-ancient-inspired-glyph-system)
3. [Sumerian Mathematical Glyphs](#3-sumerian-mathematical-glyphs)
4. [Angular vs Curved Glyph Partitioning](#4-angular-vs-curved-glyph-partitioning)
5. [Movement & Relationship Glyphs](#5-movement--relationship-glyphs)
6. [World-Entity Ontology](#6-world-entity-ontology)
7. [Icelandic Morphology Layer](#7-icelandic-morphology-layer)
8. [Old Norse Runic Morphology](#8-old-norse-runic-morphology)
9. [Chinese Radical Logic](#9-chinese-radical-logic)
10. [Japanese Kami Semantic Clusters](#10-japanese-kami-semantic-clusters)
11. [Mycelium Network Layer](#11-mycelium-network-layer)
12. [Brain-Inspired Cognition Model](#12-brain-inspired-cognition-model)
13. [Pocket-Galaxy Knowledge Model](#13-pocket-galaxy-knowledge-model)
14. [AEL Overlays](#14-ael-overlays)
15. [ILE — Interpretation Layer Engine](#15-ile--interpretation-layer-engine)
16. [Distributed Mesh Integration](#16-distributed-mesh-integration)
17. [Metadata Channels](#17-metadata-channels)
18. [Glyph Construction Engine](#18-glyph-construction-engine)
19. [JSON Schemas](#19-json-schemas)
20. [Example Glyph Sets](#20-example-glyph-sets)
21. [Appendices](#21-appendices)

---

## 1. Introduction & Design Philosophy

### 1.1 What Is QRrune / RQ²?

QRrune (also stylized RQ² — Radical Query, Runic Quantum) is a cognitive encoding system that
fuses ancient writing traditions with modern computational semantics. It is not merely a font or
a cipher — it is a living symbolic substrate designed to encode, compress, route, and recall
meaning across agents, languages, and machines.

### 1.2 Core Tenets

| Tenet | Description |
|-------|-------------|
| Radical Composability | Every glyph is built from atomic radicals that carry independent meaning, combinable like chemistry |
| Multi-Dimensional Encoding | Each glyph exists in a 14-dimensional attribute space (color, brightness, ethereal/solid, time, etc.) |
| Biological Resonance | System structures mirror mycelium, neurons, and ecosystems — not arbitrary hierarchies |
| Cultural Respect | Ancient traditions are honored as inspiration, never appropriated or claimed as lineage |
| Deterministic Construction | Glyph assembly follows formal rules; no ambiguity in encoding or decoding |
| Local-First | All encoding/decoding runs locally; no cloud dependency for core operations |
| Forward Compatibility | New glyphs, radicals, and dimensions can be added without breaking existing encodings |

### 1.3 Architectural Overview

```
┌──────────────────────────────────────────────────────────────┐
│                    USER / AGENT INTERFACE                    │
├──────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────────┐ │
│  │   Glyph     │  │     ILE      │  │    AEL Overlays     │ │
│  │ Construction│◄─┤ Interpretation│◄─┤ (Affective-Emotive) │ │
│  │   Engine    │  │ Layer Engine  │  │                     │ │
│  └──────┬──────┘  └──────┬───────┘  └──────────┬──────────┘ │
│         │                │                      │            │
│  ┌──────▼──────────────▼──────────────────────▼──────────┐  │
│  │                  METADATA CHANNEL BUS                  │  │
│  │  [color] [intensity] [halo] [trust] [activation]       │  │
│  │  [connectivity] [temporal] [spatial] [lineage]         │  │
│  └──────┬──────────────────────────────────────────────────┘  │
│         │                                                     │
│  ┌──────▼──────────────────────────────────────────────────┐ │
│  │              COGNITIVE SUBSTRATE LAYERS                  │ │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────────┐ │ │
│  │  │ Mycelium │ │  Brain   │ │  Pocket  │ │   World    │ │ │
│  │  │ Network  │ │Cognition │ │  Galaxy  │ │  Ontology  │ │ │
│  │  └──────────┘ └──────────┘ └──────────┘ └────────────┘ │ │
│  └──────┬──────────────────────────────────────────────────┘ │
│         │                                                     │
│  ┌──────▼──────────────────────────────────────────────────┐ │
│  │               DISTRIBUTED MESH LAYER                    │ │
│  │   [Yggdrasil Overlay] [Agent Routing] [Node Discovery]  │ │
│  └─────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────┘
```

### 1.4 Terminology

| Term | Definition |
|------|-----------|
| Glyph | A complete symbolic unit composed of one or more radicals, carrying encoded meaning |
| Radical | An atomic component of a glyph, carrying a single semantic dimension |
| Stroke | A visual element within a radical (angular, curved, dot, etc.) |
| Cluster | A group of related glyphs sharing a common radical root |
| Halo | A metadata envelope surrounding a glyph, encoding non-visual attributes |
| Caern | A sacred/anchored node in the network where knowledge crystallizes |
| Tendril | A mycelium-inspired connection between glyphs or knowledge nodes |
| Pocket | A self-contained knowledge unit within the galaxy model |
| AEL | Affective-Emotive Layer — emotional/tonal overlay on glyphs |
| ILE | Interpretation Layer Engine — contextual meaning resolver |

---

## 2. Hybrid Ancient-Inspired Glyph System

The glyph system draws structural and philosophical inspiration from three ancient Semitic/Iranian
writing traditions. These are not reproductions — they are design principles extracted from
historical writing systems and reforged for cognitive encoding.

### 2.1 Source Traditions

#### 2.1.1 Phoenician Inspiration

| Aspect | Historical Phoenician | QRrune Adaptation |
|--------|----------------------|-------------------|
| Structure | 22 consonantal letters, linear, right-to-left | Atomic radical set with directional encoding |
| Key Principle | Acrophony — letter represents initial sound of its name | Semantic acrophony — radical represents the initial concept of its domain |
| Stroke Style | Simple, angular, few curves | Angular radicals for structural/logical concepts |
| Contribution | Proto-alphabet simplicity | Minimal stroke count for maximum discriminability |

**Design Extraction:**
- Each radical should be constructible in ≤ 5 strokes
- Radicals must be visually distinct at small scale (16×16 px minimum)
- Semantic acrophony: the radical for "fire" begins with the glyph's fire-stroke

#### 2.1.2 Ugaritic Inspiration

| Aspect | Historical Ugaritic | QRrune Adaptation |
|--------|---------------------|-------------------|
| Structure | 30-letter cuneiform alphabet | Extended radical set with wedge-derived strokes |
| Key Principle | Cuneiform on clay — wedge impressions | Pressure encoding — stroke weight carries meaning |
| Stroke Style | Wedge-shaped, impressed | Triangular terminals on angular radicals |
| Contribution | Bridged pictographic and alphabetic thinking | Hybrid radical-logographic composability |

**Design Extraction:**
- Stroke weight (thin/medium/bold) encodes intensity or certainty
- Wedge terminals distinguish structural radicals from organic ones
- Three-tier pressure: whisper (thin), voice (medium), command (bold)

#### 2.1.3 Old Persian Cuneiform Inspiration

| Aspect | Historical Old Persian | QRrune Adaptation |
|--------|----------------------|-------------------|
| Structure | 36 signs + ideograms + numerals | Mixed radical-ideographic system with numeric integration |
| Key Principle | Royal inscriptions — formal, monumental | Authority encoding — glyph formality signals trust level |
| Stroke Style | Horizontal, vertical, and angled wedges | Grid-aligned construction for machine readability |
| Contribution | Determinatives (semantic classifiers) | Category radicals that prefix glyphs with domain markers |

**Design Extraction:**
- Determinative radicals: a "domain prefix" stroke that classifies the glyph
- Formal vs informal glyph variants signal trust/authority level
- Grid-aligned construction ensures pixel-perfect rendering at any scale

### 2.2 Unified Ancient Stroke Vocabulary

All ancient-inspired strokes are catalogued in the Stroke Codex:

```
STROKE CODEX v1.0
═══════════════════════════════════════════

Category: ANGULAR (Phoenician-derived)
──────────────────────────────────────
  ANG-01 │ Vertical bar        |
  ANG-02 │ Horizontal bar      ─
  ANG-03 │ Forward diagonal    /
  ANG-04 │ Back diagonal       \
  ANG-05 │ Right angle (L)     └
  ANG-06 │ Right angle (Γ)     ┌
  ANG-07 │ Chevron up          ^
  ANG-08 │ Chevron down        v
  ANG-09 │ Cross               +
  ANG-10 │ X-cross             ×

Category: WEDGE (Ugaritic-derived)
──────────────────────────────────────
  WDG-01 │ Horizontal wedge    ◄
  WDG-02 │ Vertical wedge      ▲
  WDG-03 │ Corner wedge        ◣
  WDG-04 │ Double wedge        ◄◄
  WDG-05 │ Micro wedge (dot)   •

Category: GRID (Old Persian-derived)
──────────────────────────────────────
  GRD-01 │ Column stack        ┃┃
  GRD-02 │ Row stack           ═
  GRD-03 │ Box frame           □
  GRD-04 │ Nested box          ⊡
  GRD-05 │ Grid cross          ╬
```

### 2.3 Composition Rules

```
RULE 1: RADICAL STACKING
  Radicals compose LEFT → RIGHT (primary axis)
  then BOTTOM → TOP (secondary axis)
  Maximum: 4 radicals per glyph (2×2 grid)

RULE 2: DETERMINATIVE PREFIX
  A category radical occupies the TOP-LEFT cell
  It classifies but does not modify the glyph's core meaning

RULE 3: STROKE PRIORITY
  Angular strokes render BEFORE curved strokes
  Wedge terminals render LAST (as decorators)

RULE 4: DISAMBIGUATION
  If two glyphs would be visually identical:
    → Add a diacritical dot (WDG-05) at a canonical position
    → Positions: top-center, bottom-center, right-center (in priority order)
```

---

## 3. Sumerian Mathematical Glyphs

Sumerian mathematics operated on a sexagesimal (base-60) system with remarkable positional
notation. QRrune extracts the structural genius of this system for encoding quantities,
relationships, and magnitudes.

### 3.1 Design Principles from Sumerian Math

| Sumerian Concept | QRrune Application |
|-----------------|-------------------|
| Base-60 positional notation | Compact magnitude encoding (60 values per position) |
| Distinct symbols for 1 and 10 | Binary radical pair: unit and bundle |
| Metrological systems (capacity, weight, area) | Domain-specific numeric radicals |
| Reciprocal tables | Inverse-relationship glyphs |
| Sexagesimal fractions | Sub-unit precision encoding |

### 3.2 Numeric Radical Set

```
NUMERIC RADICALS
═══════════════════════════════════════════

Base Units:
  NUM-01 │ Unit (1)            ·
  NUM-02 │ Bundle (10)         ○
  NUM-03 │ Cycle (60)          ◎
  NUM-04 │ Grand Cycle (3600)  ◉

Operators:
  OPR-01 │ Addition            ∧  (convergence)
  OPR-02 │ Subtraction         ∨  (divergence)
  OPR-03 │ Multiplication      ⊗  (cross-circle)
  OPR-04 │ Division            ⊘  (split-circle)
  OPR-05 │ Reciprocal          ↺  (loop-back)
  OPR-06 │ Exponent            ↑· (ascent + unit)

Magnitude Modifiers:
  MAG-01 │ Micro (10⁻³)        ·̣  (sub-dot)
  MAG-02 │ Milli (10⁻²)        ·̤  (under-dot)
  MAG-03 │ Kilo (10³)          ○̂  (over-circle)
  MAG-04 │ Mega (10⁶)          ◎̂  (over-double)
  MAG-05 │ Infinity/unbounded  ∞  (lemniscate)
  MAG-06 │ Zero/null           ∅  (empty set)
```

### 3.3 Sexagesimal Encoding Table

Values 0–59 are encoded as combinations of unit and bundle:

| Value | Encoding      | Glyph Pattern         |
|-------|---------------|-----------------------|
| 0     | ∅             | (empty)               |
| 1     | ·             | single dot            |
| 2     | ··            | two dots horizontal   |
| 3     | ···           | three dots triangular |
| 5     | ·····         | quincunx pattern      |
| 10    | ○             | single circle         |
| 11    | ○·            | circle + dot          |
| 20    | ○○            | two circles           |
| 30    | ○○○           | three circles         |
| 59    | ○○○○○·······  | five circles + nine dots |
| 60    | ◎             | double circle (new position) |

### 3.4 Mathematical Glyph Examples

```
EXAMPLE: Encoding the value 147
─────────────────────────────────
  147 = 2×60 + 27
  147 = 2×60 + 2×10 + 7

  Glyph: ◎·· │ ○○·······
         ───────────────
         pos-1   pos-0
         (2×60)   (27)

EXAMPLE: Encoding a ratio (golden ratio approximation)
──────────────────────────────────────────────────────
  φ ≈ 1.618 ≈ 97/60 = 1 remainder 37

  Glyph: · │ ○○○·······
         ──────────────
         1   37/60

EXAMPLE: Encoding "three-fold increase"
────────────────────────────────────────
  [OPR-03] ⊗ [NUM: ···]
  "multiply by three"
  Rendered: ⊗···
```

### 3.5 Metrological Domain Prefixes

| Domain | Prefix Radical | Meaning |
|--------|---------------|---------|
| Quantity/Count | QNT ┃ | "how many" |
| Distance/Space | DST ═ | "how far" |
| Time/Duration  | TMP ◷ | "how long" |
| Weight/Mass    | WGT ▽ | "how heavy" |
| Temperature    | THR ∿ | "how hot/cold" |
| Trust/Certainty| TRS ◇ | "how sure" |
| Connectivity   | CON ⊞ | "how linked" |

---

## 4. Angular vs Curved Glyph Partitioning

A foundational design axis of QRrune is the angular/curved duality — a binary partition that
encodes the fundamental nature of a concept before any semantic content is read.

### 4.1 The Partition Principle

```
┌─────────────────────────────────────────────────────────────┐
│                       GLYPH UNIVERSE                        │
│                                                             │
│  ┌─────────────────────┐    ┌─────────────────────────┐    │
│  │   ANGULAR DOMAIN    │    │     CURVED DOMAIN        │    │
│  │                     │    │                          │    │
│  │  Structure          │    │  Process                 │    │
│  │  Logic              │    │  Emotion                 │    │
│  │  Boundary           │    │  Flow                    │    │
│  │  Discrete           │    │  Continuous              │    │
│  │  Mineral            │    │  Organic                 │    │
│  │  Built              │    │  Grown                   │    │
│  │  Static             │    │  Dynamic                 │    │
│  │  Defined            │    │  Emergent                │    │
│  │                     │    │                          │    │
│  │  Strokes: │─/\+×    │    │  Strokes: )(~○◠◡∿        │    │
│  └─────────────────────┘    └──────────────────────────┘    │
│                                                             │
│                ┌──────────────────────┐                     │
│                │      HYBRID ZONE     │                     │
│                │  (Mixed strokes)     │                     │
│                │  Transition states   │                     │
│                │  Duality concepts    │                     │
│                └──────────────────────┘                     │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Angular Domain — Detailed

Angular glyphs encode structural, logical, bounded, and constructed concepts.

| Subcategory | Semantic Field | Stroke Profile | Examples |
|-------------|---------------|----------------|----------|
| Architecture | Walls, frames, containers, limits | Right angles, parallel lines | House, boundary, law, rule |
| Logic | Boolean, conditional, causal | Crosses, chevrons, branches | If/then, and/or, because |
| Hierarchy | Rank, order, sequence, priority | Vertical stacks, ladders | Parent, child, root, leaf |
| Measurement | Quantity, distance, precision | Grid lines, tick marks | Count, length, exact, equal |
| Mineral | Stone, metal, crystal, earth-solid | Sharp edges, facets | Rock, iron, gem, salt |
| Technology | Tools, machines, constructs | Interlocking angles | Gear, circuit, bridge, code |

**Angular Stroke Rules:**

```
1. All angles are multiples of 15° (15, 30, 45, 60, 75, 90)
2. No stroke curves — all paths are straight segments
3. Corners are sharp (no rounding)
4. Minimum stroke length: 2 grid units
5. Maximum strokes per radical: 5
```

### 4.3 Curved Domain — Detailed

Curved glyphs encode processual, organic, flowing, and emergent concepts.

| Subcategory | Semantic Field | Stroke Profile | Examples |
|-------------|---------------|----------------|----------|
| Organic | Life, growth, decay, biology | Arcs, spirals, waves | Tree, root, bloom, spore |
| Emotion | Feeling, mood, affect, desire | Flowing curves, swells | Joy, grief, longing, calm |
| Flow | Water, air, current, diffusion | Sine waves, streams | River, wind, breath, pour |
| Cycle | Season, phase, orbit, rhythm | Circles, ellipses, loops | Moon, tide, heartbeat, year |
| Relation | Bond, kinship, affinity, mesh | Intertwined curves | Love, friendship, symbiosis |
| Music | Sound, vibration, harmony, tone | Waveforms, resonance arcs | Song, echo, chord, silence |

**Curved Stroke Rules:**

```
1. All curves are segments of circles or ellipses (Bézier control points allowed)
2. No sharp corners — all junctions are smooth (G1 continuity minimum)
3. Minimum curve radius: 1 grid unit
4. Spirals rotate counterclockwise by convention (clockwise = inverse)
5. Maximum strokes per radical: 5
```

### 4.4 Hybrid Zone

Some concepts inherently span both domains. These use mixed stroke construction:

```
HYBRID CONSTRUCTION RULES
═════════════════════════
1. Angular strokes form the LEFT/BOTTOM foundation
2. Curved strokes form the RIGHT/TOP elaboration
3. The junction point is marked with a transition dot (•)
4. Reading order: angular base → transition → curved extension

EXAMPLES OF HYBRID CONCEPTS:
  "bridge"       = angular (structure) + curved (connection)
  "algorithm"    = angular (logic) + curved (flow)
  "metamorphosis"= angular (before-state) + curved (becoming)
  "lightning"    = angular (energy) + curved (path)
  "melody"       = angular (note/structure) + curved (expression)
```

### 4.5 Partition Decision Tree

```
Is the concept primarily about STRUCTURE or PROCESS?
│
├─ STRUCTURE → Angular
│   ├─ Is it bounded/finite? → Pure Angular
│   └─ Does it transform?    → Hybrid (angular-base)
│
├─ PROCESS → Curved
│   ├─ Is it continuous/flowing? → Pure Curved
│   └─ Does it have fixed form?  → Hybrid (curved-base)
│
└─ BOTH EQUALLY → Hybrid
    └─ Which aspect is more fundamental?
        ├─ Structure → Angular-base hybrid
        └─ Process   → Curved-base hybrid
```

### 4.6 Angular-Curved Ratio as Metadata

Every glyph carries an AC-ratio (Angular-Curved ratio) as metadata:

| AC-Ratio | Classification | Interpretation |
|----------|---------------|----------------|
| 1.00 | Pure Angular | Fully structural/logical |
| 0.75 | Angular-dominant Hybrid | Mostly structural, some flow |
| 0.50 | Balanced Hybrid | Equal structure and process |
| 0.25 | Curved-dominant Hybrid | Mostly processual, some structure |
| 0.00 | Pure Curved | Fully organic/flowing |

---

## 5. Movement & Relationship Glyphs

Movement and relationship glyphs encode verbs, transitions, connections, and spatial/temporal
dynamics — the grammar of the glyph language.

### 5.1 Movement Glyph Categories

#### 5.1.1 Directional Movement

```
DIRECTIONAL PRIMITIVES
═══════════════════════

Cardinal:
  MOV-N  │ Ascent / upward         ↑  Growth, promotion, emergence
  MOV-S  │ Descent / downward      ↓  Gravity, decline, grounding
  MOV-E  │ Forward / advance       →  Progress, future, output
  MOV-W  │ Backward / retreat      ←  Memory, past, input

Diagonal:
  MOV-NE │ Ascent-forward          ↗  Aspiration, optimization
  MOV-NW │ Ascent-backward         ↖  Reflection-growth, nostalgia
  MOV-SE │ Descent-forward         ↘  Entropy, delegation, release
  MOV-SW │ Descent-backward        ↙  Deep memory, archaeology

Axial:
  MOV-IO │ Inward                  ⊙  Focus, compression, internalize
  MOV-OI │ Outward                 ⊕  Broadcast, expansion, externalize
  MOV-CW │ Clockwise rotation      ↻  Natural cycle, forward-time
  MOV-CC │ Counter-clockwise       ↺  Reverse, undo, anti-pattern
```

#### 5.1.2 Velocity & Manner

| Modifier | Symbol | Meaning | Application |
|----------|--------|---------|-------------|
| VEL-STILL | ◇ | Stationary / at rest | Meditation, storage, equilibrium |
| VEL-DRIFT | ◇~ | Slow, ambient movement | Diffusion, passive spread |
| VEL-WALK | ◇~~ | Steady, deliberate pace | Normal processing, routine |
| VEL-RUSH | ◇~~~ | Rapid, urgent movement | Priority routing, emergency |
| VEL-FLASH | ◇⚡ | Instantaneous | Interrupt, quantum-hop, cache-hit |
| MNR-SMOOTH | ≈ | Continuous, uninterrupted | Stream, flow, pipeline |
| MNR-PULSE | ⋮ | Rhythmic, periodic | Heartbeat, polling, cron |
| MNR-STAGGER | ⋰ | Irregular, bursty | Event-driven, sporadic |
| MNR-SPIRAL | @ | Helical, returning-but-advancing | Learning loops, recursive refinement |

#### 5.1.3 Transformation Movement

```
TRANSFORMATION PRIMITIVES
══════════════════════════

  TRN-BECOME │ ◇→◆  Metamorphosis (state A becomes state B)
  TRN-SPLIT  │ ◆→◇◇ Fission (one becomes many)
  TRN-MERGE  │ ◇◇→◆ Fusion (many become one)
  TRN-CYCLE  │ ◇→◆→◇ Reversible transformation
  TRN-SHED   │ ◆→◆' Shedding (entity persists, attribute lost)
  TRN-ABSORB │ ◆+◇→◆⁺ Absorption (entity gains attribute)
  TRN-DECAY  │ ◆→·  Dissolution (entity breaks to particles)
  TRN-EMERGE │ ·→◆  Emergence (particles coalesce to entity)
```

### 5.2 Relationship Glyph Categories

#### 5.2.1 Structural Relationships

| Glyph | Symbol | Meaning | Example |
|-------|--------|---------|---------|
| REL-PARENT | ┬ | Hierarchical parent | Class → subclass |
| REL-CHILD | ┴ | Hierarchical child | Instance → class |
| REL-SIBLING | ├┤ | Same-level peer | Co-workers, array elements |
| REL-CONTAIN | ⊃ | Contains / encloses | Set membership, folder → file |
| REL-WITHIN | ⊂ | Is contained by | Element → set |
| REL-BRIDGE | ╌ | Connects across gap | API, translator, adapter |
| REL-MIRROR | ⌐¬ | Reflects / duals | Encryption ↔ decryption |

#### 5.2.2 Dynamic Relationships

| Glyph | Symbol | Meaning | Example |
|-------|--------|---------|---------|
| REL-FEED | ≻ | Nourishes / supplies | Data source → processor |
| REL-DRAIN | ≺ | Consumes / depletes | Sink, garbage collector |
| REL-SYMBIOSIS | ∞ | Mutual benefit loop | Agent cooperation |
| REL-PARASITE | ⊁ | One-sided drain | Resource leak, bloatware |
| REL-COMPETE | ⊗ | Contention for shared resource | Lock contention, race condition |
| REL-GUARD | ⊡ | Protects / shields | Firewall, validator |
| REL-OBSERVE | ◎ | Watches without affecting | Logger, monitor, readonly |

#### 5.2.3 Temporal Relationships

| Glyph | Symbol | Meaning | Example |
|-------|--------|---------|---------|
| TMP-BEFORE | ◁ | Precedes in time | Prerequisite, cause |
| TMP-AFTER | ▷ | Follows in time | Consequence, effect |
| TMP-DURING | ◁▷ | Co-temporal / simultaneous | Parallel execution |
| TMP-UNTIL | ◁⊣ | Persists until condition | Timeout, await, watch |
| TMP-SINCE | ⊢▷ | Active from a point | Uptime, session start |
| TMP-EPOCH | ⊢⊣ | Bounded time window | Transaction, scope |
| TMP-ETERNAL | ○ | No temporal bound | Constant, axiom, law |

### 5.3 Composing Movement + Relationship

Movement and relationship glyphs combine to form verb phrases:

```
COMPOSITION SYNTAX:
  [SUBJECT] + [RELATIONSHIP] + [MOVEMENT] + [OBJECT]

EXAMPLES:

  "Agent feeds data upstream"
  ◆agent + REL-FEED(≻) + MOV-N(↑) + ◆data
  Rendered: ◆≻↑◆

  "Knowledge merges and spirals into understanding"
  ◆knowledge + TRN-MERGE(◇◇→◆) + MNR-SPIRAL(@) + ◆understanding
  Rendered: ◇◇→◆@◆

  "Guardian observes the boundary eternally"
  ◆guardian + REL-OBSERVE(◎) + REL-GUARD(⊡) + TMP-ETERNAL(○)
  Rendered: ◆◎⊡○
```

---

## 6. World-Entity Ontology

The World-Entity Ontology defines the ten primordial domains of the natural world. Every
concept in QRrune can trace at least one root to these domains — they are the ground-truth
metaphors from which all meaning grows.

### 6.1 The Ten Domains

```
THE WORLD WHEEL
═══════════════════════════════════════════════════
                       🌬 WIND
                    · · · · ·
               🔥 FIRE         💧 WATER
             · ·
         🌸 FLOWERS     🌿 VINES
           · ·
         🍄 FUNGI    ◎ CORE    🐛 BUGS
           · ·
         🌍 SOIL       🐦 BIRDS
             · ·
               🌳 TREES · · ·
                    · · · ·

  The domains form a wheel — adjacent domains share
  affinity; opposite domains create tension/contrast.
═══════════════════════════════════════════════════
```

### 6.2 Domain Definitions

| # | Domain | Radical | Element | Semantic Core | Glyph Class |
|---|--------|---------|---------|---------------|-------------|
| 1 | Trees | 🜁 | Wood | Structure, permanence, branching hierarchy, sheltering | Angular-dominant |
| 2 | Bugs | 🜂 | Chitin | Industriousness, swarm intelligence, persistence, modularity | Hybrid |
| 3 | Birds | 🜃 | Air-bone | Vision, freedom, migration, song, messenger patterns | Curved-dominant |
| 4 | Fungi | 🜄 | Mycelium | Hidden networks, decomposition, recycling, symbiosis | Curved |
| 5 | Vines | 🜅 | Tendril | Connection, climbing, binding, parasitism, adaptation | Hybrid |
| 6 | Flowers | 🜆 | Petal | Beauty, signaling, reproduction, attraction, ephemera | Curved |
| 7 | Soil | 🜇 | Earth | Foundation, memory, decay-into-renewal, substrate | Angular |
| 8 | Water | 🜈 | Fluid | Flow, purification, erosion, depth, reflection | Curved |
| 9 | Fire | 🜉 | Plasma | Transformation, energy, destruction-creation, urgency | Hybrid |
| 10 | Wind | 🜊 | Gas | Invisibility, force, breath, communication, dispersal | Curved |

### 6.3 Domain Affinities & Tensions

```
AFFINITY MATRIX (adjacent = affinity, opposite = tension)
══════════════════════════════════════════════════════════

         TREE BUG BIRD FUNG VINE FLOW SOIL WATR FIRE WIND
  TREE ──  +   ·   +    +   ·    +   ·    ─   ·
  BUG   +  ──  ─   +    ·   +    +   ·    ·   ·
  BIRD  ·  ─  ──   ·    ·   ·    ·   ·    ·   +
  FUNG  +  +   ·  ──    +   ·    +   +    ─   ·
  VINE  +  ·   ·   +   ──   +    ·   +    ·   ·
  FLOW  ·  +   ·   ·    +  ──    ·   +    ─   +
  SOIL  +  +   ·   +    ·   ·   ──   +    +   ─
  WATR  ·  ·   ·   +    +   +    +  ──    ─   +
  FIRE  ─  ·   ·   ─    ·   ─    +   ─   ──   +
  WIND  ·  ·   +   ·    ·   +    ─   +    +  ──

  Key: + affinity   ─ tension   · neutral
```

### 6.4 Ontological Mapping to Computing Concepts

| Domain | Computing Metaphor | Agent Application |
|--------|--------------------|-------------------|
| Trees | File systems, ASTs, DOM trees, class hierarchies | Structure agent, parser, indexer |
| Bugs | Microservices, worker threads, task queues | Swarm workers, batch processors |
| Birds | Message brokers, observers, event dispatchers | Messenger agent, notification system |
| Fungi | Distributed caches, gossip protocols, shared state | Mycelium router, consensus layer |
| Vines | Dependency injection, middleware chains, plugins | Plugin manager, adapter layer |
| Flowers | UI components, API surfaces, documentation | Interface agent, presentation layer |
| Soil | Databases, persistent storage, archive | Memory agent, long-term storage |
| Water | Streams, pipelines, ETL flows | Data pipeline, transform agent |
| Fire | Compilers, optimizers, garbage collectors | Forge agent, optimizer, purifier |
| Wind | Network I/O, broadcast, pub/sub | Network agent, broadcast system |

### 6.5 Entity Radical Construction

Each domain contributes a determinative radical used to prefix glyphs:

```
DOMAIN RADICALS — CONSTRUCTION RULES
═════════════════════════════════════

  🜁 TREE radical:  ┃ with branching fork ┣
     Strokes: ANG-01 + ANG-06  (2 strokes, angular)
     Position: top-left determinative cell

  🜂 BUG radical:   ╳ with legs ╪
     Strokes: ANG-10 + ANG-02  (2 strokes, hybrid)
     Position: top-left determinative cell

  🜃 BIRD radical:  ∧ with trailing arc ∧~
     Strokes: ANG-07 + CRV-01  (2 strokes, curved-lean)
     Position: top-left determinative cell

  🜄 FUNGI radical: ◠ with descending threads ◠┊
     Strokes: CRV-02 + ANG-01×n  (variable, curved)
     Position: top-left determinative cell

  🜅 VINE radical:  ~ with anchor point ~•
     Strokes: CRV-01 + WDG-05  (2 strokes, hybrid)
     Position: top-left determinative cell

  🜆 FLOWER radical: ✿ simplified to ◎ with petals
     Strokes: CRV-03 + CRV-04×4  (5 strokes, curved)
     Position: top-left determinative cell

  🜇 SOIL radical:  ═ with texture dots ═·
     Strokes: GRD-02 + WDG-05  (2 strokes, angular)
     Position: top-left determinative cell

  🜈 WATER radical: ≈ double wave
     Strokes: CRV-01×2  (2 strokes, curved)
     Position: top-left determinative cell

  🜉 FIRE radical:  ∧ with interior flicker ∧̃
     Strokes: ANG-07 + CRV-01  (2 strokes, hybrid)
     Position: top-left determinative cell

  🜊 WIND radical:  ))) triple arc stream
     Strokes: CRV-02×3  (3 strokes, curved)
     Position: top-left determinative cell
```

### 6.6 Cross-Domain Compound Entities

When a concept spans two domains, compound radicals are formed:

| Compound | Domains | Meaning | Example |
|----------|---------|---------|---------|
| 🜁+🜄 | Tree+Fungi | Symbiotic partnership | Mycorrhizal network |
| 🜈+🜉 | Water+Fire | Phase transformation | Steam, evaporation |
| 🜊+🜃 | Wind+Bird | Long-distance message | Network broadcast |
| 🜇+🜆 | Soil+Flower | Grounded beauty | Reliable UI |
| 🜂+🜅 | Bug+Vine | Parasitic dependency | Circular dependency |
| 🜈+🜇 | Water+Soil | Erosive memory | Cache invalidation |
| 🜉+🜁 | Fire+Tree | Destructive renewal | Refactoring |
| 🜄+🜊 | Fungi+Wind | Invisible dispersal | Gossip protocol |

---

## 7. Icelandic Morphology Layer

Icelandic is the most conservative living Germanic language — its morphological richness
provides a model for glyph inflection, case-marking, and compositional semantics in QRrune.

### 7.1 Why Icelandic?

| Property | Icelandic Feature | QRrune Extraction |
|----------|------------------|-------------------|
| Case System | 4 cases (nominative, accusative, dative, genitive) | Glyphs inflect by role: agent, patient, instrument, possessor |
| Declension | Strong/weak paradigms, gender agreement | Glyph variant classes with systematic suffix patterns |
| Compounding | Agglutinative compounds (e.g., tölvunarfræði = computer science) | Multi-radical compound glyph construction |
| Vowel Shifts | Umlaut (i-mutation, u-mutation) | Internal radical modification for semantic shifts |
| Preservation | Minimal drift from Old Norse | Long-term stability guarantee for glyph meanings |

### 7.2 Case-Role Mapping

QRrune adopts a four-case inflection system mirroring Icelandic grammar:

```
CASE INFLECTION TABLE
═════════════════════

  NOMINATIVE (nefnifall)  → AGENT role
    Suffix: -∅  (unmarked, base form)
    Meaning: "the one who acts"
    Example: ◆fire  (fire as subject/actor)

  ACCUSATIVE (þolfall)    → PATIENT role
    Suffix: -╴  (right tick)
    Meaning: "the one acted upon"
    Example: ◆fire╴  (fire as object/target)

  DATIVE (þágufall)       → INSTRUMENT role
    Suffix: -╶  (left tick)
    Meaning: "by means of / affected by"
    Example: ◆fire╶  (by means of fire / fire-affected)

  GENITIVE (eignarfall)   → POSSESSOR role
    Suffix: -╷  (bottom tick)
    Meaning: "belonging to / of the nature of"
    Example: ◆fire╷  (of fire / fire's quality)
```

### 7.3 Declension Classes

Glyphs are organized into three declension classes modeled on Icelandic strong/weak paradigms:

| Class | Name | Pattern | Applies To |
|-------|------|---------|-----------|
| I | Strong (sterk) | Full case distinction, all four suffixes unique | Core domain entities (tree, fire, water, etc.) |
| II | Weak (veik) | Reduced case distinction, accusative = dative | Abstract concepts (truth, beauty, logic) |
| III | Invariant (óbeygjanleg) | No inflection, context determines role | Universal constants, operators, punctuation |

### 7.4 Compound Formation Rules

Icelandic-style compounding allows glyphs to form complex meanings:

```
COMPOUND FORMATION
══════════════════

Type 1: HEAD-MODIFIER (determinative)
  Structure: [modifier] + [head]
  Head carries inflection; modifier is bare stem
  Example: ◆water + ◆tree = ◆water·tree  (water-tree = willow)

Type 2: DVANDVA (coordinative)
  Structure: [element-A] + [element-B]  (co-equal)
  Both elements share inflection
  Example: ◆fire + ◆water = ◆fire∧water  (fire-and-water = steam)

Type 3: BAHUVRIHI (possessive/exocentric)
  Structure: [attribute] + [possessor-marker]
  Compound refers to entity POSSESSING the attribute
  Example: ◆strong + ◆root╷ = "the strong-rooted one"

Type 4: CHAIN COMPOUND (agglutinative)
  Structure: [A] + [B] + [C] + ...  (up to 4 elements)
  Only final element carries inflection
  Example: ◆soil·fungi·vine·network = "mycorrhizal vine network"
```

### 7.5 Umlaut — Internal Radical Modification

Borrowing from Icelandic i-umlaut and u-umlaut, QRrune defines radical mutation as a mechanism
for systematic semantic shifts:

| Mutation Type | Trigger | Effect on Radical | Semantic Shift |
|--------------|---------|-------------------|----------------|
| i-mutation | Diminutive / refinement | Curves tighten, scale reduces | General → specific |
| u-mutation | Augmentative / expansion | Curves widen, scale increases | Specific → general |
| a-mutation | Historicizing / archaic | Strokes simplify toward angular | Current → ancestral |
| ö-mutation | Alienation / othering | Strokes mirror horizontally | Familiar → foreign |

```
EXAMPLE: i-mutation of TREE radical
────────────────────────────────────
  Base:        ┃┣  (full tree radical)
  i-mutated:   ┃┤  (smaller fork = specific tree, sapling, branch)

  "tree" → "branch" via i-mutation
```

### 7.6 Number & Definiteness

| Feature | Singular | Plural | Dual (optional) |
|---------|----------|--------|-----------------|
| Marker | -∅ (unmarked) | -═ (double bar) | -┃┃ (twin bars) |
| Meaning | One instance | Multiple instances | Exactly two / paired |
| Example | ◆bird | ◆bird═ | ◆bird┃┃ |

| Feature | Indefinite | Definite |
|---------|-----------|----------|
| Marker | -∅ (unmarked) | -▪ (filled square suffix) |
| Meaning | "a/any" entity | "the/this specific" entity |
| Example | ◆tree (a tree) | ◆tree▪ (the tree) |

---

## 8. Old Norse Runic Morphology

The Elder Futhark and Younger Futhark rune systems encode a phonetic-symbolic duality —
each rune is simultaneously a sound, a name, and a cosmic concept. QRrune extracts this
triadic encoding principle.

### 8.1 Runic Design Principles

| Runic Principle | Historical Function | QRrune Adaptation |
|----------------|--------------------|--------------------|
| Triadic Identity | Each rune = sound + name + concept (e.g., ᚠ = /f/ + fehu + "wealth/cattle") | Each radical = stroke + label + semantic domain |
| Stave Construction | Vertical stave (┃) with branches; carving-optimized (no horizontal on wood grain) | Vertical primary axis; strokes favor diagonals for discriminability |
| Aettir Grouping | 24 runes ÷ 3 families of 8 (Freyr's, Hagal's, Tyr's) | Radical families grouped by ontological domain |
| Bind Runes | Overlapping runes share staves to form compound signs | Radical fusion — shared strokes reduce total stroke count |
| Inversions | Reversed/inverted runes carry altered or opposite meaning | Mirrored radicals encode negation or inversion |
| Magical Intention | Runes carved with purpose activate meaning | Glyph activation metadata — dormant vs active state |

### 8.2 Aettir-Inspired Radical Families

QRrune organizes radicals into three aettir (families), each governing a domain of experience:

```
FIRST AETT — CREATION & SUBSTANCE (Freyr's Aett analog)
═══════════════════════════════════════════════════════
  Governs: Material world, resources, beginnings, primal forces

  Radical │ Name          │ Domain          │ Stroke
  ────────┼───────────────┼─────────────────┼──────────
  R-01    │ Fé (wealth)   │ Resource        │ ┣╲
  R-02    │ Úr (rain)     │ Primal force    │ ┃╲╱
  R-03    │ Þurs (giant)  │ Chaos/entropy   │ ┣>
  R-04    │ Áss (god)     │ Authority       │ ┣╱
  R-05    │ Reið (ride)   │ Journey         │ ┣>╲
  R-06    │ Kaun (sore)   │ Vulnerability   │ ┃<
  R-07    │ Gjöf (gift)   │ Exchange        │ ╳┃
  R-08    │ Vend (joy)    │ Fulfillment     │ ┣╱╲

SECOND AETT — DISRUPTION & TRANSFORMATION (Hagal's Aett analog)
══════════════════════════════════════════════════════════════════
  Governs: Weather, fate, constraint, necessity, transformation

  Radical │ Name          │ Domain          │ Stroke
  ────────┼───────────────┼─────────────────┼──────────
  R-09    │ Hagall (hail) │ Disruption      │ ╬
  R-10    │ Nauð (need)   │ Constraint      │ ┃╲┃
  R-11    │ Ís (ice)      │ Stillness       │ ┃
  R-12    │ Ár (harvest)  │ Cycle/reward    │ ┃╱╲╱
  R-13    │ Sól (sun)     │ Energy/light    │ ╲╱╲
  R-14    │ Týr (Tyr)     │ Justice/order   │ ↑┃
  R-15    │ Björk (birch) │ Growth/renewal  │ ┣>┣>
  R-16    │ Maðr (human)  │ Self/identity   │ ┃╱╲┃

THIRD AETT — CONSCIOUSNESS & COMPLETION (Tyr's Aett analog)
════════════════════════════════════════════════════════════════
  Governs: Mind, communication, completion, transcendence

  Radical │ Name          │ Domain          │ Stroke
  ────────┼───────────────┼─────────────────┼──────────
  R-17    │ Lögr (water)  │ Flow/depth      │ ┃╲
  R-18    │ Yr (yew)      │ Persistence     │ ┃╱╲┃╱
  R-19    │ Algiz (elk)   │ Protection      │ ┃↑╱╲
  R-20    │ Dagr (day)    │ Clarity         │ ╳═
  R-21    │ Óðal (home)   │ Belonging       │ ◇┃
  R-22    │ Ing (seed)    │ Potential       │ ◇
  R-23    │ Erda (earth)  │ Foundation      │ ═┃═
  R-24    │ Wyrd (fate)   │ Destiny/weave   │ ┃╳┃
```

### 8.3 Bind-Rune Fusion Rules

When two or more radicals combine, they follow bind-rune fusion — shared structural elements
merge:

```
BIND-RUNE FUSION ALGORITHM
═══════════════════════════

Step 1: IDENTIFY SHARED STAVES
  If both radicals contain a vertical stave (┃), merge into one stave.

Step 2: ARRANGE BRANCHES
  Left branches from radical-A stay left.
  Right branches from radical-B stay right.
  If both have branches on the same side:
    → Stack vertically (A-branch above B-branch)

Step 3: COLLISION RESOLUTION
  If merged glyph exceeds 7 total strokes:
    → Simplify the less-dominant radical (fewer semantic features)
    → Minimum: retain 2 strokes per radical

Step 4: REGISTER FUSION
  The resulting bind-glyph is logged with its component IDs
  for deterministic decoding.

EXAMPLE: Fé (┣╲) + Nauð (┃╲┃) → Bind: ┣╲┃
  Shared: ┃ (vertical stave) → merged
  Result: wealth-constrained = "scarcity" or "budgeted resource"
```

### 8.4 Inversion Semantics

| Operation | Visual Effect | Semantic Effect |
|-----------|--------------|-----------------|
| Horizontal Mirror | Left ↔ Right | Negation / opposite meaning |
| Vertical Mirror | Top ↔ Bottom | Inversion of hierarchy (root ↔ crown) |
| 180° Rotation | Full flip | Reversal of process direction |
| Stave Removal | Delete the central ┃ | Abstract the concept (remove grounding) |

```
EXAMPLE:
  R-14 Týr (↑┃) = justice, order, lawful authority

  Horizontal mirror: (┃↑ reflected) = injustice, disorder
  Vertical mirror:   (↓┃)           = submission, yielding
  180° rotation:     (┃↓)           = fallen authority, exile
  Stave removal:     (↑)            = abstract ideal of justice (platonic)
```

### 8.5 Activation States

Inspired by the Norse concept of runes being activated through carving with intention:

| State | Marker | Meaning |
|-------|--------|---------|
| Dormant | No marker | Glyph exists but is not currently active/relevant |
| Awakened | Single dot above (˙) | Glyph is loaded in working memory |
| Charged | Double dot above (¨) | Glyph is actively being processed/transformed |
| Released | Ring above (°) | Glyph has completed its purpose, results emitted |
| Sealed | Bar above (¯) | Glyph is locked — cannot be modified or decoded |

---

## 9. Chinese Radical Logic

Chinese characters achieve extraordinary semantic density through radical composition — a
small set of meaning-carrying components combine in predictable positions to generate
thousands of characters. QRrune adopts this architectural genius as its primary composability
engine.

### 9.1 Principles Extracted from Chinese Radicals

| Chinese Principle | Description | QRrune Application |
|-------------------|------------|-------------------|
| Radical + Phonetic | Semantic radical hints at meaning; phonetic component hints at pronunciation | Domain radical (meaning) + modifier radical (specification) |
| Positional Semantics | Radical position matters: left = category, right = specifics; top = abstract, bottom = concrete | 2×2 grid with positional meaning (§9.3) |
| Radical Reuse | ~214 radicals generate 50,000+ characters | ~60 core radicals generate unbounded glyph space |
| Stroke Order | Canonical stroke order ensures consistency and aids recall | Deterministic construction order ensures machine reproducibility |
| Semantic Clustering | Characters sharing a radical share semantic affinity (氵water: 河 river, 湖 lake, 海 sea) | Glyphs sharing a domain radical cluster in semantic space |
| Simplification | Complex → simplified forms for efficiency (龍 → 龙) | Formal → compact glyph variants for bandwidth optimization |

### 9.2 Core Radical Inventory

QRrune defines 60 core radicals organized into six tiers of ten:

```
TIER 1 — ELEMENTAL (from World Ontology domains)
═════════════════════════════════════════════════
  CR-01 Tree   │ CR-02 Bug    │ CR-03 Bird
  CR-04 Fungi  │ CR-05 Vine   │ CR-06 Flower
  CR-07 Soil   │ CR-08 Water  │ CR-09 Fire
  CR-10 Wind

TIER 2 — STRUCTURAL (from Angular domain)
═════════════════════════════════════════════════
  CR-11 Wall/Boundary   │ CR-12 Frame/Container
  CR-13 Path/Channel    │ CR-14 Bridge/Span
  CR-15 Gate/Threshold  │ CR-16 Tower/Stack
  CR-17 Root/Foundation │ CR-18 Branch/Fork
  CR-19 Grid/Matrix     │ CR-20 Knot/Junction

TIER 3 — PROCESSUAL (from Curved domain)
═════════════════════════════════════════════════
  CR-21 Flow/Stream   │ CR-22 Spiral/Cycle
  CR-23 Wave/Pulse    │ CR-24 Bloom/Expand
  CR-25 Wilt/Contract │ CR-26 Merge/Converge
  CR-27 Split/Diverge │ CR-28 Twist/Transform
  CR-29 Echo/Resonate │ CR-30 Drift/Diffuse

TIER 4 — COGNITIVE (from Brain model)
═════════════════════════════════════════════════
  CR-31 Perceive/Sense  │ CR-32 Remember/Store
  CR-33 Decide/Choose   │ CR-34 Create/Generate
  CR-35 Compare/Weigh   │ CR-36 Abstract/Distill
  CR-37 Embody/Ground   │ CR-38 Dream/Imagine
  CR-39 Focus/Attend    │ CR-40 Release/Forget

TIER 5 — RELATIONAL (from Movement/Relationship)
═════════════════════════════════════════════════
  CR-41 Parent/Above   │ CR-42 Child/Below
  CR-43 Sibling/Beside │ CR-44 Bond/Link
  CR-45 Guard/Shield   │ CR-46 Feed/Nourish
  CR-47 Compete/Clash  │ CR-48 Observe/Watch
  CR-49 Carry/Transport│ CR-50 Anchor/Hold

TIER 6 — META (system-level)
═════════════════════════════════════════════════
  CR-51 Agent/Self     │ CR-52 Message/Signal
  CR-53 Rule/Law       │ CR-54 Error/Fault
  CR-55 Time/Clock     │ CR-56 Space/Location
  CR-57 Truth/Verified │ CR-58 Unknown/Query
  CR-59 Null/Void      │ CR-60 Infinity/All
```

### 9.3 Positional Grid Semantics

The 2×2 glyph grid assigns semantic roles by position, mirroring Chinese radical placement:

```
POSITIONAL GRID
═══════════════════════════════════════════
  ┌────────────────┬────────────────┐
  │   TOP-LEFT     │   TOP-RIGHT    │
  │                │                │
  │  DOMAIN        │  MODIFIER      │
  │  (determinative│  (qualifier/   │
  │   radical —    │   adjective —  │
  │   classifies   │   refines the  │
  │   the glyph)   │   core meaning)│
  ├────────────────┼────────────────┤
  │   BOT-LEFT     │   BOT-RIGHT    │
  │                │                │
  │  ACTION        │  OBJECT        │
  │  (verb radical │  (noun radical │
  │   — what is    │   — what is    │
  │   happening)   │   involved)    │
  └────────────────┴────────────────┘

POSITION RULES:
  1. TOP-LEFT is mandatory if a domain radical exists.
  2. BOT-RIGHT is mandatory for entity glyphs.
  3. TOP-RIGHT and BOT-LEFT are optional modifiers.
  4. A glyph with only BOT-RIGHT = pure noun (unclassified entity).
  5. A glyph with only BOT-LEFT = pure verb (unbound action).
```

### 9.4 Stroke Order Canon

Deterministic stroke rendering order ensures identical output across all implementations:

```
CANONICAL STROKE ORDER
═══════════════════════

  1. Horizontal strokes — left to right
  2. Vertical strokes — top to bottom
  3. Diagonal strokes — upper-left to lower-right
  4. Counter-diagonal — upper-right to lower-left
  5. Enclosing strokes — top, then sides, then bottom
  6. Interior strokes — after enclosure is open
  7. Closing strokes — last (seal the enclosure)
  8. Curved strokes — after all angular strokes in same cell
  9. Diacritical marks — after all body strokes
  10. Halo/AEL envelope — outermost, rendered last
```

### 9.5 Compact vs Formal Variants

Every glyph that exceeds 5 total strokes has a compact variant for bandwidth-constrained
contexts (e.g., mesh packet headers):

| Variant | Stroke Budget | Use Context | Lossy? |
|---------|--------------|-------------|--------|
| Formal | Unlimited | Storage, display, audit | No |
| Standard | ≤ 7 strokes | Normal operation | No |
| Compact | ≤ 4 strokes | Mesh routing headers | Minor (modifier dropped) |
| Token | 1–2 strokes | Ultra-low-bandwidth | Yes (domain only) |

---

## 10. Japanese Kami Semantic Clusters

Japanese Shinto cosmology organizes the world into *kami* — divine forces that inhabit all
things. QRrune uses this framework as a **semantic clustering** mechanism: each cluster groups
conceptually related glyphs under a kami-inspired organizing principle.

### 10.1 Primary Kami Clusters

| Kami | Nature | QRrune Cluster | Included Concepts |
|------|--------|----------------|-------------------|
| Izanagi | Creation, structure | FORGE | Build, compile, instantiate, schema |
| Izanami | Dissolution, cycle | RETURN | GC, expiry, death, recycle |
| Amaterasu | Light, clarity | REVEAL | Expose API, log, audit, illuminate |
| Susanoo | Storm, chaos | DISRUPT | Fault, interrupt, chaos event |
| Tsukuyomi | Moon, reflection | MIRROR | Cache, clone, checkpoint, snapshot |
| Inari | Harvest, abundance | YIELD | Output, result, artifact, reward |
| Raijin | Thunder, speed | BURST | Spike, high-priority event, alarm |
| Fujin | Wind, movement | ROUTE | Forward, proxy, mesh-hop, dispatch |
| Benzaiten | Knowledge, art | ENCODE | Glyph construction, symbol, ILE |
| Ebisu | Prosperity, luck | TRUST | Trust delta, bond, reward |

### 10.2 Cluster Glyph Prefixes

Each kami cluster is identified by a cluster prefix radical that precedes all glyphs within it:

```
KAMI CLUSTER PREFIXES
═══════════════════════════════════════════
  FORGE    │ KMI-FRG  │  ⚒  (double hammer)
  RETURN   │ KMI-RTN  │  ↻  (recycle symbol)
  REVEAL   │ KMI-RVL  │  ☀  (radiant circle)
  DISRUPT  │ KMI-DSR  │  ⚡  (lightning bolt)
  MIRROR   │ KMI-MIR  │  ⌗  (viewfinder)
  YIELD    │ KMI-YLD  │  ◇  (diamond open)
  BURST    │ KMI-BRS  │  ❋  (burst star)
  ROUTE    │ KMI-RTE  │  ⟁  (wind arrow)
  ENCODE   │ KMI-ENC  │  ⊛  (circle with star)
  TRUST    │ KMI-TRS  │  ◈  (diamond with dot)
```

### 10.3 Cluster Inheritance

Clusters can be nested. A glyph belonging to REVEAL that also involves TRUST would carry
both prefix radicals, ordered outer-to-inner:

```
  [KMI-RVL] [KMI-TRS] [core glyph]
  = "a trust-revealing action"
  = expose trust score in audit log
```

---

## 11. Mycelium Network Layer

The mycelium network layer is the biological transport model for information in QRrune. It is
inspired by fungal mycelium: a decentralized, self-healing, nutrient-routing network with no
central controller.

### 11.1 Mycelium Topology Primitives

```
MYCELIUM PRIMITIVES
═══════════════════════════════════════════

  HYPHA        │ A single connection strand between two nodes
  ANASTOMOSIS  │ A loop formed when two hyphae merge
  FRUITING BODY│ A node that produces outputs (artifacts, events)
  CAERN        │ A crystallized, persistent knowledge node
  TENDRIL      │ A nascent connection (candidate edge)
  SPORE        │ A lightweight agent or message in transit
  SUBSTRATE    │ The underlying medium (SQLite, memory, mesh)
```

### 11.2 Routing Model

Nutrient (data) flows through hyphae from high-concentration to low-concentration nodes.
In QRrune terms:

```
ROUTING ALGORITHM:
  1. Source node emits a spore (event + payload)
  2. Spore travels along highest-trust hyphae first
  3. At each junction (anastomosis), spore splits to all outbound hyphae
  4. Caern nodes absorb and store spores (persist to SQLite)
  5. Fruiting bodies consume spores and produce artifacts
  6. Dead hyphae (trust < threshold) are pruned by GCAgent
```

### 11.3 Mycelium Glyph Set

```
MYCELIUM GLYPHS
═══════════════════════════════════════════
  MYC-01 │ Hypha (single strand)      ─
  MYC-02 │ Anastomosis (loop)         ◯─
  MYC-03 │ Fruiting body              ⊕
  MYC-04 │ Caern (crystallized node)  ◈
  MYC-05 │ Tendril (candidate)        ·─
  MYC-06 │ Spore (in-transit)         ·>
  MYC-07 │ Substrate                  ⊡
  MYC-08 │ Mycelium cluster           ❋
  MYC-09 │ Dead hypha (pruned)        ─✗
  MYC-10 │ New growth                 ·→
```

### 11.4 Network Health Metrics

Each mycelium network is continuously assessed on:

| Metric | Description | Healthy Range |
|--------|-------------|---------------|
| Hyphal density | Edges per node | 2–8 |
| Caern count | Persistent knowledge nodes | > 3 |
| Spore throughput | Events/sec | Stable variance |
| Pruning rate | Dead hyphae/min | < 10% of total |
| Anastomosis ratio | Loops / total edges | 0.1–0.4 |
| Trust gradient | Variance in edge trust | Low (< 0.3 stddev) |

---

## 12. Brain-Inspired Cognition Model

The brain cognition model maps the cognitive architecture of the QRrune system to biological
neural structures. This is the metaphorical substrate for the `rune_brain` process.

### 12.1 Structural Analogues

| Brain Structure | QRrune Analogue | Implementation |
|----------------|-----------------|----------------|
| Neuron | Rune | `runes` table entry |
| Synapse | Strategy / fusion bond | `fusion_log`, `trust_ledger` |
| Cortical column | Strategy cluster | `strategies` table |
| Hippocampus | Knowledge substrate | `knowledge` + `symbols` tables |
| Prefrontal cortex | EpicRuneAgent / OverwatchAgent | `brain_system.cpp` |
| Microglia | GCAgent | GC sweep cycle |
| Action potential | BrainEvent on EventBus | `EventBus::post()` |
| Long-term potentiation | Trust level increase | `apply_trust_updates()` |
| Synaptic pruning | Quarantine + GC | `quarantine_low_trust_runes()` |
| Memory consolidation | Checkpoint save | `BrainDb::save_checkpoint()` |

### 12.2 Cognitive Cycles

```
COGNITIVE CYCLE (5-second heartbeat):
  ┌──────────────────────────────────┐
  │  1. Sense (HeartAgent tick)      │ ← AllCounts snapshot
  │  2. Integrate (WorkerAgent)      │ ← Process pending events
  │  3. Trust-update (RuneTrust)     │ ← adjust rune trust
  │  4. Prune (GCAgent)              │ ← GC expired/errored runes
  │  5. Reflect (LibrarianAgent)     │ ← store_knowledge, archive audit
  │  6. Act (EpicRuneAgent / Chat)   │ ← emit artifacts
  └──────────────────────────────────┘
```

### 12.3 Cognitive Agent Roster

| Agent | Biological Analogue | Primary Function |
|-------|---------------------|-----------------|
| HeartAgent | Autonomic nervous system | Vital signs, heartbeat events |
| WorkerAgent | Motor cortex | Execute enqueued work |
| GCAgent | Microglia | Prune dead/expired runes and events |
| RuneTrustManager | Basal ganglia | Reward/punish rune trust |
| ChatAgent | Broca's area / Wernicke's area | LLM conversation |
| WalletAgent | Amygdala (trust gatekeeper) | Ed25519 signing, identity |
| NodeAgent | Sensory cortex | Peer health monitoring |
| StrategyAgent | Prefrontal cortex | Strategy evaluation |
| EpicRuneAgent | Hippocampus | Long-term rune synthesis |
| LibrarianAgent | Memory consolidation | Knowledge archival |
| OverwatchAgent | Anterior cingulate cortex | Anomaly detection |

---

## 13. Pocket-Galaxy Knowledge Model

The Pocket-Galaxy model organizes knowledge as a miniature cosmology: a central attractor
(the Caern), orbiting bodies (Pockets), and connecting filaments (Tendrils). It is the
*spatial* complement to the mycelium's *topological* model.

### 13.1 Cosmological Structure

```
POCKET-GALAXY ANATOMY
═══════════════════════════════════════════

  CAERN CORE
    │ The gravitational center; a crystallized knowledge node
    │ Anchored in SQLite: knowledge table + checkpoints
    │
    ├── INNER ORBIT (Hot Pockets — frequently accessed)
    │   ├── Pocket-A: recent events
    │   ├── Pocket-B: active strategies
    │   └── Pocket-C: live runes
    │
    ├── OUTER ORBIT (Cold Pockets — archived knowledge)
    │   ├── Pocket-D: audit archive
    │   ├── Pocket-E: old checkpoints
    │   └── Pocket-F: expired symbols
    │
    └── FILAMENTS (Tendrils between pockets)
        Encoded as: source_pocket → REL → target_pocket
```

### 13.2 Pocket Schema

```json
{
  "pocket_id":   "string",
  "caern_id":    "string",
  "label":       "string",
  "orbit":       "inner | outer",
  "temperature": "hot | warm | cold | frozen",
  "entries":     "integer",
  "last_access": "ISO-8601",
  "tendrils":    [{"target_pocket": "id", "strength": 0.0}]
}
```

### 13.3 Galaxy Navigation

Agents traverse the galaxy using a gravity-weighted search:

```
GALAXY SEARCH ALGORITHM:
  1. Agent emits a query spore with a radical + layer key
  2. Spore attracted to nearest pocket with matching entries
  3. Temperature determines search depth:
       hot    → search inner orbit only
       warm   → inner + outer orbit
       cold   → outer orbit only
       frozen → archived; requires explicit thaw
  4. Results returned ordered by tendril strength × recency
```

---

## 14. AEL Overlays

The Affective-Emotive Layer (AEL) encodes emotional or tonal context as a transparent overlay
on any glyph. AEL is non-destructive — it does not alter the glyph's core semantic content
but adds a dimension that influences interpretation and routing priority.

### 14.1 Affective Dimensions

| Dimension | Range | Description |
|-----------|-------|-------------|
| Valence | −1.0 → +1.0 | Negative (threat) to Positive (reward) |
| Arousal | 0.0 → 1.0 | Calm to Urgent |
| Dominance | 0.0 → 1.0 | Submissive to Authoritative |
| Certainty | 0.0 → 1.0 | Speculative to Confirmed |
| Temporality | −1 / 0 / +1 | Past / Present / Future orientation |

### 14.2 AEL Encoding

AEL values are encoded in the Halo envelope as a 5-tuple:

```
AEL TUPLE FORMAT:
  [valence, arousal, dominance, certainty, temporality]
  Example: [+0.8, 0.9, 0.7, 1.0, 0]
  = "highly positive, urgent, authoritative, certain, present"
  = a confirmed high-priority success event
```

### 14.3 AEL Visual Encoding

| AEL State | Halo Color | Halo Shape | Stroke Weight |
|-----------|------------|------------|---------------|
| Positive + Calm | Amber glow | Soft ring | Medium |
| Positive + Urgent | Gold pulse | Spiky ring | Bold |
| Negative + Calm | Blue tinge | Thin ring | Thin |
| Negative + Urgent | Red flash | Jagged ring | Bold |
| Neutral | None | None | Normal |
| Speculative | Grey haze | Dashed ring | Thin |

### 14.4 AEL Routing Rules

AEL values influence the WorkerAgent event dispatch priority:

```
AEL → PRIORITY MAPPING:
  arousal > 0.8  AND  valence < -0.3  → priority = CRITICAL (0)
  arousal > 0.8  AND  valence >= 0.0  → priority = HIGH     (1)
  arousal 0.4–0.8                     → priority = NORMAL   (2)
  arousal < 0.4                       → priority = LOW      (3)
```

---

## 15. ILE — Interpretation Layer Engine

The Interpretation Layer Engine resolves the *contextual meaning* of a glyph sequence. A
single glyph has a fixed symbolic definition; a sequence of glyphs in context can produce
emergent meaning that neither individual glyph carries alone.

### 15.1 ILE Pipeline

```
ILE INTERPRETATION PIPELINE
═══════════════════════════════════════════

  INPUT: [glyph sequence] + [context object]
  │
  ├─► 1. TOKENIZE
  │       Split sequence into radical tokens
  │
  ├─► 2. RESOLVE CASES
  │       Apply Icelandic case diacritics
  │
  ├─► 3. CLUSTER MATCH
  │       Map radicals to Kami cluster prefixes
  │
  ├─► 4. ONTOLOGY ANCHOR
  │       Bind each token to an ontological class
  │
  ├─► 5. AEL DECODE
  │       Extract affective tuple from halo
  │
  ├─► 6. RELATIONSHIP GRAPH
  │       Build edge list from MOV and REL glyphs
  │
  ├─► 7. MEANING SYNTHESIS
  │       Produce structured JSON meaning object
  │
  └─► OUTPUT: MeaningObject (see §19 for schema)
```

### 15.2 Context Object

The ILE requires a context object to disambiguate polysemous sequences:

```json
{
  "agent":     "string (calling agent name)",
  "layer":     "string (active cognitive layer)",
  "domain":    "string (semantic domain)",
  "timestamp": "ISO-8601",
  "history":   ["prior glyph sequence IDs"],
  "trust":     "float"
}
```

### 15.3 Disambiguation Rules

```
ILE DISAMBIGUATION RULES:
  RULE D-1: If a glyph has multiple ontological anchors, prefer the one
            matching the active layer in the context object.
  RULE D-2: If an AEL tuple carries certainty < 0.3, wrap meaning in
            a "speculative" envelope.
  RULE D-3: If a MOV glyph has no source or target, infer from the
            previous and next non-MOV tokens respectively.
  RULE D-4: Bindrunes are always resolved as a unit before individual
            radical decomposition.
  RULE D-5: Cluster prefixes have higher semantic priority than
            individual radical matches.
```

---

## 16. Distributed Mesh Integration

The distributed mesh layer connects QRrune nodes into a peer network using the Yggdrasil
overlay topology. Each node runs `rune` (:7070) and `rune_brain` (:7071), communicating
over HTTP (httplib) with SSE for real-time event streaming.

### 16.1 Yggdrasil Overlay Topology

Inspired by the Norse world-tree, Yggdrasil organizes nodes into three tiers:

```
YGGDRASIL TIER MODEL
═══════════════════════════════════════════

  ASGARD (Tier 1 — Caern Nodes)
    High-trust, high-memory nodes.
    Store long-term knowledge, serve as anchors.
    Minimum requirements: 4 GB RAM, persistent storage.

  MIDGARD (Tier 2 — Worker Nodes)
    Standard compute nodes.
    Process events, run agent ticks, route spores.
    Minimum requirements: 1 GB RAM.

  HELHEIM (Tier 3 — Edge Nodes)
    Low-resource nodes, IoT, edge compute.
    Relay-only; minimal local storage.
    Minimum requirements: 64 MB RAM.
```

### 16.2 Mesh Protocol

```
MESH COMMUNICATION PROTOCOL
═══════════════════════════════════════════

  Discovery:
    POST /mesh/announce   → register peer (node_id, address, tier)
    GET  /mesh/peers      → list known peers with trust scores

  Event Routing:
    POST /brain/event     → ingest remote event into local WorkerAgent
    GET  /brain/events/stream  → SSE stream for live event subscription

  Knowledge Sync:
    POST /brain/knowledge → push knowledge entry to peer
    GET  /brain/snapshot  → pull full knowledge snapshot

  Trust Exchange:
    POST /trust/record    → submit trust delta for a subject
    GET  /trust/aggregate → retrieve aggregated trust scores
```

### 16.3 Node Identity & Authentication

Each node possesses an Ed25519 keypair (managed by WalletAgent):

```
NODE IDENTITY:
  secret_key: 32-byte seed (stored encrypted locally)
  public_key: 32-byte public key (broadcast on announce)
  node_id:    hex(SHA-256(public_key))[0:16]

MESSAGE SIGNING:
  All outbound mesh messages include:
    X-Node-Id: <node_id>
    X-Signature: hex(ed25519_sign(body_bytes))
  Receiving nodes verify with stored public key.
```

### 16.4 Mesh Glyph Set

```
MESH GLYPHS
═══════════════════════════════════════════
  MSH-01 │ Announce (join mesh)     ⬡→
  MSH-02 │ Peer bond established    ⬡═⬡
  MSH-03 │ Event routed             ·→⬡
  MSH-04 │ Knowledge pushed         ⊡→⬡
  MSH-05 │ Node unreachable         ⬡✗
  MSH-06 │ Trust updated            ⬡◈Δ
  MSH-07 │ Signature verified       ⬡✓
  MSH-08 │ SSE stream active        ⬡~∞
```

---

## 17. Metadata Channels

Every glyph in QRrune can carry metadata through up to 9 channels. Channels are orthogonal —
they do not interfere with the core semantic content of the glyph.

### 17.1 Channel Definitions

| Channel | Code | Type | Description |
|---------|------|------|-------------|
| Color | CH-CLR | enum | Domain color (see palette below) |
| Intensity | CH-INT | float [0,1] | Brightness / signal strength |
| Halo | CH-HAL | AEL tuple | Affective-Emotive envelope |
| Trust | CH-TRS | float [0,1] | Confidence in this glyph's meaning |
| Activation | CH-ACT | float [0,1] | How "live" this glyph is right now |
| Connectivity | CH-CON | integer | Number of active tendrils |
| Temporal | CH-TMP | signed int | Age in ticks (+ = old, − = future) |
| Spatial | CH-SPC | [x,y,z] | Position in pocket-galaxy model |
| Lineage | CH-LIN | string | Origin rune / ancestor glyph ID |

### 17.2 Domain Color Palette

| Domain | Color | Hex | Meaning |
|--------|-------|-----|---------|
| Compute | Electric blue | #00BFFF | Active processing |
| Storage | Warm amber | #FFA500 | Persistent data |
| Trust | Jade green | #00A86B | Trust + verification |
| Threat | Crimson | #DC143C | Threat, error, quarantine |
| Unknown | Neutral grey | #808080 | Unclassified |
| Spiritual | Deep violet | #8B00FF | Kami-layer concepts |
| Flow | Cyan | #00CED1 | Event streams |
| Temporal | Silver | #C0C0C0 | Time-related |
| Growth | Forest green | #228B22 | Expansion, spawn |
| Entropy | Dark orange | #FF4500 | Chaos, disruption |

### 17.3 Channel Encoding in JSON

```json
{
  "glyph_id": "string",
  "channels": {
    "color":       "electric_blue",
    "intensity":   0.85,
    "halo":        [0.7, 0.6, 0.8, 0.9, 0],
    "trust":       0.92,
    "activation":  0.75,
    "connectivity": 4,
    "temporal":    -3,
    "spatial":     [12.4, -3.1, 0.0],
    "lineage":     "rune:ansuz"
  }
}
```

---

## 18. Glyph Construction Engine

The Glyph Construction Engine (GCE) is the deterministic assembly system that converts a
semantic specification into a renderable glyph. It is the runtime implementation of all rules
defined in §2.3, §4.5, and §14.

### 18.1 Construction Pipeline

```
GLYPH CONSTRUCTION PIPELINE
═══════════════════════════════════════════

  INPUT: GlyphSpec (see §19)
  │
  ├─► 1. RADICAL SELECTION
  │       Resolve domain → determinative radical
  │       Resolve action → action radical
  │       Resolve modifiers → modifier radicals
  │
  ├─► 2. GRID PLACEMENT
  │       Assign radicals to 2×2 grid cells
  │       Apply RULE 1–4 from §2.3
  │
  ├─► 3. STROKE RENDERING
  │       Render curved strokes (z=1)
  │       Render angular strokes (z=2)
  │       Render wedge terminals (z=3)
  │
  ├─► 4. CASE DIACRITICS
  │       Apply Icelandic case marks if present
  │
  ├─► 5. HALO GENERATION
  │       Convert AEL tuple → visual halo descriptor
  │
  ├─► 6. CHANNEL OVERLAY
  │       Apply color, intensity, connectivity markers
  │
  └─► OUTPUT: GlyphRenderObject (SVG path + metadata)
```

### 18.2 GlyphSpec Structure

```json
{
  "label":      "string",
  "domain":     "string (semantic domain)",
  "radicals":   ["radical-code", ...],
  "case":       "NOM | ACC | DAT | GEN | null",
  "kami_cluster": "string | null",
  "norse_rune": "string | null",
  "ael":        [0.0, 0.0, 0.0, 1.0, 0],
  "channels":   { }
}
```

### 18.3 Rendering Targets

The GCE supports multiple rendering targets:

| Target | Format | Use Case |
|--------|--------|---------|
| SVG | Vector path | Web UI, scalable display |
| PNG | Raster (16px, 32px, 64px, 128px) | Icons, thumbnails |
| ASCII | Unicode approximation | CLI output, logging |
| JSON-path | Stroke list | Machine-to-machine |
| Hash | SHA-256(canonical JSON) | Deduplication, integrity |

---

## 19. JSON Schemas

This section defines the canonical JSON schemas for all major QRrune data structures.

### 19.1 Radical Schema

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "Radical",
  "type": "object",
  "required": ["code", "category", "form", "domain", "stroke_count"],
  "properties": {
    "code":         { "type": "string", "pattern": "^[A-Z]{2,4}-[0-9]{2}$" },
    "category":     { "enum": ["ANGULAR", "WEDGE", "GRID", "CURVED", "NUMERIC", "OPERATOR", "MOVEMENT", "RELATIONSHIP", "MYCELIUM", "MESH", "KAMI"] },
    "form":         { "type": "string", "description": "Unicode approximation" },
    "domain":       { "type": "string" },
    "stroke_count": { "type": "integer", "minimum": 1, "maximum": 5 },
    "weight":       { "enum": ["thin", "medium", "bold"] }
  }
}
```

### 19.2 Glyph Schema

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "Glyph",
  "type": "object",
  "required": ["glyph_id", "label", "radicals", "channels"],
  "properties": {
    "glyph_id":     { "type": "string", "format": "uuid" },
    "label":        { "type": "string" },
    "radicals":     { "type": "array", "items": { "type": "string" } },
    "case":         { "enum": ["NOM", "ACC", "DAT", "GEN", null] },
    "kami_cluster": { "type": ["string", "null"] },
    "norse_rune":   { "type": ["string", "null"] },
    "channels": {
      "type": "object",
      "properties": {
        "color":        { "type": "string" },
        "intensity":    { "type": "number", "minimum": 0, "maximum": 1 },
        "halo":         { "type": "array", "items": { "type": "number" }, "minItems": 5, "maxItems": 5 },
        "trust":        { "type": "number", "minimum": 0, "maximum": 1 },
        "activation":   { "type": "number", "minimum": 0, "maximum": 1 },
        "connectivity": { "type": "integer", "minimum": 0 },
        "temporal":     { "type": "integer" },
        "spatial":      { "type": "array", "items": { "type": "number" }, "minItems": 3, "maxItems": 3 },
        "lineage":      { "type": ["string", "null"] }
      }
    },
    "render": {
      "type": "object",
      "properties": {
        "svg_path": { "type": "string" },
        "ascii":    { "type": "string" },
        "hash":     { "type": "string", "pattern": "^[0-9a-f]{64}$" }
      }
    }
  }
}
```

### 19.3 MeaningObject Schema (ILE output)

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "MeaningObject",
  "type": "object",
  "required": ["input_sequence", "agent", "timestamp", "entities", "relations", "ael"],
  "properties": {
    "input_sequence": { "type": "string" },
    "agent":          { "type": "string" },
    "timestamp":      { "type": "string", "format": "date-time" },
    "entities": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "token":    { "type": "string" },
          "class":    { "type": "string" },
          "case":     { "type": ["string", "null"] },
          "cluster":  { "type": ["string", "null"] },
          "rune":     { "type": ["string", "null"] }
        }
      }
    },
    "relations": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "from":  { "type": "string" },
          "to":    { "type": "string" },
          "type":  { "type": "string" }
        }
      }
    },
    "ael": {
      "type": "object",
      "properties": {
        "valence":     { "type": "number" },
        "arousal":     { "type": "number" },
        "dominance":   { "type": "number" },
        "certainty":   { "type": "number" },
        "temporality": { "type": "integer" }
      }
    },
    "speculative": { "type": "boolean" },
    "summary":     { "type": "string" }
  }
}
```

### 19.4 NetworkNode Schema

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "NetworkNode",
  "type": "object",
  "required": ["node_id", "tier", "address", "public_key", "trust"],
  "properties": {
    "node_id":    { "type": "string" },
    "tier":       { "enum": ["ASGARD", "MIDGARD", "HELHEIM"] },
    "address":    { "type": "string" },
    "port":       { "type": "integer" },
    "public_key": { "type": "string", "pattern": "^[0-9a-f]{64}$" },
    "trust":      { "type": "number", "minimum": 0, "maximum": 1 },
    "last_seen":  { "type": "string", "format": "date-time" },
    "status":     { "enum": ["online", "unreachable", "quarantined"] }
  }
}
```

---

## 20. Example Glyph Sets

This section provides concrete worked examples of glyph construction for common QRrune
operations.

### 20.1 System Events

| Event | Radicals | Kami | Norse | AEL Summary | ASCII |
|-------|----------|------|-------|-------------|-------|
| Heartbeat | [ENT-PR, CRV-D01] | YIELD | ᛃ Jera | calm/positive | ◁▷∿ |
| GC sweep | [ENT-PR, MOV-07] | RETURN | ᚺ Hagalaz | neutral/certain | ◁▷↻ |
| Rune leveled up | [ENT-AF, MOV-03, TRS▲] | TRUST | ᛊ Sowilo | positive/high | ⬜↑◈ |
| Rune quarantined | [ENT-AF, ANG-D04] | DISRUPT | ᚦ Thurisaz | negative/alert | ⬜× |
| Fusion complete | [ENT-AF, MOV-10] | FORGE | ᚷ Gebo | positive/certain | ⬜⇔ |
| Agent error | [ENT-AG, ANG-D04, MAG-01] | DISRUPT | ᚾ Nauthiz | negative/urgent | ◈✗· |
| Peer connected | [ENT-ND, REL-02] | ROUTE | ᛖ Ehwaz | positive/calm | ⬡═⬡ |
| Knowledge stored | [ENT-AF, STR, MYC-04] | MIRROR | ᛟ Othala | calm/certain | ⊡◈ |

### 20.2 Agent Lifecycle

```
AGENT SPAWN:
  [KMI-FRG] [ENT-AG] ˊ [MOV-14]
  = "FORGE: a new agent is born (nominative), becoming active"
  ASCII: ⚒◈ˊ○→●

AGENT TICK:
  [KMI-YLD] [ENT-PR] [CRV-D01]
  = "YIELD: a process flows rhythmically"
  ASCII: ◇◁▷∿

AGENT SHUTDOWN:
  [KMI-RTN] [ENT-AG] [MOV-16]
  = "RETURN: agent transitions to failure/stop"
  ASCII: ↻◈●→✗
```

### 20.3 Trust Cycle

```
TRUST DELTA POSITIVE:
  [ENT-RL] [TRS+] [MOV-03] [TMP: present]
  = "Relationship: trust increases now"
  ASCII: ─◎─◈↑[0]

TRUST DELTA NEGATIVE:
  [ENT-RL] [TRS−] [MOV-04] [TMP: present]
  = "Relationship: trust decreases now"
  ASCII: ─◎─◈↓[0]

QUARANTINE:
  [ENT-AF] [TRS: < 0.1] [ANG-D04] [KMI-DSR]
  = "Artifact: extremely low trust, cancelled, disrupt-cluster"
  ASCII: ⬜◇✗⚡
```

### 20.4 Mesh Operations

```
NODE ANNOUNCE:
  [MSH-01] [ENT-ND] ˊ [CH-CLR: electric_blue]
  = "Mesh: a new node announces itself (nominative)"
  ASCII: ⬡→

KNOWLEDGE PUSH:
  [MSH-04] [ENT-AF] ˋ [ENT-ND] ˆ
  = "Mesh: knowledge (accusative) pushed to node (dative)"
  ASCII: ⊡→⬡

SIGNATURE VERIFY:
  [MSH-07] [ENT-AG] [WalletAgent]
  = "Mesh: agent verifies signature"
  ASCII: ⬡✓◈
```

---

## 21. Appendices

### Appendix A: Complete Radical Index

All radical codes defined in this specification, sorted alphabetically:

```
ANG-D01 through ANG-D08  — Angular Domain Radicals
ANG-01  through ANG-10   — Stroke Codex: Angular
CH-ACT, CH-CLR, CH-CON, CH-HAL, CH-INT, CH-LIN, CH-SPC, CH-TMP, CH-TRS — Metadata Channels
CRV-D01 through CRV-D08  — Curved Domain Radicals
DST, QNT, THR, TMP, TRS, WGT, CON — Metrological Domain Prefixes
ENT-AF, ENT-AG, ENT-CO, ENT-ND, ENT-PR, ENT-RL — Entity Radicals
GRD-01  through GRD-05   — Stroke Codex: Grid
HYB-01  through HYB-05   — Hybrid Domain Radicals
KMI-BRS, KMI-DSR, KMI-ENC, KMI-FRG, KMI-MIR, KMI-RTE, KMI-RTN, KMI-RVL, KMI-TRS, KMI-YLD — Kami Cluster Prefixes
MAG-01  through MAG-06   — Magnitude Modifiers
MOV-01  through MOV-17   — Movement Glyphs
MSH-01  through MSH-08   — Mesh Glyphs
MYC-01  through MYC-10   — Mycelium Glyphs
NUM-01  through NUM-04   — Numeric Base Units
OPR-01  through OPR-06   — Numeric Operators
REL-01  through REL-15   — Relationship Glyphs
WDG-01  through WDG-05   — Stroke Codex: Wedge
```

### Appendix B: Norse Rune ↔ QRrune Domain Table

(Full 24-entry Elder Futhark mapping — see §8.1)

### Appendix C: Kami Cluster ↔ Brain Agent Mapping

| Kami Cluster | Primary Agent(s) |
|-------------|-----------------|
| FORGE | WorkerAgent, StrategyAgent |
| RETURN | GCAgent |
| REVEAL | LibrarianAgent, OverwatchAgent |
| DISRUPT | OverwatchAgent |
| MIRROR | LibrarianAgent |
| YIELD | HeartAgent, EpicRuneAgent |
| BURST | WorkerAgent (high-priority path) |
| ROUTE | NodeAgent, WorkerAgent |
| ENCODE | ChatAgent, LibrarianAgent |
| TRUST | RuneTrustManager, WalletAgent |

### Appendix D: Changelog

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0-draft | 2026-04-11 | Initial full assembly — all 21 sections |

### Appendix E: Glossary

See §1.4 for core terminology. Additional terms:

| Term | Definition |
|------|-----------|
| Aett | A family of 8 Elder Futhark runes |
| Anastomosis | A loop formed when two mycelium hyphae merge |
| Bindrune | A composite rune formed by overlaying two Elder Futhark runes |
| Caern | A crystallized, persistent knowledge node |
| Hypha | A single mycelium strand connecting two nodes |
| Sexagesimal | Base-60 numbering system (Sumerian origin) |
| Spore | A lightweight message or agent in transit through the mesh |
| Tendril | A nascent or candidate edge in the mycelium/galaxy network |
| Tier | A Yggdrasil network classification: Asgard, Midgard, or Helheim |
| Yggdrasil | The Norse world-tree; metaphor for the three-tier mesh topology |

---

*End of QRRUNE-MASTER-SPEC.md — Version 1.0.0-draft*
