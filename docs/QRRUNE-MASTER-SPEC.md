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

Every glyph radical belongs to one of two primary stroke families: **Angular** (hard-edged,
algorithmic, structural) or **Curved** (organic, relational, living). This partition is not
aesthetic — it is semantic.

### 4.1 The Partition Axiom

```
AXIOM: SEMANTIC DUALITY
  Angular strokes  → deterministic, computational, structural concepts
  Curved strokes   → probabilistic, biological, relational concepts
  Mixed glyphs     → hybrid domains (AI, trust, memory)
```

### 4.2 Angular Domain Catalogue

| Code | Stroke Form | Semantic Domain | Example Concept |
|------|------------|-----------------|-----------------|
| ANG-D01 | Single vertical bar | Identity, uniqueness | Node ID |
| ANG-D02 | Double horizontal | Equality, parity | Hash match |
| ANG-D03 | Nested right angles | Recursion, containment | Self-reference |
| ANG-D04 | X-cross | Error, cancellation | Quarantine |
| ANG-D05 | Grid cross | Network intersection | Mesh node |
| ANG-D06 | Stacked chevrons | Hierarchy, ordering | Priority queue |
| ANG-D07 | Hollow square | Boundary, scope | Namespace |
| ANG-D08 | Diagonal slash | Division, cut | Partition |

### 4.3 Curved Domain Catalogue

| Code | Stroke Form | Semantic Domain | Example Concept |
|------|------------|-----------------|-----------------|
| CRV-D01 | Sine wave | Flow, rhythm | Heartbeat |
| CRV-D02 | Spiral | Growth, recursion | Mycelium spread |
| CRV-D03 | Arc (open) | Possibility, potential | Pending |
| CRV-D04 | Full circle | Completion, wholeness | Cycle done |
| CRV-D05 | Teardrop | Origin, source | Seed node |
| CRV-D06 | Figure-eight | Infinity, balance | Trust loop |
| CRV-D07 | S-curve | Transition, bridge | State change |
| CRV-D08 | Tendril hook | Connection, attachment | Edge in mesh |

### 4.4 Mixed-Domain (Hybrid) Radicals

Mixed glyphs are constructed by overlaying one angular and one curved stroke. They encode
concepts that live at the boundary of the two worlds.

| Code | Composition | Concept |
|------|-------------|---------|
| HYB-01 | ANG-D01 + CRV-D04 | Agent identity (fixed but alive) |
| HYB-02 | ANG-D05 + CRV-D08 | Peer connection in mesh |
| HYB-03 | ANG-D07 + CRV-D02 | Bounded growth (strategy execution) |
| HYB-04 | ANG-D04 + CRV-D06 | Trust under quarantine |
| HYB-05 | ANG-D06 + CRV-D01 | Ordered flow (event queue) |

### 4.5 Visual Rendering Rules

```
RENDERING PRIORITY (z-order):
  1. Background grid or halo (if present)
  2. Curved strokes (lower layer)
  3. Angular strokes (upper layer)
  4. Wedge terminals / diacritical dots (topmost)

STROKE WEIGHT ENCODING:
  Thin  (1px)  → whisper, background process, low priority
  Medium (2px) → normal operation
  Bold  (3px)  → alert, primary concept, high trust
```

---

## 5. Movement & Relationship Glyphs

Movement glyphs encode the *dynamics* between entities — they are the verbs of the QRrune
language. Relationship glyphs encode *structural bonds* — the prepositions and conjunctions.

### 5.1 Movement Glyph Taxonomy

```
MOVEMENT GLYPHS
═══════════════════════════════════════════

Directional Flow:
  MOV-01 │ →   Propagate forward (event emission)
  MOV-02 │ ←   Return / respond
  MOV-03 │ ↑   Escalate / level-up
  MOV-04 │ ↓   Cascade / delegate
  MOV-05 │ ↔   Bidirectional sync
  MOV-06 │ ↕   Full exchange (request + response)
  MOV-07 │ ↻   Recycle / GC
  MOV-08 │ ↯   Interrupt / force stop

Transformation:
  MOV-09 │ ⇒   Transform (input → output)
  MOV-10 │ ⇔   Fuse (merge two into one)
  MOV-11 │ ⇒⇒  Pipeline (multi-stage)
  MOV-12 │ ⊃   Absorb (incorporate)
  MOV-13 │ ⊂   Spawn from (derive)

State Transitions:
  MOV-14 │ ○→● Active (pending → running)
  MOV-15 │ ●→✓ Complete (running → done)
  MOV-16 │ ●→✗ Fail (running → error)
  MOV-17 │ ✗→○ Retry (error → pending)
```

