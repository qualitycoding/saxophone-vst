// SPDX-License-Identifier: Apache-2.0
#include "sax/Keys.h"
#include <array>

namespace sax {
namespace {
constexpr std::array<std::string_view, kKeyCount> kNames = {"octave","front_f","palm_d","palm_eb","palm_f","lh1","bis","lh2","lh3","g_sharp","low_c_sharp","low_b","low_bb","side_e","side_c","side_bb","high_f_sharp","rh1","rh2","rh3","alt_f_sharp","low_eb","low_c"};
}

std::string_view keyName(KeyId key) noexcept {
    const auto i = static_cast<std::size_t>(key);
    return i < kKeyCount ? kNames[i] : std::string_view{};
}

std::optional<KeyId> keyFromName(std::string_view name) noexcept {
    for (std::size_t i = 0; i < kKeyCount; ++i)
        if (kNames[i] == name) return static_cast<KeyId>(i);
    return std::nullopt;
}

} // namespace sax
