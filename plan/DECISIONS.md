# Decisions, interfaces and decision rules

## Design decisions

**D-001 Framework.** JUCE 9.0.3 (commit `be29c814…`), CMake ≥ 3.22, C++20. Rationale: current release (C-002); intake said "JUCE 8" only because it predated the check. JUCE is consumed by FetchContent, never vendored.

**D-002 Pinning.** Every FetchContent dependency is pinned to a full 40-hex commit SHA (JUCE, Catch2 v3.16.0, nlohmann/json v3.12.0). Python tools: exact `==` pins in `tools/requirements.txt` + `tools/requirements.lock`. GitHub Actions: full SHAs (D-020). Enforced by T-024.

**D-003 Architecture.**
- `core/` → static library `sax_core`: pure C++20, no JUCE, all DSP, fingering, layout geometry, analysis and state. Everything testable headless.
- `plugin/` → `juce_add_plugin(SaxophoneVST)`: thin wrapper (parameters, MIDI → `SaxVoice`, editor drawing).
- `tools/render/` → `sax_render` CLI (core only) used by realism tooling.
- `tools/resonator/` (Python) generates `data/alto_resonators.json`; `tools/realism/` (Python) compares renders with TinySOL.
- `data/alto_resonators.json` is embedded into `sax_core` at build time (D-009).

**D-004 Fingering chart.** Standard fingerings from the Woodwind Fingering Guide basic chart (C-010) for written 58–90, frozen in `tests/fixtures/alto_standard_fingerings.txt`. Choices where the chart offers several "standard" options: written Bb4/Bb5 = LH1 + bis; F#6 = F6 fingering + high-F# key. Octave vent: body for written 74–80, neck for 81–90, none below 74 (C-012). Palm-key notes have `targetRegister = 2`.

**D-005 Synthesis model and scheme.** Exciter and resonator exactly as Colinot et al. 2021 eqs. (4)–(13) (C-006). Integration (verified by spike S1):
- modal states: `p_n ← e^{s_n Δt} p_n + C_n (e^{s_n Δt} − 1)/s_n · u` (flow held over the step);
- reed: central differences, `x_{k+1} = (Δt²ω_r²(p − γ + F_c − x_k) + 2x_k − x_{k−1} + ½q_rω_rΔt x_{k−1}) / (1 + ½q_rω_rΔt)`;
- `p = 2 Σ Re(p_n)` evaluated before the reed update (one-sample explicit coupling).
Internal rate `F_int = K·F_host`, `K = clamp(ceil(176400 / F_host), 1, 8)` (C-008). Decimation: cascaded half-band FIR (≥ 90 dB stop-band, linear phase, latency reported to host ≤ 64 host samples) when K is a power of two; otherwise K is rounded up to the next power of two.
The offline `simulateColinot()` uses the same equations at a fixed rate (default 176 400 Hz) without decimation.

**D-006 Overblow knob (o ∈ [0,1]).** Physically grounded (C-007, C-009): as o rises, the virtual player blows harder, with a tighter attack and a voicing that makes the resonator's 2nd mode more harmonic. Starting mapping (constants live in `core/src/VoiceTuning.h`, are **not frozen**, and may be retuned to satisfy T-011/T-013 and G-003):
- `γ_target = γ_base · (1 + 0.6·o)`; `ζ = ζ_base · (1 + 0.25·o)`;
- `Im(s_2) ← Im(s_2) − 0.8·o·(Im(s_2) − 2·Im(s_1))` (pull f2/f1 toward 2);
- attack rise time `τ_g = τ_base · (1 − 0.9·o)`, `τ_base` = 15 ms;
- breath-noise gain `× (1 + o)`.

**D-007 Harmonic switch.** `resolveNote(n, true)` for concert 61–76 uses the fingering of written(n)−12 without octave key (UI shows that low fingering). The voice then: forces the D-006 mapping with `o_eff = max(o, 0.6)`, sets `Im(s_2) = 2·Im(s_1)`, scales `|C_1|` by 0.3 (lip/voicing suppression of mode 1), and retunes the whole resonator so the 2nd mode sits at the target note. Fallback rule in "Decision rules".

**D-008 Tuning.** Each table entry carries `tuning_scale` (default 1.0) multiplying every Im(s_n). S-009 calibrates it by rendering each note at velocity 0.6, overblow 0, 48 kHz and iterating `scale ← scale · 2^(−cents/1200)` until |cents| ≤ 2 (max 8 iterations), then rewrites `data/alto_resonators.json`. Pitch bend and tuning A4 multiply all Im(s_n) by `2^(bend·2/12) · (A4/440)`.

