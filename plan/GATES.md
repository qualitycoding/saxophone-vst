# Human gates

Rule-10 required gates:
- **G-001** — N/A (`math`/`computational` inactive).
- **G-002** — N/A: the plan contains no external or irreversible action (no release, package publication, archive deposit or submission; CI artifacts expire after 14 days). If a public release is wanted later, re-run the protocol with `software.deploys = true`.

Risk-mitigation gates (Phase 4.4):

## G-003 Realism sign-off
- **Trigger:** end of S-016.
- **Evidence bundle** (`gates/G-003/`): `results.json` from T-022b and its pass/fail summary; `blind/` with 12 pairs `pair-NN-A.wav`/`pair-NN-B.wav` (one real TinySOL note, one render of the same pitch/dynamic, A/B order from `random.Random(20261001)`), answer key in `blind/key.json.b64` (base64 so it is not read accidentally); `demos/`: chromatic scale Db3–A5 at mf, a legato phrase with vibrato, overblow sweep 0→1 on concert F#3, Harmonic-mode octave run 61–76, each 44.1 kHz WAV; `GATE-G-003.md` summarising metrics and any deviations.
- **Questions:** (1) Without the key, which file in each pair is the real sax? (2) Is the sound acceptable as "realistic" for your use? (3) Is the Overblow/Harmonic behaviour what you wanted?
- **Allowed responses:** `proceed` | `iterate: <notes>` (max 3 iterations of S-016) | `proceed-with-rescope: <text>` | `stop`.
- **Branches:** proceed → S-017; iterate → S-016 with notes, then G-003 again; rescope → record A-0xx, adjust only non-frozen constants/scope, continue S-017; stop → write final report, halt.

## G-004 Fingering chart and UI check
- **Trigger:** end of S-014.
- **Evidence bundle** (`gates/G-004/`): 33 PNG screenshots `written-<NN>-<name>.png` of the editor highlighting each standard fingering (rendered headless via `juce::Component::createComponentSnapshot`), 16 harmonic-mode screenshots (concert 61–76), the fixture file, and `GATE-G-004.md` listing the D-004 choices (bis Bb; F#6 = F6 + high F#; RH low C included for Bb3/B3/C#4).
- **Questions:** (1) Are all highlighted fingerings correct for your saxophone practice? (2) Any key drawn in the wrong place?
- **Allowed responses:** `proceed` | `fix-layout: <keys>` (non-frozen KeyLayout geometry only) | `correct-chart: <notes>` (frozen fixture → `TEST_CHALLENGE.md`, protocol re-run) | `stop`.
- **Branches:** proceed → S-015; fix-layout → amend `KeyLayout.cpp`, re-run T-018/T-029, re-issue G-004; correct-chart → halt per Test Challenge Rule; stop → halt.

## G-003 — blind listening gate `realism`

- **Trigger step:** S-LISTEN after applicable implementation/review prerequisites and before downstream packaging/retrospective work.
- **Evidence bundle:** `GATE-G-003.LISTENING.md` and `listening/G-003/round-001/` containing the machine-readable `result.json`, ordered locked answers, exact score and one-sided binomial p-value, side-bias count, verbatim findings, reference/candidate provenance and checksums, pair-builder settings, render manifest, demo definitions, and `SHA256SUMS`.
- **Frozen primary condition:** 12 matched onset pairs, initial seed `20261001`, common RMS −23 dBFS, 1.0 s clips, 10 ms fades, 44.1 kHz PCM-24 WAV, pass at ≤8/12 with no unresolved qualitative finding. Maximum three total rounds; rounds 2 and 3 use the initial seed +1000/+2000.
- **Questions for the human:** ordered A/B reference choices for all 12 pairs, then the repository-specific demo/weak-spot questions in `plan/LISTENING-AMENDMENT.md`.
- **Allowed responses:** `proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, `stop`.
- **Plan branches:** see `plan/LISTENING-AMENDMENT.md`; downstream resume is `S-017`. `stop` and `blocked` leave this gate open with the reason and next prerequisite recorded.
