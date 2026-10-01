// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-011, T-012, T-013 (integration) — SC-5 overblow behaviour; decisions D-006, D-007;
// claims C-007, C-012, C-014, C-015.
#include "TestSupport.h"
#include "sax/Analysis.h"
#include "sax/Pitch.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
using namespace sax;

TEST_CASE("T-011 with Overblow at 0, low fingerings speak in the first register", "[T-011][integration]") {
    for (int n = 49; n <= 64; ++n)
        for (float v : {0.3f, 0.6f, 0.9f}) {
            INFO("concert " << n << " velocity " << v);
            const auto x = saxtest::steady(saxtest::renderNote(48000, n, v, 1.0), 48000);
            CHECK(classifyRegime(x, 48000, equalTemperedHz(n)) == Regime::FirstRegister);
        }
}

TEST_CASE("T-012 Harmonic mode sounds the note as the second register of the low fingering", "[T-012][integration]") {
    VoiceParameters p; p.harmonicMode = true; p.overblow = 0.5f;
    for (int n = 61; n <= 76; ++n) {
        INFO("concert " << n);
        const auto x = saxtest::steady(saxtest::renderNote(48000, n, 0.7f, 1.0, p), 48000);
        CHECK(classifyRegime(x, 48000, equalTemperedHz(n - 12)) == Regime::SecondRegister);
        // +-70 cents: un-vented second register is naturally sharp/flat by tens of cents (C-015).
        CHECK(std::abs(cents(estimateF0(x, 48000), equalTemperedHz(n))) <= 70.0);
    }
}

TEST_CASE("T-013 Overblow brightens the tone monotonically", "[T-013][integration]") {
    for (int n : {56, 61, 68}) {
        INFO("concert " << n);
        double c[3]; int i = 0;
        for (float o : {0.0f, 0.5f, 1.0f}) {
            VoiceParameters p; p.overblow = o;
            c[i++] = spectralCentroidHz(saxtest::steady(saxtest::renderNote(48000, n, 0.7f, 1.0, p), 48000), 48000);
        }
        CHECK(c[0] > 0.0);
        CHECK(c[1] > c[0]);
        CHECK(c[2] > c[1]);
    }
}