**D-009 Resonator table.** Schema `saxophone-vst/resonators@1`:
```json
{"schema":"saxophone-vst/resonators@1","instrument":"alto",
 "units":{"pole":"rad/s","residue":"Z/Zc"},
 "provenance":{"generator":"tools/resonator/generate_table.py","git":"<sha>","geometry":"D-009"},
 "entries":[{"written":58,"hole":"none","tuning_scale":1.0,
             "modes":[{"re_s":-17.6,"im_s":1195.0,"re_c":176.1,"im_c":0.0}, ...]}]}
```
Validation (T-006): JSON object, schema tag, `hole ∈ {none, body, neck}`, 4 ≤ modes ≤ 16, `re_s < 0`, `im_s` strictly increasing, all numbers finite, unique (written, hole), file ≤ 1 MiB. `tuning_scale` optional (default 1.0, must be in [0.8, 1.25]).
Geometry ("simplified alto", C-016 scaled by 1.5, C-017): cone half-angle 1.74°, input radius R1 = 6.9 mm, cylindrical mouthpiece L_cyl = R1/(3 tan φ), body register hole at L1 = 195 mm (Rh 2.1 mm, Lh 6 mm), neck register hole at L1 = 60 mm (Rh 1.275 mm, Lh 6 mm). Per entry the cone length L is solved (bisection, 1e-6 m) so that the closed-hole first resonance equals `ET(written−9) · 2^(+25/1200)` (C-013: playing frequency sits below f1). Second-register entries (74–85): geometry of written−12 with the D-004 hole open. Palm-key notes (86–90): cone length solved for `ET(written−9−12)·2^(25/1200)` (virtual first-register length) with neck hole open. Losses: visco-thermal (Zwikker–Kosten); radiation: unflanged (Silva et al. 2009). Modal fit: N_m = 8 per entry over 20 Hz … 1.15 × the 9th peak. Embedding: CMake `file(READ)` → generated `embedded_resonators.cpp` exposing `embeddedResonatorJson()`.

**D-010 Real-time safety.** After `prepare()`: no allocation, no locks, no exceptions, no I/O in `process()`, note/parameter setters, or `currentKeys()`. Parameters are smoothed (one-pole, 10 ms). UI reads an `std::atomic<uint32_t>` key bitmask and `std::atomic<int>` note, written once per block. Denormals flushed (`juce::ScopedNoDenormals` in the plugin; `fesetenv`-free core: add 1e-20 anti-denormal offset to modal states).

**D-011 Expression.** Breath absent until a breath value ≥ 0 arrives (`setBreath(-1)` resets to absent). Absent: `γ_base = 0.40 + 0.22·velocity`; present: `γ_base = breath < 0.02 ? 0 : 0.38 + 0.24·breath`. `ζ_base = 0.8 − 0.4·reedHardness`. Velocity also scales output by `0.5 + 0.5·velocity` only when breath is absent. Vibrato: γ and pole scaling modulated at `vibratoRateHz`, depth `vibratoDepth · (0.3 + 0.7·aftertouch)`, max ±15 cents. Note change (legato): resonator parameters interpolated linearly over `portamentoMs` (min 3 ms, to avoid clicks). Note off: γ ramps to 0 over 40 ms; when output RMS < 1e-5 for 50 ms, the voice is idle (note −1, keys empty). MIDI: omni; CC2 and CC11 → setBreath(value/127); channel pressure → aftertouch; pitch bend → ±2 semitones; CC123/CC120 → allNotesOff.

**D-012 Stub conventions.** Non-noexcept stubs throw `sax::NotImplemented` (C++) / `NotImplementedError` (Python). Noexcept stubs return the sentinel documented in the header. Tests must fail by assertion or exception, never by crash.

**D-013 Parameters and state.** Parameter IDs (permanent): `overblow, harmonic, reed_hardness, brightness, breath_noise, vibrato_rate, vibrato_depth, portamento, tuning_a4, output_gain` with ranges as in `VoiceParameters`. State blob = UTF-8 JSON `{"schema":"saxophone-vst/state@1", "<VoiceParameters field name>": value, ...}` written by `serializeState()`; the plugin's `getStateInformation` writes exactly that string; `setStateInformation` parses with `deserializeState()` and on `nullopt` keeps current values.

