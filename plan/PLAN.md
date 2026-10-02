<!-- STATUS HEADER (Phase 5) is inserted above this line -->
# Execution plan — saxophone-vst v1

Conventions: run all commands from the repository root on branch `impl/saxophone-v1` (D-019). "Build" means
`cmake --build build -j`. After each step: commit `S-0xx: <title>`, update `.checkpoints/impl-state.json`
(`{"step":"S-0xx","status":"done","commit":"<sha>","utc":"..."}`), push. Every step is idempotent: re-running
it after partial completion is safe; partial completion is detected by its "Done when" checks.

### S-001 Environment and baseline
- Tier: Sonnet
- Profile: software
- Depends on: none
- Inputs: generation branch head; plan/ENVIRONMENT.md
- Actions:
  1. `git checkout -b impl/saxophone-v1 origin/<generation branch>` (or `git checkout impl/saxophone-v1` if it exists).
  2. Install system packages and Python venv exactly as in plan/ENVIRONMENT.md.
  3. `bash tests/scripts/verify_freeze.sh`.
  4. `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release` and build.
  5. `ctest --test-dir build --output-on-failure || true` and `python -m pytest tests/python || true`; save outputs to `logs/S-001-red.txt`.
- Outputs: build/, .venv/, logs/S-001-red.txt
- Evidence produced: none (baseline)
- Done when: freeze check prints `freeze OK`; build succeeds (plugin included); every test except the D-018 guard tests (T-024a, T-024b, T-028) fails.
- Checkpoint: step, commit, red summary counts.
- On failure: missing packages → install and retry; build failure in JUCE/plugin stub → `BLOCKED.md` (the stubs were planned to compile).
- Gate: none
- Relevant decisions/claims: D-001, D-002, D-012, D-018, C-002, C-023

### S-002 CI workflow and third-party notices
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: D-017, D-020
- Actions:
  1. Create `.github/workflows/ci.yml` with the five jobs of D-020, `on: [push, pull_request]`, `permissions: contents: read`, actions pinned to the SHAs in D-020.
  2. Create `THIRD_PARTY_NOTICES.md` with the D-017 entries (name, licence, URL, what it is used for, shipped yes/no).
  3. Push; open the Actions run.
- Outputs: .github/workflows/ci.yml, THIRD_PARTY_NOTICES.md
- Evidence produced: T-024 (all 4 cases green)
- Done when: `python -m pytest tests/python/test_supply_chain.py` passes; CI `freeze` job green (others may be red).
- Checkpoint: step, commit, CI run URL.
- On failure: YAML error → fix syntax; never unpin actions.
- Gate: none
- Relevant decisions/claims: D-017, D-020, C-022

### S-003 Pitch, keys, fingering, key layout
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: core/include/sax/{Pitch,Keys,Fingering,KeyLayout}.h; tests/fixtures/alto_standard_fingerings.txt
- Actions:
  1. Implement `Pitch.cpp`, `Keys.cpp` (names exactly as in Keys.h comments), `Fingering.cpp` (a `constexpr`/static table of 33 entries transcribed from the fixture; do not read the fixture at runtime), `resolveNote` per D-007.
  2. Implement `KeyLayout.cpp`: normalised coordinates of an upright alto (neck top-right, bell bottom-right): octave (thumb, rear), palm D/Eb/F clustered upper-left of LH stack, front F above LH1, LH1, bis (between LH1 and LH2, small), LH2, LH3 vertically; G# and LH pinky table (C#, B, Bb) left of LH3; side E/C/Bb stacked right of RH1/RH2, high F# right of side E; RH1–RH3 vertically below LH3; alt F# beside RH2; RH pinky Eb/C below RH3. Keys must not share centres.
  3. Build; run `build/tests/sax_unit_tests "[T-001],[T-002],[T-003],[T-004],[T-005],[T-018]"`.
