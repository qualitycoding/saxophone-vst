# Research question tree

Profile: `software` only. Each leaf names what it informs. Status after round 3.

## Engineering
- E1 Which plugin framework/version, and what licence obligations? → D-001, D-017 — **C-001, C-002, C-003** (done)
- E2 How to pin all dependencies reproducibly? → D-002, T-024 — **C-019, C-022, C-023** (done)
- E3 Linux build prerequisites for JUCE? → plan/ENVIRONMENT.md — **C-002** (done, verified by install)
- E4 How to validate a plugin like a host would? → T-023 — **C-019, C-020** (C-020 verified in sandbox)
- E5 Real-time safety rules (no alloc/locks) and how to test them? → D-010, T-015 (engineering practice; test design)
- E6 Threat surface (state blobs, data files)? → T-006, T-017 (done: fuzz + validation)

## Instrument & acoustics (feeds software design, not a math profile)
- A1 Alto range and transposition? → T-001 — **C-004** (done)
- A2 Standard fingerings incl. low B/Bb, C#4, palm/side keys? → T-003, G-004 — **C-010** (single-source → R-003)
- A3 Which octave vent opens for which notes? → T-004 — **C-012** (done)
- A4 A real-time-capable physical model with published parameters? → D-005 — **C-006, C-008** (done, spike-verified)
- A5 What physically produces overblowing, and can a control drive it? → D-006, D-007 — **C-007, C-009, C-014** (done)
- A6 Where do per-fingering resonator parameters come from for an alto? → D-009 — **C-011, C-016, C-017, C-018** (C-017 inferred → R-002)
- A7 Expected intonation of overblown/second-register notes? → T-012 tolerance — **C-015** (done)
- A8 What is needed beyond the exciter-resonator for realistic radiated sound? → D-014 — **C-021** (single-source → R-001)

## Evaluation
- V1 Openly licensed alto-sax reference recordings? → D-015, T-022b — **C-005** (done)
- V2 Defensible objective realism metrics/thresholds? → A-012, T-022b — no Tier-1 standard exists; planner assumption A-012 + human gate G-003 (→ R-004)