### 5.2 Relationship Glyph Taxonomy

```
RELATIONSHIP GLYPHS
═══════════════════════════════════════════

Structural:
  REL-01 │ ─── Peer (equal connection)
  REL-02 │ ═══ Strong bond (high trust)
  REL-03 │ ··· Weak bond (low trust / candidate)
  REL-04 │ ─┬─ Branch (one-to-many)
  REL-05 │ ─┴─ Merge (many-to-one)
  REL-06 │ ─╫─ Cross-link (graph edge)

Temporal:
  REL-07 │ ⟶   Precedes (A before B)
  REL-08 │ ⟵   Follows from (B after A)
  REL-09 │ ∥   Concurrent (A ∥ B)
  REL-10 │ ⊏   Contained within (A ⊂ B timeline)

Semantic:
  REL-11 │ ≡   Equivalent (same meaning, different form)
  REL-12 │ ≈   Similar (related meaning)
  REL-13 │ ≠   Contrast (opposite domain)
  REL-14 │ ⊕   Complement (A + B = whole)
  REL-15 │ ⊖   Deficit (A without B)
```

### 5.3 Compound Movement Expressions

Movement and relationship glyphs can be chained into expressions:

```
EXAMPLE: Agent A routes an event to Agent B which transforms it:
  [A] →REL-01→ [B] ⇒ [output]

EXAMPLE: Trust propagation through mesh:
  [Node-X] ═══ [Node-Y] ↑ trust(+0.3)

EXAMPLE: Fusion of two runes:
  [RuneAlpha] ⇔ [RuneBeta] ⇒ [RuneGamma]
```

---

## 6. World-Entity Ontology

The World-Entity Ontology defines the fundamental categories of *things that exist* in the
QRrune universe. Every glyph that refers to a real-world or system concept must be anchored to
an ontological class.

### 6.1 Ontological Hierarchy

```
WORLD-ENTITY TREE
═══════════════════════════════════════════

ROOT: EXISTENCE (∃)
├── AGENT          — any autonomous acting entity
│   ├── human
│   ├── ai-agent
│   ├── daemon
│   └── collective
├── NODE           — any network-addressable location
│   ├── compute-node
│   ├── storage-node
│   ├── edge-node
│   └── caern-node
├── ARTIFACT       — any produced or storable object
│   ├── glyph
│   ├── rune
│   ├── strategy
│   ├── checkpoint
│   └── symbol
├── PROCESS        — any ongoing dynamic
│   ├── event
│   ├── task
│   ├── workflow
│   └── tick
├── RELATIONSHIP   — any bond between entities
│   ├── trust-bond
│   ├── peer-link
│   ├── parent-child
│   └── fusion
└── CONCEPT        — any abstract idea
    ├── knowledge
    ├── intent
    ├── domain
    └── pattern
```

### 6.2 Entity Radicals

Each ontological class has a canonical determinative radical used as a prefix:

| Class | Radical | Code | Visual Form |
|-------|---------|------|-------------|
| AGENT | ◈ | ENT-AG | Diamond with inner cross |
| NODE  | ⬡ | ENT-ND | Hexagon |
| ARTIFACT | ⬜ | ENT-AF | Square |
| PROCESS | ◁▷ | ENT-PR | Double chevron |
| RELATIONSHIP | ─◎─ | ENT-RL | Circle on wire |
| CONCEPT | ☁ | ENT-CO | Cloud |

### 6.3 Entity Properties

All entities carry a standard property envelope:

```json
{
  "id":        "string (UUID)",
  "class":     "AGENT | NODE | ARTIFACT | PROCESS | RELATIONSHIP | CONCEPT",
  "label":     "string",
  "trust":     "float [0.0–1.0]",
  "active":    "boolean",
  "created_at":"ISO-8601",
  "layer":     "string (ontological layer name)",
  "glyphs":    ["glyph-id", ...]
}
```

