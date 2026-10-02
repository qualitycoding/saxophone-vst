// SPDX-License-Identifier: Apache-2.0
// sax_calibrate: per-fingering tuning calibration (D-008). For every table entry the tuning_scale is
// iterated until the voice sounds within <= tol cents of equal temperament (velocity 0.6, 48 kHz,
// steady part of a 1 s render, A4 = 440 Hz). Run after tools/resonator/generate_table.py.
//   sax_calibrate --in data/alto_resonators.json --out data/alto_resonators.json [--tol 2] [--iters 25]
#include "sax/Analysis.h"
#include "sax/Pitch.h"
#include "sax/ResonatorTable.h"
#include "sax/SaxVoice.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

using nlohmann::json;

namespace {

constexpr double kFs = 48000.0;

double measureHz(const json& doc, int concertNote, bool harmonic = false) {
    auto table = std::make_shared<const sax::ResonatorTable>(sax::ResonatorTable::fromJson(doc.dump()));
    sax::SaxVoice v(table);
    v.prepare(kFs, 256);
    v.setBreath(-1.0f);
    sax::VoiceParameters vp;
    vp.harmonicMode = harmonic;
    v.setParameters(vp);
    v.noteOn(concertNote, harmonic ? 0.7f : 0.6f);
    std::vector<float> out(static_cast<std::size_t>(kFs));
    for (std::size_t i = 0; i < out.size(); i += 256)
        v.process(out.data() + i, static_cast<int>(std::min<std::size_t>(256, out.size() - i)));
    const std::vector<float> steady(out.begin() + static_cast<long>(0.3 * kFs), out.end());
    return sax::estimateF0(steady, kFs);
}

} // namespace

int main(int argc, char** argv) {
    std::string in, out;
    double tol = 2.0;
    int maxIters = 25;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--in" && i + 1 < argc) in = argv[++i];
        else if (a == "--out" && i + 1 < argc) out = argv[++i];
        else if (a == "--tol" && i + 1 < argc) tol = std::atof(argv[++i]);
        else if (a == "--iters" && i + 1 < argc) maxIters = std::atoi(argv[++i]);
        else { std::cerr << "unknown argument " << a << "\n"; return 2; }
    }
    if (in.empty() || out.empty()) { std::cerr << "usage: sax_calibrate --in FILE --out FILE [--tol CENTS] [--iters N]\n"; return 2; }

    std::ifstream f(in);
    if (!f) { std::cerr << "cannot open " << in << "\n"; return 2; }
    std::stringstream ss;
    ss << f.rdbuf();
    json doc = json::parse(ss.str());

    int failures = 0;
    for (json& e : doc.at("entries")) {
        const int written = e.at("written").get<int>();
        const int note = written - sax::kAltoTransposition;
        const double target = sax::equalTemperedHz(note);
        double scale = e.value("tuning_scale", 1.0);
        double err = 1e9, f0 = 0.0;
        int it = 0;
        for (; it < maxIters; ++it) {
            e["tuning_scale"] = scale;
            f0 = measureHz(doc, note);
            if (f0 <= 0.0) { scale *= 1.01; continue; }
            err = sax::cents(f0, target);
            if (std::abs(err) <= tol) break;
            const double step = std::clamp(target / f0, 0.95, 1.05);
            scale = std::clamp(scale * step, 0.8, 1.25);
        }
        const bool ok = std::abs(err) <= tol;
        if (!ok) ++failures;
        std::printf("written %2d (concert %2d)  scale %.5f  f0 %.2f Hz  err %+.2f cents  iters %2d  %s\n", written, note, scale, f0, err, it, ok ? "ok" : "NOT CONVERGED");
    }
    // Harmonic mode (D-007): per low fingering, calibrate the second-register pitch (note + 12) at velocity 0.7.
    for (json& e : doc.at("entries")) {
        const int written = e.at("written").get<int>();
        const int low = written - sax::kAltoTransposition;
        if (e.at("hole").get<std::string>() != "none" || low + 12 > sax::kHighestConcert || low + 12 < 61) continue;
        const double target = sax::equalTemperedHz(low + 12);
        // The pitch-vs-stretch curve is monotone but can jump by tens of cents where the oscillation
        // re-locks to the bore partials, so search a grid and refine locally instead of iterating.
        auto errAt = [&](double st, double& hz) {
            e["harmonic_stretch"] = st;
            hz = measureHz(doc, low + 12, true);
            return hz > 0.0 ? std::abs(sax::cents(hz, target)) : 1e9;
        };
        double best = 1.0, bestErr = 1e9, f0 = 0.0, hz = 0.0;
        for (double st = 0.99; st <= 1.0801; st += 0.005) {
            const double er = errAt(st, hz);
            if (er < bestErr) { bestErr = er; best = st; f0 = hz; }
        }
        const double center = best;
        for (double st = center - 0.0045; st <= center + 0.00451; st += 0.0005) {
            const double er = errAt(st, hz);
            if (er < bestErr) { bestErr = er; best = st; f0 = hz; }
        }
        e["harmonic_stretch"] = best;
        const double stretch = best;
        const double err = f0 > 0.0 ? sax::cents(f0, target) : 1e9;
        const int it = 0;
        const bool ok = std::abs(err) <= 40.0;
        std::printf("harmonic  %2d (concert %2d)  stretch %.5f  f0 %.2f Hz  err %+.2f cents  iters %2d  %s\n", written, low + 12, stretch, f0, err, it, ok ? "ok" : "NOT CONVERGED");
    }
    doc["provenance"]["calibrated"] = "tools/render/calibrate_tuning.cpp; 48 kHz, velocity 0.6, A4 440";
    std::ofstream o(out);
    o << doc.dump(1) << "\n";
    std::printf("%d entries not converged\n", failures);
    return failures == 0 ? 0 : 1;
}
