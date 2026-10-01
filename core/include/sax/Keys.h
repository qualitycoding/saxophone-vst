// SPDX-License-Identifier: Apache-2.0
// The 23 player-operated keys of a modern alto saxophone that the UI can highlight.
// Names follow the Woodwind Fingering Guide key scheme (C-010).
#pragma once
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace sax {

enum class KeyId : std::uint8_t {
    Octave,      ///< left-thumb octave key                         "octave"
    FrontF,      ///< LH front F (altissimo)                         "front_f"
    PalmD,       ///< LH palm D                                      "palm_d"
    PalmEb,      ///< LH palm Eb                                     "palm_eb"
    PalmF,       ///< LH palm F                                      "palm_f"
    LH1,         ///< LH first finger (B)                            "lh1"
    Bis,         ///< LH bis Bb                                      "bis"
    LH2,         ///< LH second finger (A)                           "lh2"
    LH3,         ///< LH third finger (G)                            "lh3"
    GSharp,      ///< LH pinky G#                                    "g_sharp"
    LowCSharp,   ///< LH pinky low C#                                "low_c_sharp"
    LowB,        ///< LH pinky low B                                 "low_b"
    LowBb,       ///< LH pinky low Bb                                "low_bb"
    SideE,       ///< RH side high E                                 "side_e"
    SideC,       ///< RH side C                                      "side_c"
    SideBb,      ///< RH side Bb                                     "side_bb"
    HighFSharp,  ///< RH side high F#                                "high_f_sharp"
    RH1,         ///< RH first finger (F)                            "rh1"
    RH2,         ///< RH second finger (E)                           "rh2"
    RH3,         ///< RH third finger (D)                            "rh3"
    AltFSharp,   ///< RH alternate (chromatic) F#                    "alt_f_sharp"
    LowEb,       ///< RH pinky low Eb                                "low_eb"
    LowC,        ///< RH pinky low C                                 "low_c"
    Count
};

inline constexpr std::size_t kKeyCount = static_cast<std::size_t>(KeyId::Count); // 23
using KeySet = std::bitset<kKeyCount>;

inline constexpr std::size_t index(KeyId k) noexcept { return static_cast<std::size_t>(k); }

/// Stable snake_case identifier shown in comments above (used in JSON fixtures). Stub sentinel: "".
std::string_view keyName(KeyId key) noexcept;

/// Inverse of keyName. Stub sentinel: std::nullopt.
std::optional<KeyId> keyFromName(std::string_view name) noexcept;

} // namespace sax
