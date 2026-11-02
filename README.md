# saxophone-vst

A physically modelled **alto saxophone** virtual instrument (VST3 / AU / Standalone) with an
on-screen saxophone that **lights up the keys** for every note you play, and an **Overblow**
control for overblown and harmonic (second-register) sounds.

> **Status: partly implemented, not finished.** `main` holds only the licence files, two
> blind-listening helper scripts and this file. The code is on the **`impl/saxophone-v1`**
> branch (not merged). It has the synthesis core (reed model, resonator table, fingerings,
> pitch, state) and a JUCE plugin with the drawn saxophone and key highlighting, along with
> frozen C++ and Python tests. It stopped at step S-014, waiting for a human check of the
> fingering chart and UI (gate G-004). The realism comparison against TinySOL recordings is
> not done, and the blind listening test (gate G-003) is blocked until it is. No sound
> quality claim has been verified. The plan is on `gen-20261001T211329Z-saxophone-vst-plan`
> (see `HANDOFF.md` there).

## What it is meant to do

- **Sound** – a real-time physical model of a single-reed saxophone: a mass–spring reed with
  mouthpiece-lay contact, a Bernoulli reed-channel flow, and a modal (sum-of-resonances)
  model of the conical bore for every fingering. Realism is checked objectively against
  recordings of a real alto saxophone and then by ear.
- **Keys** – a drawn alto saxophone (octave key, palm keys, side keys, pinky tables, main
  keys) that highlights the standard fingering of the sounding note in real time.
- **Overblow** – a continuous *Overblow* knob that pushes the virtual player's breath and
  embouchure into the edgy, unstable regime where the reed beats hard and the horn can break
  into its upper register, plus a *Harmonic* switch that keeps the low fingering and sounds
  the second register (the UI shows the low fingering, just as a player would finger it).
- **Expression** – velocity, breath controller (CC2/CC11), pitch bend (±2 semitones),
  aftertouch vibrato, monophonic legato with last-note priority and portamento.
- **Tone** – reed hardness, mouthpiece brightness, breath noise, vibrato rate/depth.

Range: concert D♭3–A5 (written B♭3–F♯6). MIDI input is concert pitch.

## Building

C++20, CMake ≥ 3.22, JUCE 9.0.3 (fetched automatically). Build from the implementation
branch, not `main`. See `plan/ENVIRONMENT.md` there for pinned versions and setup commands.

```bash
git checkout impl/saxophone-v1
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Scripts on `main`

`scripts/make_blind_pairs.py` and `scripts/score_blind.py` build and score the blind listening test
that gate G-003 will use.

## Licence

The source code in this repository is licensed under the **Apache License 2.0** – see
[`LICENSE`](LICENSE) and [`NOTICE`](NOTICE).

Third-party components keep their own licences:

- **JUCE** is dual-licensed under the AGPLv3 and the commercial JUCE licence. Binaries you
  build and distribute are subject to the JUCE licence you build them under.
- **VST3 SDK** (bundled with JUCE) – MIT licence since VST 3.8.
- **TinySOL** reference recordings (used only for offline testing, never shipped) –
  CC BY 4.0, Cella et al., IRCAM.
- Saxophone model parameters derived from Colinot, Vergez, Guillemain & Doc (2021),
  *Acta Acustica* 5, 33 (CC BY 4.0).
