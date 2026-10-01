// SPDX-License-Identifier: Apache-2.0
// Concert/written pitch handling for the Eb alto saxophone. MIDI note numbers, C4 = 60.
#pragma once

namespace sax {

inline constexpr int kLowestConcert  = 49; ///< Db3, written Bb3 (C-004)
inline constexpr int kHighestConcert = 81; ///< A5,  written F#6 (C-004, high-F# key)
inline constexpr int kAltoTransposition = 9; ///< written = concert + 9 (major sixth, C-004)
inline constexpr int kLowestWritten  = kLowestConcert  + kAltoTransposition; // 58
inline constexpr int kHighestWritten = kHighestConcert + kAltoTransposition; // 90

/// true iff concertMidi is playable with a standard fingering. Stub sentinel: false.
bool isInRange(int concertMidi) noexcept;

/// Written (transposed) MIDI note. Throws std::out_of_range if !isInRange(concertMidi).
int writtenFromConcert(int concertMidi);

/// Concert MIDI note for a written note. Throws std::out_of_range outside [58, 90].
int concertFromWritten(int writtenMidi);

/// 12-TET frequency in Hz: a4Hz * 2^((midi-69)/12). Stub sentinel: 0.0.
double equalTemperedHz(double midi, double a4Hz = 440.0) noexcept;

} // namespace sax