- Outputs: core/src/{Pitch,Keys,Fingering,KeyLayout}.cpp
- Evidence produced: T-001, T-002, T-003, T-004, T-005, T-018
- Done when: those tags pass.
- Checkpoint: step, commit, passing tags.
- On failure: fixture mismatch → fix the table (the fixture is authoritative); if you believe the fixture is wrong → `TEST_CHALLENGE.md`.
- Gate: none
- Relevant decisions/claims: D-004, D-007, C-004, C-010, C-012

### S-004 Analysis utilities (C++ and Python)
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: core/include/sax/Analysis.h; tools/realism/metrics.py
- Actions:
  1. Implement YIN (de Cheveigné & Kawahara 2002: difference function, cumulative-mean normalisation, absolute threshold 0.1, parabolic interpolation, search 50–2000 Hz, frame = whole input capped at the last 1.0 s), `classifyRegime` exactly per header, `harmonicLevelsDb` (Hann, zero-pad to next pow2 ≥ 4×len, peak within ±3 % of h·f0), `spectralCentroidHz` (power spectrum 20 Hz–fs/2), `cents`. Use a self-written radix-2 FFT (no new dependency).
  2. Implement the same metrics in `tools/realism/metrics.py` (numpy/scipy) incl. `attack_time_s`, `steady_segment`, `compare` per docstrings.
  3. Run `build/tests/sax_unit_tests "[T-027]"` and `python -m pytest tests/python/test_realism_metrics.py`.
- Outputs: core/src/Analysis.cpp, tools/realism/metrics.py
- Evidence produced: T-027, T-022a
- Done when: both pass.
- Checkpoint: step, commit.
- On failure: f0 error > 2 cents → check interpolation and that the last-1.0 s cap is applied.
- Gate: none
- Relevant decisions/claims: D-015

### S-005 Reed model and Colinot reference integrator
- Tier: Opus
- Profile: software
- Depends on: S-004
- Inputs: ReedModel.h, ColinotReference.h, research/spikes/colinot_spike.py, C-006 (Table 1/2 values)
- Actions:
  1. Implement eqs. (5)–(10) in `ReedModel.cpp`.
  2. Implement `simulateColinot` as a line-by-line port of `run()` in `research/spikes/colinot_spike.py` (D-005), in double precision; `colinotDSharpTable2()` returns Table 2 (residues real).
  3. Run `build/tests/sax_unit_tests "[T-007]"` and `build/tests/sax_integration_tests "[T-008],[T-009]"`.
- Outputs: core/src/ReedModel.cpp, core/src/ColinotReference.cpp
- Evidence produced: T-007, T-008, T-009
- Done when: those tags pass.
- Checkpoint: step, commit, measured f0 values.
- On failure: decision rule "T-008 f0 outside window".
- Gate: none
- Relevant decisions/claims: D-005, C-006, C-007, C-008, C-009

### S-006 Transfer-matrix model and modal fit (Python)
- Tier: Opus
- Profile: software
- Depends on: S-001
- Inputs: tools/resonator/tmm.py; D-009; C-016
- Actions:
  1. Implement plane-wave TMM: cylinder and cone (spherical-wave) matrices with Zwikker–Kosten visco-thermal losses (`lossless=True` disables), side hole (series/shunt model of Lefebvre & Scavone 2012 / Dalmont 2002, open with radiation, closed as a short closed tube), unflanged radiation (Silva et al. 2009 approximation; `radiation=False` → Z_R = 0).
  2. `modal_fit`: initialise poles from |Z| peaks (frequency, half-power bandwidth), residues from peak heights; refine all with `scipy.optimize.least_squares` on complex Z (relative error weighting); enforce Re(s) < 0, sort by Im.
  3. `alto_geometry` per D-009 (bisection on cone length).
  4. `python -m pytest tests/python/test_resonator_tmm.py -k "not t031"`.
- Outputs: tools/resonator/tmm.py
- Evidence produced: T-030
- Done when: the 6 non-T-031 tests pass.
- Checkpoint: step, commit.
- On failure: cone test off by > 5 % → check that the cylinder substitute length uses R1/(3 tan φ); modal-fit test → widen the frequency span used for initialisation, never the tolerances.
- Gate: none
- Relevant decisions/claims: D-009, C-016, C-017, C-018

