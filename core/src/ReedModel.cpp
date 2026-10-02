// SPDX-License-Identifier: Apache-2.0
// Colinot et al. 2021, eqs. (5)-(10): contact force and Bernoulli reed-channel flow.
#include "sax/ReedModel.h"
#include <cmath>

namespace sax {

double regularisedMinOpening(double x, double eta) noexcept {
    const double o = x + 1.0;
    return 0.5 * (o - std::sqrt(o * o + eta));
}

double contactForce(double x, const ReedParams& p) noexcept {
    const double m = regularisedMinOpening(x, p.eta);
    return p.Kc * m * m;
}

double reedFlow(double x, double gamma, double pressure, double zeta, double eta) noexcept {
    const double o = x + 1.0;
    const double mx = 0.5 * (o + std::sqrt(o * o + eta));
    const double d = gamma - pressure;
    const double sgn = (d > 0.0) - (d < 0.0);
    return zeta * mx * sgn * std::sqrt(std::sqrt(d * d + eta));
}

} // namespace sax
