# Pre-mortem round 2

New failure modes (not covered in round 1):
1. Tuning is calibrated at velocity 0.6 only; at pp/ff pitch drifts several cents (also true of real saxes). → R-015 (accepted)
2. The Harmonic-mode fallback (decision rule) silently swaps in the octave-fingering resonator; tests pass but the
   sound is "less overblown" than intended. → R-016 (disclosure via DEVIATIONS.md + G-003 demo)
3. MSVC rejects a very long string literal when embedding the resonator JSON. → R-017 (S-007 chunking)