**D-014 Output stage.** mouthpiece pressure → radiation filter (first-order high-pass at 120 Hz × second-order high-shelf at f_c = 1.2 kHz × 1.5^−1 ≈ 800 Hz (alto tone-hole-lattice cutoff scaled from soprano, C-021), shelf gain `−6 dB + 9 dB·brightness`) → + breath noise (xorshift32 white noise, seed 0x5A5A5A5A at `prepare()`, band-passed 1–6 kHz, gain `breathNoise · 0.05 · |u|`) → DC blocker (5 Hz) → `outputGainDb` → `tanh` soft clip → |y| ≤ 1. Plugin output: mono copied to both channels. All filter constants are tuning constants (not frozen) adjusted during S-016.

**D-015 Realism procedure (frozen procedure, A-012 thresholds).** For every TinySOL row with `Instrument (in full) == "Alto Saxophone"`: concert MIDI = `Pitch ID`; render with `sax_render --note <midi> --velocity {pp:.25, mf:.6, ff:.95} --seconds <ref duration capped at 4 s> --fs 44100 --release-at <ref duration − 0.3 s>`; resample reference to 44.1 kHz if needed; compute `metrics.compare(ref, syn, fs)` on steady segments, using the **measured** reference f0 for harmonic picking; write one JSON row per note `{midi, dynamics, harm_mad_db, centroid_ratio, attack_ratio, centroid_syn_hz, f0_ref, f0_syn}` sorted by (midi, dynamics).

**D-016 UI.** Editor 900×600 (resizable, fixed aspect, 0.75–2×). Left 60 %: `SaxophoneView` — body/bell/neck drawn with `juce::Path`, 23 keys from `altoKeyLayout()`; pressed keys filled with accent `#F2B134` at full opacity, others outlined `#8A8F98`; written and concert note names shown under the horn. Right 40 %: rotary knobs for the 9 continuous parameters and a toggle for Harmonic, attached via `AudioProcessorValueTreeState` attachments. Editor timer 60 Hz → `refreshFromProcessor()`.

**D-017 Third-party notices.** `THIRD_PARTY_NOTICES.md` lists: JUCE 9 (AGPLv3 / JUCE licence), VST 3 SDK (MIT), nlohmann/json (MIT), Catch2 (BSL-1.0, tests only), pluginval (GPLv3, CI only, not distributed), TinySOL (CC BY 4.0, Cella et al., test data only, not distributed), Colinot et al. 2021 and Szwarcberg et al. 2025 (CC BY 4.0, model equations/parameters).

**D-018 Guard tests.** T-024a (`test_cmake_dependencies_pinned_to_full_sha`), T-024b (`test_python_requirements_pinned`) and T-028 (`verify_freeze.sh`) protect planning deliverables and therefore already pass at freeze time; documented exception to "red verification". All other tests fail against the stubs.

**D-019 Branching.** Implementation branch `impl/saxophone-v1` from the generation branch head. Commit per step: `S-0xx: <title>`. Merging into `main` is the human's choice after G-003 (not an irreversible external action).

**D-020 CI** (`.github/workflows/ci.yml`, created in S-002). Actions pinned: `actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1` (v7.0.1), `actions/setup-python@5fda3b95a4ea91299a34e894583c3862153e4b97` (v7.0.0), `actions/cache@55cc8345863c7cc4c66a329aec7e433d2d1c52a9` (v6.1.0), `actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a` (v7.0.1). Jobs:
1. `freeze` (ubuntu): `bash tests/scripts/verify_freeze.sh`.
2. `core` (matrix ubuntu-24.04, macos-15, windows-2025; Release, `-DSAX_BUILD_PLUGIN=OFF`): build, `ctest -LE perf`, then `ctest -L perf`.
3. `python` (ubuntu): install `tools/requirements.txt`, `pytest tests/python -k "not tinysol"`, `pip-audit -r tools/requirements.txt`.
4. `plugin` (matrix as 2; Release, plugin ON): build; Linux under `xvfb-run`: `ctest -R plugin`; `tests/scripts/run_pluginval.sh <VST3 path>`; macOS additionally copies the AU to `~/Library/Audio/Plug-Ins/Components` and runs `auval -v aumu Saxa Qcod` (R-018); upload VST3/Standalone as workflow artifacts (retention 14 days).
5. `realism` (ubuntu, needs core): cache `reference-data/tinysol` keyed on `tools/realism/tinysol.py` hash; `SAX_RENDER=build/tools/render/sax_render TINYSOL_DIR=reference-data/tinysol pytest tests/python/test_realism_vs_tinysol.py`; upload `results.json`.

