// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-003, T-004, T-005 (unit) — SC-2 highlighted keys; decisions D-004, D-007; claims C-010, C-012.
#include "TestSupport.h"
#include "sax/Fingering.h"
#include "sax/Pitch.h"
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
using namespace sax;

static KeySet keysFrom(const std::vector<std::string>& names) {
    KeySet s;
    for (const auto& n : names) { auto k = keyFromName(n); REQUIRE(k.has_value()); s.set(index(*k)); }
    return s;
}
static RegisterHole holeFrom(const std::string& h) {
    if (h == "none") return RegisterHole::None;
    if (h == "body") return RegisterHole::Body;
    if (h == "neck") return RegisterHole::Neck;
    FAIL("bad hole " << h); return RegisterHole::None;
}

TEST_CASE("T-003 standard fingering chart matches the frozen fixture for all 33 notes", "[T-003][unit]") {
    const auto fx = saxtest::loadFingeringFixture();
    REQUIRE(fx.size() == 33);
    for (const auto& f : fx) {
        INFO("written " << f.written);
        const Fingering& got = standardFingering(f.written);
        CHECK(got.writtenMidi == f.written);
        CHECK(got.keys == keysFrom(f.keys));
        CHECK(got.targetRegister == f.reg);
        CHECK(got.hole == holeFrom(f.hole));
    }
    CHECK_THROWS_AS(standardFingering(57), std::out_of_range);
    CHECK_THROWS_AS(standardFingering(91), std::out_of_range);
}

TEST_CASE("T-004 octave key opens body vent below written A5 and neck vent from A5", "[T-004][unit]") {
    for (int w = kLowestWritten; w <= kHighestWritten; ++w) {
        INFO("written " << w);
        const auto& f = standardFingering(w);
        const bool oct = f.keys.test(index(KeyId::Octave));
        CHECK(oct == (w >= 74));
        if (!oct) CHECK(f.hole == RegisterHole::None);
        else      CHECK(f.hole == (w >= 81 ? RegisterHole::Neck : RegisterHole::Body));
    }
}

TEST_CASE("T-005 resolveNote: standard vs harmonic (overblown) mode", "[T-005][unit]") {
    for (int n = 0; n < 128; ++n) {
        INFO("concert " << n);
        const auto std_ = resolveNote(n, false);
        const auto harm = resolveNote(n, true);
        if (!isInRange(n)) { CHECK_FALSE(std_.has_value()); CHECK_FALSE(harm.has_value()); continue; }
        REQUIRE(std_.has_value()); REQUIRE(harm.has_value());
        CHECK(std_->concertMidi == n);
        CHECK(std_->fingering == standardFingering(n + 9));
        CHECK_FALSE(std_->harmonicOverblow);
        CHECK(harm->concertMidi == n);
        if (n >= 61 && n <= 76) {
            CHECK(harm->harmonicOverblow);
            CHECK(harm->fingering == standardFingering(n + 9 - 12));
            CHECK_FALSE(harm->fingering.keys.test(index(KeyId::Octave)));
        } else {
            CHECK(*harm == *std_);
        }
    }
}