### S-007 Resonator table: generation, parsing, embedding
- Tier: Sonnet
- Profile: software
- Depends on: S-003, S-006
- Inputs: D-009; ResonatorTable.h
- Actions:
  1. Implement `generate_table.py` (deterministic: fixed frequency grid, no randomness, `json.dumps(..., indent=1, sort_keys=True)`, floats rounded to 6 significant digits; provenance.git = `git rev-parse HEAD` or "unknown").
  2. `python -m tools.resonator.generate_table --out data/alto_resonators.json`.
  3. Implement `ResonatorTable::fromJson/lookup/size` with nlohmann/json and the D-009 validation (wrap every exception as `ParseError`).
  4. In `core/CMakeLists.txt` add a custom command that reads `data/alto_resonators.json` and writes `${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp` defining `embeddedResonatorJson()` (raw string literal split into ≤ 16 kB chunks for MSVC); remove the stub definition from ResonatorTable.cpp.
  5. Run `pytest tests/python/test_resonator_tmm.py -k t031`, `sax_unit_tests "[T-006]"`, `sax_unit_tests "[T-026]"`.
- Outputs: tools/resonator/generate_table.py, data/alto_resonators.json, core/src/ResonatorTable.cpp, core/CMakeLists.txt
- Evidence produced: T-006, T-031 (T-026 structural part; cents part may still fail until S-009)
- Done when: T-006 and T-031 pass; T-026 failures, if any, are only the cents window.
- Checkpoint: step, commit, table SHA-256.
- On failure: decision rule "T-026 cents window".
- Gate: none
- Relevant decisions/claims: D-009, C-013, C-016

### S-008 Real-time voice core
- Tier: Opus
- Profile: software
- Depends on: S-005, S-007
- Inputs: SaxVoice.h; D-005, D-010, D-011, D-014
- Actions:
  1. Create `core/src/VoiceTuning.h` holding all non-frozen constants of D-006/D-011/D-014.
  2. Implement `SaxVoice::Impl`: oversampled exciter+modal loop (D-005), half-band decimators, parameter smoothing, note state machine (D-011), output stage (D-014), atomics for UI (D-010), `clamped()`.
  3. Run `sax_operational_tests`, `sax_alloc_tests`, `sax_integration_tests "[T-020]"`.
- Outputs: core/src/SaxVoice.cpp, core/src/VoiceTuning.h (+ private helpers)
- Evidence produced: T-014, T-015, T-020, T-021, T-025
- Done when: those pass.
- Checkpoint: step, commit.
- On failure: NaN in T-014 → add state reset when |p| > 50 or non-finite (log); allocation → decision rule.
- Gate: none
- Relevant decisions/claims: D-005, D-010, D-011, D-014, C-008

### S-009 Tuning calibration
- Tier: Sonnet
- Profile: software
- Depends on: S-008
- Inputs: D-008
- Actions:
  1. Create `tools/render/CMakeLists.txt` and add `add_subdirectory(tools/render)` to the root `CMakeLists.txt` right after `add_subdirectory(core)`. Add `tools/render/calibrate_tuning.cpp` → target `sax_calibrate` (links sax_core) that loads `data/alto_resonators.json`, renders each entry's note per D-008 and rewrites `tuning_scale` (stable formatting as in S-007).
  2. Run it, rebuild (re-embeds the table).
  3. Run `sax_integration_tests "[T-010]"` and `sax_unit_tests "[T-026]"`.
- Outputs: tools/render/CMakeLists.txt, tools/render/calibrate_tuning.cpp, CMakeLists.txt, data/alto_resonators.json (updated)
- Evidence produced: T-010, T-026
- Done when: both pass.
- Checkpoint: step, commit, max |cents| per note.
- On failure: decision rule "T-010 fails".
- Gate: none
- Relevant decisions/claims: D-008, C-013