---

## 7. Icelandic Morphology Layer

The Icelandic morphology layer contributes principles of **grammatical case** and **declension**
to the QRrune encoding system. Icelandic is one of the most morphologically conservative of all
living Germanic languages, preserving a four-case system that maps naturally onto the semantic
roles that glyphs can play in a sentence.

### 7.1 Case Mapping

| Icelandic Case | Grammatical Role | QRrune Application |
|----------------|-----------------|-------------------|
| Nominative | Subject — who acts | The emitting agent of an event |
| Accusative | Direct object — what is acted upon | The payload or target entity |
| Dative | Indirect object — who benefits | The receiving agent or node |
| Genitive | Possession / origin | The source layer or owning namespace |

### 7.2 Case Diacritics

Each glyph can be inflected by appending a case diacritic:

```
CASE DIACRITICS
═══════════════════════════════════════════
  NOM (nominative) │ ˊ   (acute accent, upper-right)
  ACC (accusative) │ ˋ   (grave accent, upper-left)
  DAT (dative)     │ ˆ   (circumflex, top-center)
  GEN (genitive)   │ ˜   (tilde, top-center, wavy)
```

### 7.3 Icelandic Morphological Radicals

Icelandic provides a set of functional morphemes mapped to QRrune function radicals:

| Morpheme | Meaning | Radical |
|----------|---------|---------|
| -inn / -in | The definite article (the X) | DEF ▐ |
| -legur | Adjectival suffix (-like, -ful) | ADJ ∼ |
| -semi | Abstract noun (quality, -ness) | NOM ○ |
| -næni | Sensitivity, receptiveness | RCP ⊾ |
| -skapur | State, condition | STA ≡ |
| veita | To grant, bestow | GRT → |
| geyma | To store, keep | STR ⊡ |
| senda | To send, transmit | SND ⟶ |

### 7.4 Compound Morphological Expressions

```
EXAMPLE: "The trusted node" (nominative)
  [ENT-ND] [TRS: high] [DEF ▐] ˊ
  = hexagon + strong-trust-radical + definite-article + nominative

EXAMPLE: "Sending to the receiving agent" (dative)
  [SND] [ENT-AG] [RCP] ˆ
  = send-radical + agent + receptive-radical + dative-mark
```

---

## 8. Old Norse Runic Morphology

Old Norse runes are the visual and phonological spine of the QRrune glyph aesthetic. The Elder
Futhark (24 runes) and Younger Futhark (16 runes) contribute both stroke forms and semantic
resonance.

### 8.1 Elder Futhark Mappings

The 24 Elder Futhark runes are mapped to QRrune semantic domains, not phonemes:

| Rune | Name | Traditional Meaning | QRrune Domain |
|------|------|--------------------|--------------------|
| ᚠ | Fehu | Cattle, wealth | Resources, assets |
| ᚢ | Uruz | Aurochs, strength | Raw compute power |
| ᚦ | Thurisaz | Giant, thorn | Threat, defense |
| ᚨ | Ansuz | God, mouth | Communication, LLM |
| ᚱ | Raidho | Journey, wheel | Routing, mesh traversal |
| ᚲ | Kenaz | Torch, knowledge | Illumination, search |
| ᚷ | Gebo | Gift, exchange | Mutual trust, fusion |
| ᚹ | Wunjo | Joy, clan | Harmony, consensus |
| ᚺ | Hagalaz | Hail, disruption | Chaos, GC event |
| ᚾ | Nauthiz | Need, necessity | Constraint, dependency |
| ᛁ | Isa | Ice, stillness | Frozen state, lock |
| ᛃ | Jera | Year, harvest | Cycle complete, reward |
| ᛇ | Eihwaz | Yew tree, axis | Core connection, spine |
| ᛈ | Perthro | Lot-cup, mystery | Entropy, randomness |
| ᛉ | Algiz | Elk-sedge, protection | Shield, audit trail |
| ᛊ | Sowilo | Sun, victory | Success, trust peak |
| ᛏ | Tiwaz | Tyr, justice | Governance, protocol |
| ᛒ | Berkano | Birch, growth | Spawn, creation |
| ᛖ | Ehwaz | Horse, partnership | Peer bond, cooperation |
| ᛗ | Mannaz | Man, self | Agent identity |
| ᛚ | Laguz | Water, flow | Data stream, event flow |
| ᛜ | Ingwaz | Ing, completeness | Encapsulation, closure |
| ᛞ | Dagaz | Day, breakthrough | Transformation, dawn event |
| ᛟ | Othala | Estate, heritage | Lineage, provenance |

