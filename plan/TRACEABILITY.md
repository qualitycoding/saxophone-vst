# Traceability

## Success criteria (measurable)
| ID | Criterion |
|---|---|
| SC-1 | VST3 + Standalone (+AU on macOS) build on Windows, macOS, Linux; pluginval strictness 10 passes; parameters/state work in the plugin. |
| SC-2 | The UI shows an alto saxophone; for every in-range note the 23-key highlight equals the standard fingering (harmonic mode: the low fingering) within one audio block; out-of-range notes are ignored. |
| SC-3 | Every note sounds within ±10 cents of equal temperament (A4 parameter respected); pitch independent of sample rate (< 3 cents). |
| SC-4 | Objective realism vs TinySOL alto recordings meets A-012 thresholds. |
| SC-5 | Overblow: at 0, low notes speak reliably in the first register; increasing it brightens the tone; Harmonic mode sounds the second register of the low fingering (±70 cents). |
| SC-6 | Expression: velocity/breath (CC2/CC11), pitch bend, aftertouch vibrato, mono legato with last-note priority, release to silence. |
| SC-7 | Real-time: RTF ≤ 0.05 at 48 kHz/128; zero allocations on the audio path. |
| SC-8 | All 10 parameters persist through save/load. |
| SC-9 | Robustness: no NaN/Inf, |y| ≤ 1 under abuse; malformed state/data rejected safely; extreme host configs work; deterministic output. |
| SC-10 | Repository is Apache-2.0 with README/NOTICE; dependencies pinned; third-party notices complete. |
| SC-11 | The human signs off realism (G-003) and the fingering chart/UI (G-004). |

## Requirement → evidence → step
| SC | Tests / gates | Steps |
|---|---|---|
| SC-1 | T-023, T-029 | S-013, S-014, S-017 |
| SC-2 | T-001, T-002, T-003, T-004, T-005, T-018, T-019, T-029; G-004 | S-003, S-010, S-014 |
| SC-3 | T-010, T-020, T-026 | S-007, S-008, S-009 |
| SC-4 | T-022a, T-022b; G-003 | S-004, S-015, S-016 |
| SC-5 | T-008, T-009, T-011, T-012, T-013 | S-005, S-011 |
| SC-6 | T-019 | S-010 |
| SC-7 | T-015, T-016 | S-008, S-017 |
| SC-8 | T-017, T-029 | S-012, S-013 |
| SC-9 | T-006, T-014, T-017, T-021, T-025, T-027 | S-004, S-007, S-008, S-012 |
| SC-10 | T-024, T-028 | S-002, S-018 |
| SC-11 | G-003, G-004 | S-014, S-016 |

Supporting (code verification of tools): T-007, T-027, T-030, T-031 → SC-3/4/5 measurements.

## Test → requirement (every test maps to a requirement)
| Test | File | Category | SC |
|---|---|---|---|
| T-001 | tests/core/test_pitch.cpp | unit | SC-2, SC-3 |
| T-002 | tests/core/test_keys.cpp | unit | SC-2 |
| T-003 | tests/core/test_fingering.cpp | unit | SC-2 |
| T-004 | tests/core/test_fingering.cpp | unit | SC-2 |
| T-005 | tests/core/test_fingering.cpp | unit | SC-2, SC-5 |
| T-006 | tests/core/test_resonator_table.cpp | unit / security | SC-9 |
| T-007 | tests/core/test_reed_model.cpp | unit | SC-5 |
| T-008 | tests/core/test_colinot_reference.cpp | integration (code verification) | SC-5 |
| T-009 | tests/core/test_colinot_reference.cpp | integration | SC-5 |
| T-010 | tests/core/test_voice_pitch.cpp | integration | SC-3 |
| T-011 | tests/core/test_voice_registers.cpp | integration | SC-5 |
| T-012 | tests/core/test_voice_registers.cpp | integration | SC-5 |
| T-013 | tests/core/test_voice_registers.cpp | integration | SC-5 |
| T-014 | tests/core/test_voice_robustness.cpp | operational | SC-9 |
| T-015 | tests/core/test_alloc.cpp | performance | SC-7 |
| T-016 | tests/core/test_perf.cpp | performance | SC-7 |
| T-017 | tests/core/test_state.cpp | unit / security | SC-8, SC-9 |
| T-018 | tests/core/test_key_layout.cpp | unit | SC-2 |
| T-019 | tests/core/test_midi_behaviour.cpp | integration | SC-2, SC-6 |
| T-020 | tests/core/test_voice_pitch.cpp | integration | SC-3 |
| T-021 | tests/core/test_voice_robustness.cpp | operational | SC-9 |
| T-022a | tests/python/test_realism_metrics.py | unit | SC-4 |
| T-022b | tests/python/test_realism_vs_tinysol.py | integration (realism) | SC-4 |
| T-023 | tests/scripts/run_pluginval.sh | operational | SC-1 |
| T-024 | tests/python/test_supply_chain.py | security | SC-10 |
| T-025 | tests/core/test_voice_robustness.cpp | operational | SC-9 |
| T-026 | tests/core/test_resonator_table.cpp | integration | SC-3 |
| T-027 | tests/core/test_analysis.cpp | unit | SC-3, SC-4 |
| T-028 | tests/scripts/verify_freeze.sh | operational | SC-10 |
| T-029 | tests/plugin/test_plugin.cpp | integration | SC-1, SC-2, SC-8 |
| T-030 | tests/python/test_resonator_tmm.py | unit | SC-3 |
| T-031 | tests/python/test_resonator_tmm.py | integration | SC-3 |