### S-010 Expression and MIDI behaviour
- Tier: Sonnet
- Profile: software
- Depends on: S-009
- Inputs: D-011
- Actions: implement breath/velocity, legato stack (fixed-size array of 16 held notes), portamento interpolation, release/idle detection, vibrato, pitch bend; run `sax_integration_tests "[T-019]"` and re-run all previously green core tests.
- Outputs: core/src/SaxVoice.cpp
- Evidence produced: T-019
- Done when: T-019 passes and no regression.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-011

### S-011 Overblow and Harmonic mode
- Tier: Opus
- Profile: software
- Depends on: S-010
- Inputs: D-006, D-007, C-007, C-009, C-015
- Actions: implement the D-006 mapping and D-007 harmonic handling; run `sax_integration_tests "[T-011],[T-012],[T-013]"` and all core tests.
- Outputs: core/src/SaxVoice.cpp, core/src/VoiceTuning.h
- Evidence produced: T-011, T-012, T-013
- Done when: those pass with no regression.
- Checkpoint: step, commit, final constants.
- On failure: decision rules for T-011/T-012/T-013.
- Gate: none
- Relevant decisions/claims: D-006, D-007

### S-012 State persistence
- Tier: Sonnet
- Profile: software
- Depends on: S-003
- Inputs: State.h, D-013
- Actions: implement with nlohmann/json (`parse(text, nullptr, false)` — no exceptions; size check first; float fields via `is_number()`; bool via `is_boolean()`; then `clamped()`); implement `clamped()` if not done; run `sax_unit_tests "[T-017]"`.
- Outputs: core/src/State.cpp
- Evidence produced: T-017
- Done when: passes.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-013

### S-013 Plugin processor
- Tier: Sonnet
- Profile: software
- Depends on: S-011, S-012
- Inputs: plugin/src/PluginProcessor.*, D-010, D-011, D-013
- Actions: own a `sax::SaxVoice` (table from `embeddedResonatorJson()` loaded once in the constructor); `prepareToPlay` → `voice.prepare`; `processBlock`: `juce::ScopedNoDenormals`, read APVTS atomics into `VoiceParameters`, split the block at MIDI event positions and dispatch MIDI per D-011, render mono, copy to all output channels; `setLatencySamples` with the decimator latency; state via D-013; `currentKeys()` from the voice. Run `xvfb-run -a build/plugin/sax_plugin_tests "[T-029]"` (Linux) — editor case may still fail until S-014.
- Outputs: plugin/src/PluginProcessor.cpp
- Evidence produced: T-029 (parameter, sound and state cases)
- Done when: those two cases pass and the sound/highlight case passes its processor assertions.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-010, D-011, D-013

### S-014 Editor and saxophone drawing
- Tier: Sonnet
- Profile: software
- Depends on: S-013
- Inputs: D-016, KeyLayout
- Actions:
  1. Implement `SaxophoneView` (D-016), `setHighlightedKeys` (store + repaint on change), editor layout/controls/attachments, 60 Hz timer, `refreshFromProcessor`.
  2. Add `tools/render/snapshot_ui.cpp` → target `sax_ui_snapshots` defined in `plugin/CMakeLists.txt` (links SaxophoneVST; plugin targets only exist when SAX_BUILD_PLUGIN=ON) that writes the G-004 screenshots into `gates/G-004/`; on Linux run it as `xvfb-run -a build/plugin/sax_ui_snapshots`.
  3. Run all T-029 cases; build the bundle for G-004.