### 8.2 Bindrune Construction

Bindrunes overlay two or more runes into a single composite symbol. QRrune adopts this for
creating **compound glyphs** that express relationships between two domains:

```
BINDRUNE RULES:
  1. Primary rune forms the vertical spine
  2. Secondary rune shares the spine stroke
  3. Resulting glyph is assigned a new Radical code
  4. Maximum 3 runes per bindrune
  5. Shared strokes are rendered at bold weight
```

### 8.3 Runic Aett Structure

The Elder Futhark is divided into three Aettir (families of 8). QRrune maps each Aett to a
cognitive layer:

| Aett | Runes | Cognitive Layer |
|------|-------|-----------------|
| Freyr's Aett (ᚠ–ᚹ) | Fehu→Wunjo | Physical/resource layer |
| Heimdall's Aett (ᚺ–ᛇ) | Hagalaz→Eihwaz | Process/dynamic layer |
| Tyr's Aett (ᛈ–ᛟ) | Perthro→Othala | Abstract/cognitive layer |

---

## 9. Chinese Radical Logic

The Chinese writing system achieves remarkable information density through a compositional
radical system. QRrune borrows the structural logic — not the characters themselves — to
organize semantic fields.

### 9.1 Radical Classification Principles

Chinese radicals are classified by:
1. **Semantic contribution** — the radical hints at meaning
2. **Phonetic contribution** — (not used in QRrune; we are purely semantic)
3. **Positional role** — left component, top component, enclosure, etc.

QRrune maps these positions to the 2×2 glyph grid:

```
GRID POSITION → SEMANTIC ROLE
  ┌──────────┬──────────┐
  │ TOP-LEFT │ TOP-RIGHT│
  │ DOMAIN   │ MODIFIER │
  │ (det.)   │ (qual.)  │
  ├──────────┼──────────┤
  │ BOT-LEFT │ BOT-RIGHT│
  │ ACTION   │ OBJECT   │
  │ (verb)   │ (noun)   │
  └──────────┴──────────┘
```

### 9.2 QRrune Semantic Field Radicals

Inspired by the 214 Kangxi radicals, QRrune defines a condensed set of 64 semantic field
radicals organized in 8 rows of 8 (a nod to the I Ching's 64 hexagrams):

| Row | Domain | Radicals |
|-----|--------|---------|
| 1 | Body / Self | agent, identity, memory, boundary, health, sense, will, persona |
| 2 | Earth / Space | node, location, distance, path, region, ground, horizon, map |
| 3 | Water / Flow | stream, pool, flood, drop, current, channel, source, delta |
| 4 | Fire / Energy | spark, flame, heat, light, burn, radiate, consume, ignite |
| 5 | Metal / Structure | frame, link, chain, lock, key, forge, alloy, gate |
| 6 | Wood / Growth | seed, root, branch, leaf, fruit, cycle, decay, forest |
| 7 | Time / Sequence | past, present, future, duration, instant, rhythm, epoch, era |
| 8 | Mind / Abstract | know, forget, trust, doubt, plan, dream, symbol, void |

### 9.3 Radical Combination Grammar

When two semantic field radicals combine, the resulting meaning follows a compositional rule:

```
SEMANTIC COMPOSITION:
  [DOMAIN radical] + [ACTION radical] = activity within domain
  [DOMAIN radical] + [OBJECT radical] = entity within domain
  [MODIFIER radical] + [OBJECT radical] = qualified entity
  [ACTION radical] + [OBJECT radical] = verb phrase
```

**Examples:**

| Combination | Result |
|-------------|--------|
| FLOW + LINK | Data stream routing |
| MIND + CHAIN | Memory sequence |
| FIRE + NODE | Active compute node |
| TRUST + AGENT | Verified peer |
| VOID + FUTURE | Unknown state ahead |

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
