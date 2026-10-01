# saxophone-vst

A physically modelled **alto saxophone** virtual instrument (VST3 / AU / Standalone) with an
on-screen saxophone that **lights up the keys** for every note you play, and an **Overblow**
control for overblown and harmonic (second-register) sounds.

> **Status: planning.** The execution plan lives on the `gen-*` branch (see `HANDOFF.md`
> there). No playable build exists yet.

## What it will do

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

## Building (planned)

C++20, CMake ≥ 3.22, JUCE 9.0.3 (fetched automatically). See `plan/ENVIRONMENT.md` on the
planning branch for pinned versions and verified setup commands.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

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
