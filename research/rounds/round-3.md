# Research round 3 — adversarial + saturation (2026-10-01)

R5 adversarial (in-context; a fresh-context reviewer was unavailable — logged as tier/context substitution):
- C-010 fingerings: SEO fingering pages contradicted each other (e.g. "low Bb uses right pinky"); rejected
  as Tier 4. WFG chart corrected two planner assumptions (C#4 includes RH low C; Bb4 basic is not unique).
  Remains single-source (text) → R-003, G-004.
- C-017 alto/soprano scaling: no Tier-1 alto geometry found → inferred; mitigated by D-008 per-note tuning
  calibration and T-026 bounds rather than trusting absolute geometry (R-002).
- C-020 pluginval/JUCE 9 compatibility: no evidence either way → verified in S-017 or sandbox build (R-007).
- C-005 TinySOL: dataset itself warns ~20 % retuned/out-of-tune notes → D-015 measures reference f0 instead of
  assuming A440.
- Licence: confirmed Apache-2.0 for repo source is compatible with use of AGPLv3 JUCE at distribution time
  only if distributed binaries comply with AGPLv3 (or a JUCE licence) — distribution out of scope (A-013).
R6: no new testable load-bearing claims.
Saturation: this round produced no new load-bearing claims and no confidence downgrades beyond those
recorded; contradictions resolved. Stop after the protocol minimum of 3 rounds.
Unmet bar (carried to Phase 4): C-010, C-017, C-020, C-021 → R-003, R-002, R-007, R-001.
