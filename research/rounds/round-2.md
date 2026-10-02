# Research round 2 — depth + empirical (2026-10-01)

R4 depth (Tier 1): read Colinot 2021 in full incl. Tables 1-2 (C-006..C-009, C-013, C-014);
Szwarcberg 2025 in full incl. Table 1 (C-012, C-015, C-016).
R6 empirical spikes:
- S1 `colinot_spike.py`: D# fingering, Fs 176.4 kHz. gamma 0.5/tau 10 ms → 189.1 Hz (first register; paper F#3
  185 Hz); gamma 0.9/tau 3 ms → 381.0 Hz (second register). → C-006, C-009 verified.
- S2 `overblow_map.py`: 3x3 grid (gamma 0.6-0.8, tau 0.1-30 ms): measured f2/f1 2.078 → 4/9 second
  register; f2/f1 = 2.000 → 8/9. → C-007 verified; basis of D-006.
- Environment: pinned versions resolved via GitHub/PyPI APIs (C-019, C-022, C-023); JUCE Linux deps
  installed; Python venv installed from pinned requirements.
R3 synthesis: new leaves A7 (second-register intonation → C-015), A8 (radiation/noise → C-021),
V2 (realism metric thresholds).
