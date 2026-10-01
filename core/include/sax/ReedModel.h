// SPDX-License-Identifier: Apache-2.0
// Dimensionless single-reed exciter, Colinot et al. 2021 eqs. (4)-(10).
#pragma once

namespace sax {

struct ReedParams {
    double omegaR = 4224.0; ///< reed angular eigenfrequency, rad/s (Table 1)
    double qR     = 1.0;    ///< reed damping (Table 1)
    double Kc     = 100.0;  ///< lay contact stiffness (Table 1)
    double eta    = 1e-3;   ///< regularisation (Table 1)
};

/// Regularised min(x+1, 0) of eq. (6). Stub sentinel: NaN.
double regularisedMinOpening(double x, double eta) noexcept;

/// Contact force F_c(x+1) = Kc * min(x+1,0)^2, eq. (5) with eq. (6). Stub sentinel: NaN.
double contactForce(double x, const ReedParams& p) noexcept;

/// Reed-channel flow u, eq. (7) with regularisations (9),(10). Stub sentinel: NaN.
double reedFlow(double x, double gamma, double pressure, double zeta, double eta) noexcept;

} // namespace sax