- Outputs: plugin/src/{SaxophoneView,PluginEditor}.cpp, plugin/CMakeLists.txt, tools/render/snapshot_ui.cpp, gates/G-004/*
- Evidence produced: T-029 (editor case)
- Done when: all T-029 cases pass; G-004 bundle complete.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: **G-004**
- Relevant decisions/claims: D-016, D-004, C-010

### S-015 Offline renderer and TinySOL tooling
- Tier: Sonnet
- Profile: software
- Depends on: S-011
- Inputs: tools/render/main.cpp usage line, D-015, C-005
- Actions:
  1. Implement `sax_render` (argument parsing, mono float32 WAV writer, exit codes as documented); add target `sax_render` (links sax_core only) to `tools/render/CMakeLists.txt` (created in S-009).
  2. Implement `tinysol.download` (Zenodo REST `https://zenodo.org/api/records/3685367` → file links + md5 checksums; stream download; verify; extract to `<dest>`; idempotent) and `alto_notes`, and `compare_tinysol.main` per D-015.
  3. `python -c "from tools.realism import tinysol; tinysol.download(__import__('pathlib').Path('reference-data/tinysol'))"`; record the md5 values and row counts in `data/REFERENCE_DATA.md` (attribution text for CC BY 4.0 included). `reference-data/` stays git-ignored.
- Outputs: tools/render/main.cpp, tools/render/CMakeLists.txt, tools/realism/{tinysol,compare_tinysol}.py, data/REFERENCE_DATA.md
- Evidence produced: none directly (enables T-022b)
- Done when: `SAX_RENDER=build/tools/render/sax_render TINYSOL_DIR=reference-data/tinysol python -m pytest tests/python/test_realism_vs_tinysol.py` runs to the threshold assertions (pass or fail on metrics, not on setup).
- Checkpoint: step, commit, number of alto notes found.
- On failure: decision rules "TinySOL metadata", "Zenodo unreachable".
- Gate: none
- Relevant decisions/claims: D-015, C-005

### S-016 Realism calibration
- Tier: Opus
- Profile: software
- Depends on: S-014, S-015
- Inputs: results.json from T-022b; VoiceTuning.h constants
- Actions:
  1. Run T-022b; write `logs/S-016-round-<k>.md` with per-dynamic metric means and the worst 10 notes.
  2. Adjust only non-frozen constants (D-006, D-011, D-014, ζ mapping); after each round re-run **all** frozen tests (no regression allowed). Max 5 rounds (decision rule).
  3. Build the G-003 bundle (12 blind pairs chosen with `random.Random(20261001).sample` over notes that exist in TinySOL at mf, demos per GATES.md).
- Outputs: core/src/VoiceTuning.h, logs/S-016-*.md, gates/G-003/*
- Evidence produced: T-022b
- Done when: T-022b passes or 5 rounds exhausted; G-003 bundle complete.
- Checkpoint: step, round number, metrics.
- On failure: decision rule "T-022b fails".
- Gate: **G-003**
- Relevant decisions/claims: D-014, D-015, A-012, C-021

### S-017 Performance and plugin validation on all platforms
- Tier: Sonnet
- Profile: software
- Depends on: S-016
- Inputs: CI jobs `core`, `plugin`
- Actions: push; ensure CI `core` (incl. `ctest -L perf`) and `plugin` (incl. pluginval strictness 10) are green on all three OSes; fix issues.
- Outputs: fixes as needed
- Evidence produced: T-016, T-023, T-029 on 3 OSes
- Done when: CI jobs `freeze`, `core`, `python`, `plugin`, `realism` all green on the same commit.
- Checkpoint: step, commit, CI run URL.
- On failure: decision rules "T-016", "pluginval".
- Gate: none
- Relevant decisions/claims: D-020, C-020

### S-018 Final verification and report
- Tier: Sonnet
- Profile: software
- Depends on: S-017
- Actions: `bash tests/scripts/verify_freeze.sh`; full local run of all suites; confirm zero core compiler warnings in CI logs; update README "Status" and "Building" sections (no claims beyond evidence); write `REPORT.md` (tests by ID with status, deviations, gate outcomes, open risks). Push.
- Outputs: README.md, REPORT.md
- Evidence produced: all T-IDs green on one commit
- Done when: everything above is green and REPORT.md is pushed.
- Checkpoint: final.
- On failure: default rule.
- Gate: none (merging to `main` is the human's decision, D-019)
- Relevant decisions/claims: all
