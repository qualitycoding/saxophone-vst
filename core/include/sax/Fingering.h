// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "sax/Keys.h"
#include <optional>

namespace sax {

/// Which register vent the octave mechanism opens (C-012: neck pip for written A5 and up).
enum class RegisterHole : std::uint8_t { None, Body, Neck };

struct Fingering {
    int writtenMidi = -1;
    KeySet keys;               ///< keys pressed (highlighted in the UI)
    int targetRegister = 1;    ///< 1 or 2 (palm-key notes count as 2: octave key pressed)
    RegisterHole hole = RegisterHole::None;
    friend bool operator==(const Fingering&, const Fingering&) = default;
};

/// Standard fingering for written notes 58..90 per D-004 / tests/fixtures/alto_standard_fingerings.json.
/// Throws std::out_of_range outside [58, 90].
const Fingering& standardFingering(int writtenMidi);

/// Result of mapping an incoming concert MIDI note to what the virtual player does.
struct NoteResolution {
    int concertMidi = -1;       ///< the pitch that should sound
    Fingering fingering;        ///< what the UI shows / which resonator is used
    bool harmonicOverblow = false; ///< true: fingering is an octave below; model driven to register 2
    friend bool operator==(const NoteResolution&, const NoteResolution&) = default;
};

/// D-007. harmonicMode=false: standard fingering of the note.
/// harmonicMode=true and 61 <= concert <= 76: fingering of (written-12) with NO octave key, harmonicOverblow=true.
/// Otherwise identical to harmonicMode=false. Out of range: std::nullopt (also the stub sentinel).
std::optional<NoteResolution> resolveNote(int concertMidi, bool harmonicMode) noexcept;

} // namespace sax
