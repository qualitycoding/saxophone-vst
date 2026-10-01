// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-008, T-009 (integration, code verification against a published model) — claims C-006, C-007, C-014.
// Reference: Colinot, Vergez, Guillemain, Doc, Acta Acustica 5 (2021) 33, Tables 1-2, eq. (17).
// Tolerances: the paper reports F#3 (185 Hz) for this fingering; spike research/spikes/colinot_spike.py
// with the D-005 scheme gives 189.1 Hz. The +-4 % window around 187.5 Hz covers scheme-dependent
// playing-frequency differences (omitted reed-flow term, explicit coupling) — not a tuned value.
#include "sax/Analysis.h"
#include "sax/ColinotReference.h"
#include <catch2/catch_test_macros.hpp>
#include <span>
using namespace sax;

static Regime regimeOf(const ColinotRun& r) {
    std::span<const float> s(r.pressure);
    return classifyRegime(s.subspan(s.size() / 2), r.sampleRate, 187.5);
}

TEST_CASE("T-008 reference model reproduces first register at gamma=0.5", "[T-008][integration]") {
    const auto run = simulateColinot(colinotDSharpTable2(), ReedParams{}, 0.6, 0.5, 0.010, 0.6);
    REQUIRE(run.sampleRate == 176400.0);
    std::span<const float> s(run.pressure);
    const double f0 = estimateF0(s.subspan(s.size() / 2), run.sampleRate);
    CHECK(f0 > 180.0); CHECK(f0 < 195.0);
    CHECK(regimeOf(run) == Regime::FirstRegister);
}

TEST_CASE("T-008 reference model reaches second register at gamma=0.9, tau=3 ms", "[T-008][integration]") {
    const auto run = simulateColinot(colinotDSharpTable2(), ReedParams{}, 0.6, 0.9, 0.003, 0.6);
    CHECK(regimeOf(run) == Regime::SecondRegister);
}

TEST_CASE("T-008 invalid arguments are rejected", "[T-008][integration]") {
    const auto r = colinotDSharpTable2();
    CHECK_THROWS_AS(simulateColinot(r, {}, 0.6, 0.5, 0.01, 0.0), std::invalid_argument);
    CHECK_THROWS_AS(simulateColinot(r, {}, 0.6, 0.5, 0.0, 0.5), std::invalid_argument);
    CHECK_THROWS_AS(simulateColinot(r, {}, 0.6, 0.5, 0.01, 0.5, 8000.0), std::invalid_argument);
    CHECK_THROWS_AS(simulateColinot(ResonatorParams{}, {}, 0.6, 0.5, 0.01, 0.5), std::invalid_argument);
}

TEST_CASE("T-009 harmonic second resonance favours overblowing (basis of D-006)", "[T-009][integration]") {
    // Colinot 2021 sec. 5: f2/f1 = 2 yields more second register than the measured ~2.07.
    // Spike S2 (research/spikes/overblow_map.py): 4/9 vs 8/9 second-register outcomes.
    auto measured = colinotDSharpTable2();
    auto harmonic = measured;
    harmonic.modes[1].pole = {harmonic.modes[1].pole.real(), 2.0 * harmonic.modes[0].pole.imag()};
    int r2m = 0, r2h = 0;
    for (double g : {0.6, 0.7, 0.8})
        for (double tau : {1e-4, 3e-3, 3e-2}) {
            r2m += regimeOf(simulateColinot(measured, {}, 0.6, g, tau, 0.4)) == Regime::SecondRegister;
            r2h += regimeOf(simulateColinot(harmonic, {}, 0.6, g, tau, 0.4)) == Regime::SecondRegister;
        }
    CHECK(r2h > r2m);
    CHECK(r2h >= 6);
}