## Public interfaces
Authoritative signatures are the committed headers `core/include/sax/*.h`, `plugin/src/{PluginProcessor,PluginEditor,SaxophoneView,Parameters}.h`, `tools/**/*.py` stubs and the `sax_render` usage line in `tools/render/main.cpp`. Changing a public signature requires `BLOCKED.md` (default rule). Adding private helpers/files is free.

## Decision rules (if → then)
- **Dependency fetch/install fails** → retry 3× with 30 s back-off; if a pinned SHA is gone upstream, halt with `BLOCKED.md` (never float a version).
- **Compiler warning in core** → fix it (warnings are not errors, but S-018 requires a warning-free core build on all CI OSes).
- **T-008 f0 outside [180, 195] Hz** → check the scheme against `research/spikes/colinot_spike.py` sample-by-sample for the first 1000 samples (max abs diff < 1e-6 relative); fix the port; never change the window.
- **T-026 cents window fails for some notes** → re-solve cone length with tighter tolerance; if still failing, adjust the +25-cent offset in D-009 globally (allowed range +10…+40 cents) and log in `DEVIATIONS.md`.
- **T-010 fails after D-008 calibration** → increase iterations to 16; if still > 10 cents, the note is in an unstable regime: raise `γ_base` floor for that note group by ≤ 0.05 (logged); if still failing → `BLOCKED.md`.
- **T-011 fails (low notes crack/overblow at o = 0)** → in order: (1) lengthen `τ_base` toward 25 ms, (2) shift f2/f1 of affected entries up by +0.01 steps to ≤ 2.10 (C-007: more inharmonicity favours register 1), (3) reduce `γ_base` slope 0.22 → 0.18. Log each.
- **T-012 fails (harmonic mode does not reach register 2 or > 70 cents)** → fallback: for harmonic notes use the resonator of the standard octave fingering (written n) while still displaying the low fingering; log as deviation (sound shortcut, UI unchanged).
- **T-013 fails** → raise brightness contribution of overblow (shelf gain + 3 dB·o) and γ gain 0.6 → 0.8; log.
- **T-020 fails (pitch differs between sample rates by ≥ 3 cents)** → the explicit scheme is rate-dependent: raise the internal-rate target to `K = clamp(ceil(352800 / F_host), 1, 8)` (power of two) and re-run D-008 calibration at 48 kHz; if T-016 then fails, apply the per-note correction `tuning_scale · 2^(Δcents(F_int)/1200)` from a calibration at each K (stored as `tuning_scale_k<K>` keys, optional in schema) and log.
- **T-016 RTF > 0.05** → profile; allowed optimisations: float32 modal states, K = 2 when F_host ≥ 88.2 kHz, SIMD over modes. Never reduce N_m below 8 or K below ceil(176400/F_host)/2.
- **T-015 allocation detected** → move the allocation into `prepare()`; never disable the test.
- **T-022b fails** → iterate S-016 calibration (filter constants D-014, noise, D-006 constants, ζ map); at most 5 calibration rounds; if still failing, proceed to G-003 with the failing metrics in the bundle (human decides `proceed-with-rescope` or `iterate`). Never edit thresholds.
- **TinySOL metadata header lacks a required column** → map by header names `Path`, `Instrument (in full)`, `Pitch ID`, `Dynamics`, `Needed digital retuning`; if any missing, halt `BLOCKED.md`.
- **Zenodo unreachable** → retry 3×; CI realism job fails (does not skip); local work continues on other steps.
- **pluginval fails** → read its log; fix the plugin; if the failure is a known pluginval/JUCE-9 incompatibility (issue on Tracktion/pluginval), run at strictness 8 and record in `DEVIATIONS.md` + `BLOCKED.md` for human confirmation.
- **Headless editor test crashes on Linux CI** → run under `xvfb-run -a`; never skip.
- **pip-audit reports a vulnerability** → bump only that package to the nearest fixed version, update lockfile, re-run all Python tests; log.
- **Frozen test or fixture appears wrong** (e.g. human corrects the fingering chart at G-004) → halt, write `TEST_CHALLENGE.md`; protocol re-run (Rule 2E.4).
- **Default rule:** choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md` with rationale, and continue — **unless** it touches frozen tests, security, data integrity, a public interface, or research integrity, in which case halt and write `BLOCKED.md`.
