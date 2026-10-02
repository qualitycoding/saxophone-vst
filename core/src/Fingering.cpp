// SPDX-License-Identifier: Apache-2.0
// Standard alto-saxophone fingerings (D-004). Table transcribed from
// tests/fixtures/alto_standard_fingerings.txt by a generator script; the fixture is authoritative.
#include "sax/Fingering.h"
#include "sax/Pitch.h"
#include <array>
#include <initializer_list>
#include <stdexcept>

namespace sax {
namespace {

struct Row {
    int written;
    int reg;
    RegisterHole hole;
    std::initializer_list<KeyId> keys;
};

Fingering make(const Row& r) {
    Fingering f;
    f.writtenMidi = r.written;
    f.targetRegister = r.reg;
    f.hole = r.hole;
    for (KeyId k : r.keys) f.keys.set(index(k));
    return f;
}

constexpr int kRows = kHighestWritten - kLowestWritten + 1; // 33

const std::array<Fingering, kRows>& table() {
    static const std::array<Fingering, kRows> t = [] {
        const Row rows[kRows] = {
    {58, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::LowBb, KeyId::RH1, KeyId::RH2, KeyId::RH3, KeyId::LowC}},
    {59, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::LowB, KeyId::RH1, KeyId::RH2, KeyId::RH3, KeyId::LowC}},
    {60, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1, KeyId::RH2, KeyId::RH3, KeyId::LowC}},
    {61, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::LowCSharp, KeyId::RH1, KeyId::RH2, KeyId::RH3, KeyId::LowC}},
    {62, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1, KeyId::RH2, KeyId::RH3}},
    {63, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1, KeyId::RH2, KeyId::RH3, KeyId::LowEb}},
    {64, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1, KeyId::RH2}},
    {65, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1}},
    {66, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH2}},
    {67, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3}},
    {68, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::GSharp}},
    {69, 1, RegisterHole::None, {KeyId::LH1, KeyId::LH2}},
    {70, 1, RegisterHole::None, {KeyId::LH1, KeyId::Bis}},
    {71, 1, RegisterHole::None, {KeyId::LH1}},
    {72, 1, RegisterHole::None, {KeyId::LH2}},
    {73, 1, RegisterHole::None, {}},
    {74, 2, RegisterHole::Body, {KeyId::Octave, KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1, KeyId::RH2, KeyId::RH3}},
    {75, 2, RegisterHole::Body, {KeyId::Octave, KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1, KeyId::RH2, KeyId::RH3, KeyId::LowEb}},
    {76, 2, RegisterHole::Body, {KeyId::Octave, KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1, KeyId::RH2}},
    {77, 2, RegisterHole::Body, {KeyId::Octave, KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH1}},
    {78, 2, RegisterHole::Body, {KeyId::Octave, KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::RH2}},
    {79, 2, RegisterHole::Body, {KeyId::Octave, KeyId::LH1, KeyId::LH2, KeyId::LH3}},
    {80, 2, RegisterHole::Body, {KeyId::Octave, KeyId::LH1, KeyId::LH2, KeyId::LH3, KeyId::GSharp}},
    {81, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::LH1, KeyId::LH2}},
    {82, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::LH1, KeyId::Bis}},
    {83, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::LH1}},
    {84, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::LH2}},
    {85, 2, RegisterHole::Neck, {KeyId::Octave}},
    {86, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::PalmD}},
    {87, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::PalmD, KeyId::PalmEb}},
    {88, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::PalmD, KeyId::PalmEb, KeyId::SideE}},
    {89, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::PalmD, KeyId::PalmEb, KeyId::PalmF, KeyId::SideE}},
    {90, 2, RegisterHole::Neck, {KeyId::Octave, KeyId::PalmD, KeyId::PalmEb, KeyId::PalmF, KeyId::SideE, KeyId::HighFSharp}},
        };
        std::array<Fingering, kRows> out;
        for (int i = 0; i < kRows; ++i) out[static_cast<std::size_t>(i)] = make(rows[i]);
        return out;
    }();
    return t;
}

} // namespace

const Fingering& standardFingering(int writtenMidi) {
    if (writtenMidi < kLowestWritten || writtenMidi > kHighestWritten)
        throw std::out_of_range("standardFingering: written note outside Bb3..F#6");
    return table()[static_cast<std::size_t>(writtenMidi - kLowestWritten)];
}

std::optional<NoteResolution> resolveNote(int concertMidi, bool harmonicMode) noexcept {
    if (!isInRange(concertMidi)) return std::nullopt;
    NoteResolution r;
    r.concertMidi = concertMidi;
    const int written = concertMidi + kAltoTransposition;
    if (harmonicMode && concertMidi >= 61 && concertMidi <= 76) {
        r.fingering = table()[static_cast<std::size_t>(written - 12 - kLowestWritten)];
        r.harmonicOverblow = true;
    } else {
        r.fingering = table()[static_cast<std::size_t>(written - kLowestWritten)];
        r.harmonicOverblow = false;
    }
    return r;
}

} // namespace sax
