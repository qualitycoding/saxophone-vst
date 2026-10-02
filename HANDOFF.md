# HANDOFF — saxophone-vst v1 (implementing agent: start here)

**Purpose.** Build an alto-saxophone physical-model instrument plugin (VST3/AU/Standalone) whose UI draws a
saxophone and highlights the keys of the note being played, with an Overblow control and a Harmonic
(overblown-fingering) switch, and prove realism objectively and by human sign-off. You execute
`plan/PLAN.md`; you do not re-plan.

**Active profile:** `software` (deploys = false). See `plan/PROFILE.md` for N/A sections.

## Reading order
1. This file → 2. `plan/PROFILE.md` → 3. `plan/ASSUMPTIONS.md` → 4. `plan/DECISIONS.md` (design, interfaces,
decision rules) → 5. `plan/PLAN.md` (steps S-001…S-018) → 6. `plan/GATES.md` → 7. `plan/ENVIRONMENT.md` →
8. `plan/TRACEABILITY.md` → 9. `premortem/RISK_REGISTER.md`. Background only: `research/`.

## Environment
Follow `plan/ENVIRONMENT.md` exactly (system packages, Python venv from `tools/requirements.txt`, CMake ≥ 3.22).

## Running the frozen suite
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
ctest --test-dir build -LE perf --output-on-failure     # unit, integration, operational, alloc, plugin
ctest --test-dir build -L perf --output-on-failure      # T-016 (Release only)
xvfb-run -a build/plugin/sax_plugin_tests               # T-029 on headless Linux
python -m pytest tests/python -k "not tinysol"          # T-022a, T-024, T-030, T-031
SAX_RENDER=build/tools/render/sax_render TINYSOL_DIR=reference-data/tinysol \
  python -m pytest tests/python/test_realism_vs_tinysol.py   # T-022b (needs network once)
bash tests/scripts/run_pluginval.sh build/plugin/SaxophoneVST_artefacts/Release/VST3/Saxophone.vst3  # T-023
```

## Verifying the freeze
`bash tests/scripts/verify_freeze.sh` (= `sha256sum --check --strict tests/FROZEN_MANIFEST.sha256`). It must
print `freeze OK` before and after every step. Frozen files carry a `FROZEN — DO NOT MODIFY` header.

## Steps at a glance
S-001 env/baseline → S-002 CI + notices → S-003 pitch/keys/fingering/layout → S-004 analysis →
S-005 reed + reference model → S-006 TMM (Python) → S-007 resonator table → S-008 real-time voice →
S-009 tuning calibration → S-010 expression/MIDI → S-011 Overblow/Harmonic → S-012 state →
S-013 plugin processor → S-014 editor/UI → **G-004** → S-015 renderer + TinySOL tooling →
S-016 realism calibration → **G-003** → S-017 CI on 3 OSes (perf, pluginval) → S-018 final report.
Independent branches: S-004 ∥ S-003 ∥ S-006 ∥ S-012 after S-001.

## Human gates
- **G-004** after S-014: fingering chart and UI screenshots.
- **G-003** after S-016: blind listening + realism metrics.
At a gate: stop, write `GATE-<id>.md` (evidence summary, questions, allowed responses from `plan/GATES.md`),
push, and wait. G-001 and G-002 are N/A (no headline research result; no release or other irreversible action).

## Halt / deviation protocol
- Anything not covered by a decision rule: apply the **default rule** in `plan/DECISIONS.md` and log it in
  `DEVIATIONS.md` (step, situation, choice, rationale).
- Halt and write `BLOCKED.md` if a change would touch frozen tests/fixtures, security, data integrity, a public
  interface (headers/stub signatures), or research integrity.
- If a frozen test seems invalid: halt, write `TEST_CHALLENGE.md` (test ID, evidence, proposed fix). Never
  modify, skip, `[!mayfail]`, `xfail` or weaken a frozen test.
- Never create a release, tag, package, or publish anything. Never commit `reference-data/` or the PAT.

## Integrity rule (Rule 9, verbatim)
**Integrity** `[All]`: No step may fabricate, cherry-pick without disclosure, or manually alter data, test results, benchmarks, or figures. In addition:
* `[computational, publication]` Every reported number is generated from committed results, not transcribed by hand.
* `[publication]` Generative-AI images are never used as data figures. AI assistance is disclosed according to the venue's policy, as recorded in `plan/ASSUMPTIONS.md`.

(The bracketed sub-rules are inactive for this software-only plan, but realism metrics and gate bundles must
still be produced only by the committed tools.)
