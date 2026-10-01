// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-002 (unit) — key identifiers used by fixtures and UI; claim C-010.
#include "sax/Keys.h"
#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>
using namespace sax;

TEST_CASE("T-002 23 keys with unique stable names that round-trip", "[T-002][unit]") {
    STATIC_REQUIRE(kKeyCount == 23);
    const char* expected[] = {"octave","front_f","palm_d","palm_eb","palm_f","lh1","bis","lh2","lh3",
        "g_sharp","low_c_sharp","low_b","low_bb","side_e","side_c","side_bb","high_f_sharp","rh1",
        "rh2","rh3","alt_f_sharp","low_eb","low_c"};
    std::set<std::string> seen;
    for (std::size_t i = 0; i < kKeyCount; ++i) {
        const auto k = static_cast<KeyId>(i);
        CHECK(keyName(k) == expected[i]);
        seen.insert(std::string(keyName(k)));
        const auto back = keyFromName(expected[i]);
        REQUIRE(back.has_value());
        CHECK(*back == k);
    }
    CHECK(seen.size() == kKeyCount);
    CHECK_FALSE(keyFromName("").has_value());
    CHECK_FALSE(keyFromName("Octave").has_value()); // case-sensitive
    CHECK_FALSE(keyFromName("low_a").has_value());  // baritone key not on alto
}
