// SPDX-License-Identifier: Apache-2.0
// Modal description of the bore input impedance per fingering (Colinot et al. 2021, eq. 11).
#pragma once
#include "sax/Fingering.h"
#include <complex>
#include <cstddef>
#include <string_view>
#include <vector>

namespace sax {

struct Mode {
    std::complex<double> pole;    ///< s_n in rad/s, Re < 0
    std::complex<double> residue; ///< C_n, dimensionless impedance Z/Zc
};

struct ResonatorParams {
    std::vector<Mode> modes;      ///< 4 <= size <= 16, sorted by Im(pole) ascending
};

/// Immutable table loaded from data/alto_resonators.json (schema "saxophone-vst/resonators@1",
/// documented in plan/DECISIONS.md D-009).
class ResonatorTable {
public:
    /// Throws ParseError on malformed JSON, wrong schema, unstable pole (Re >= 0),
    /// unsorted modes, mode count outside [4,16], or duplicate (written, hole) entries.
    static ResonatorTable fromJson(std::string_view json);

    /// Throws std::out_of_range if the (writtenMidi, hole) pair is absent.
    const ResonatorParams& lookup(int writtenMidi, RegisterHole hole) const;

    std::size_t size() const noexcept;

private:
    struct Entry { int written; RegisterHole hole; ResonatorParams params; };
    std::vector<Entry> entries_;
};

/// Embedded copy of data/alto_resonators.json compiled into the binary (generated at build time).
/// Throws NotImplemented until S-008.
std::string_view embeddedResonatorJson();

} // namespace sax
