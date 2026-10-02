// SPDX-License-Identifier: Apache-2.0
// Offline reference integrator (D-005): exact exponential modal update with the flow held over the
// step; reed by central differences. Port of research/spikes/colinot_spike.py.
#include "sax/ColinotReference.h"
#include "sax/Errors.h"
#include <cmath>
#include <complex>
#include <stdexcept>
#include <vector>

namespace sax {

ColinotRun simulateColinot(const ResonatorParams& resonator, const ReedParams& reed, double zeta,
                           double gammaFinal, double tauG, double seconds, double sampleRate) {
    if (!(seconds > 0.0)) throw std::invalid_argument("simulateColinot: seconds must be > 0");
    if (!(tauG > 0.0)) throw std::invalid_argument("simulateColinot: tauG must be > 0");
    if (!(sampleRate >= 44100.0)) throw std::invalid_argument("simulateColinot: sampleRate must be >= 44100");
    if (resonator.modes.empty()) throw std::invalid_argument("simulateColinot: no modes");

    using Cx = std::complex<double>;
    const double dt = 1.0 / sampleRate;
    const std::size_t nModes = resonator.modes.size();
    std::vector<Cx> E(nModes), G(nModes), pn(nModes, Cx(0.0, 0.0));
    for (std::size_t i = 0; i < nModes; ++i) {
        const Cx s = resonator.modes[i].pole;
        E[i] = std::exp(s * dt);
        G[i] = resonator.modes[i].residue * (E[i] - 1.0) / s;
    }
    const auto n = static_cast<std::size_t>(seconds * sampleRate);
    ColinotRun run;
    run.sampleRate = sampleRate;
    run.pressure.resize(n);

    const double wr = reed.omegaR, qr = reed.qR;
    const double den = 1.0 + 0.5 * qr * wr * dt;
    double x = 0.0, xp = 0.0;
    for (std::size_t k = 0; k < n; ++k) {
        const double t = static_cast<double>(k) * dt;
        const double g = 0.5 * gammaFinal * (1.0 + std::tanh((t - 5.0 * tauG) / tauG));
        double p = 0.0;
        for (const Cx& v : pn) p += v.real();
        p *= 2.0;
        const double Fc = contactForce(x, reed);
        const double xn = (dt * dt * wr * wr * (p - g + Fc - x) + 2.0 * x - xp + 0.5 * qr * wr * dt * xp) / den;
        xp = x;
        x = xn;
        const double u = reedFlow(x, g, p, zeta, reed.eta);
        for (std::size_t i = 0; i < nModes; ++i) pn[i] = E[i] * pn[i] + G[i] * u;
        run.pressure[k] = static_cast<float>(p);
    }
    return run;
}

ResonatorParams colinotDSharpTable2() {
    // Colinot, Vergez, Guillemain, Doc (2021), Acta Acustica 5:33, Table 2 (D# fingering), CC BY 4.0.
    static const double sRe[8] = {-17.59, -35.50, -65.30, -269.34, -70.32, -166.0, -94.49, -116.5};
    static const double sIm[8] = {1195, 2483, 3727, 4405, 5153, 6177, 6749, 7987};
    static const double c[8] = {176.1, 470.5, 649.4, 328.7, 541.5, 224.9, 382.2, 409.9};
    ResonatorParams r;
    for (int i = 0; i < 8; ++i) r.modes.push_back({{sRe[i], sIm[i]}, {c[i], 0.0}});
    return r;
}

} // namespace sax
