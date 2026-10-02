# Assumptions (Phase 0 intake batch, answered 2026-10-01)

The human's reply: "Add a readme, an apache v2 license. Use this repo https://github.com/qualitycoding/saxophone-vst" and a PAT.
Unanswered items adopt the proposed defaults.

| ID | Assumption | Source |
|---|---|---|
| A-001 | Profiles: `software` only; `software.deploys = false`. | default 1 |
| A-002 | Instrument: Eb alto saxophone, concert Db3–A5; MIDI input is concert pitch; UI also shows the written note. | default 4 |
| A-003 | Synthesis: physical model (reed + modal bore), no samples shipped. | default 5 |
| A-004 | "Totally realistic" = frozen objective comparison against real alto recordings (T-022b) **and** a blind listening sign-off by the human (G-003). | default 6 |
| A-005 | Reference recordings: TinySOL alto saxophone (CC BY 4.0), used offline only, never shipped. | default 7 |
| A-006 | "Levers" = the 23 player-operated keys (KeyId); highlight the standard fingering only; no alternates in v1. | default 8 |
| A-007 | Overblow = continuous Overblow knob + Harmonic switch (D-006, D-007); growl and multiphonic presets out of scope. | default 9 |
| A-008 | Monophonic legato, last-note priority, velocity + breath (CC2/CC11), pitch bend ±2 st, aftertouch → vibrato depth, portamento. | default 10 |
| A-009 | Tone controls: reed hardness, mouthpiece brightness, breath noise, vibrato rate/depth (+ tuning A4, output gain). | default 11 |
| A-010 | Performance: < 5 % of one core at 48 kHz/128 (RTF ≤ 0.05, T-016); no added latency; no allocation/locks on the audio thread. | default 12 |
| A-011 | Threat model: low — offline plugin, no network, no personal data. Attack surface: host-supplied state blobs and the embedded data table. | default 13 |
| A-012 | Objective realism thresholds (T-022b): mean harmonic-level difference ≤ 6 dB per dynamic, ≤ 10 dB per note; centroid ratio in [0.8, 1.25] and attack-time ratio in [0.5, 2.0] for ≥ 90 % of notes; centroid increases pp<mf<ff for ≥ 90 % of pitches. Chosen by the planner to reject gross timbre errors while tolerating player/instrument/recording variability (S3 notes spectra "depend very strongly" on player and recording); G-003 is the final arbiter. | planner (no Tier-1 standard) |
| A-013 | Licence: repository source is **Apache-2.0** (human instruction, overrides default 3). JUCE is used under AGPLv3 or a JUCE licence; distribution of binaries is out of scope, so no compliance step is planned beyond THIRD_PARTY_NOTICES.md. | human |
| A-014 | Repository `qualitycoding/saxophone-vst`. README, LICENSE, NOTICE committed to `main` at the human's request; all plan artifacts on the generation branch. Implementation happens on branch `impl/saxophone-v1` created from the generation branch. | human |
| A-015 | Solo hobby maintenance: prefer readability and tests over extensibility. | default 14 |
| A-016 | Execution environment (Option B as amended): single agent, no sub-agents; all "fresh-context" reviews run in-context and are logged as substitutions; tier Fable unavailable → Opus. The human supplied a PAT, so the plan is pushed directly. | human + diagnostic |
| A-017 | Formats: VST3 + Standalone on Windows/macOS/Linux, AU on macOS. Framework JUCE **9.0.3** (current release; intake text said JUCE 8 — see D-001). | default 2 |
| A-018 | CI runners: `ubuntu-24.04`, `macos-15`, `windows-2025` (GitHub-hosted). | planner |
| A-019 | MIDI: omni (all channels). Velocity 0 note-on = note-off. | planner |
| A-020 | Generative-AI imagery: none. The saxophone drawing is programmatic (JUCE Path). | planner |
