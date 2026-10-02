# Risk register

| ID | Risk | Likelihood | Impact | Mitigation | Residual | Accepted? |
|---|---|---|---|---|---|---|
| R-001 | Sound passes tests but is not convincingly realistic (C-021) | high | high | D-014 output stage; T-022b objective comparison; S-016 calibration loop (≤ 5 rounds); G-003 blind listening with `iterate`/`rescope` options | A pure physical model may plateau below "totally realistic"; human may need to rescope (e.g. hybrid convolution body) | yes, via G-003 |
| R-002 | Simplified alto geometry inaccurate (C-017 inferred) | medium | medium | D-008 per-note tuning; T-026 resonance/inharmonicity bounds; T-011 register reliability; decision rules | Timbre detail depends on calibration | yes |
| R-003 | Fingering chart error (C-010 single-source) | low | medium | Frozen fixture from WFG; G-004 human check; `correct-chart` path via TEST_CHALLENGE | — | yes |
| R-004 | Realism thresholds arbitrary (A-012) | medium | medium | Thresholds frozen and justified; G-003 is final arbiter; failing metrics still go to G-003 | — | yes |
| R-005 | Numerical instability (explicit coupling) | medium | high | Oversampling ≥ 176.4 kHz (C-008); T-014 abuse test; state reset rule | — | yes |
| R-006 | Low notes crack at o = 0 (multistability, C-009) | medium | medium | τ_base attack shaping; inharmonicity ≥ measured; T-011; ordered decision rule | — | yes |
| R-007 | pluginval/JUCE 9 incompatibility (C-020) | low | low | Sandbox validation attempt (see ENVIRONMENT verification log); decision rule (strictness 8 + BLOCKED for confirmation) | — | yes |
| R-008 | TinySOL unavailable or layout differs | low | medium | MD5-verified download, CI cache, header-name mapping rule, fail-not-skip | — | yes |
| R-009 | Headless editor test crash on CI | medium | low | `xvfb-run -a`; never skip | — | yes |
| R-010 | CPU budget exceeded | low | medium | T-016; allowed optimisations listed in decision rules | — | yes |
| R-011 | AGPLv3 obligations at distribution | n/a now | medium | Distribution out of scope (A-013); THIRD_PARTY_NOTICES.md; README states licences | Human must choose AGPL compliance or a JUCE licence before distributing binaries | yes |
| R-012 | Sample-rate-dependent pitch | medium | medium | T-020; decision rule (higher internal rate / per-K calibration) | — | yes |
| R-013 | Frozen tests weakened | low | high | SHA-256 manifest; CI `freeze` job; HANDOFF halt rules | — | yes |
| R-014 | PAT exposure | medium | high | Token never written to the repo or memory; used only via an in-memory credential helper; human advised to revoke after planning | Token was shared in chat | yes (human action) |
| R-015 | Pitch drift with dynamics | high | low | Realistic behaviour; T-010 at mf only | — | yes |
| R-016 | Harmonic-mode fallback changes the sound | medium | low | Logged in DEVIATIONS.md; G-003 demo includes harmonic run | — | yes |
| R-017 | MSVC string-literal limit when embedding JSON | medium | low | S-007 chunking ≤ 16 kB | — | yes |
| R-018 | AU never validated | medium | low | `auval` step in CI on macOS (D-020) | — | yes |
