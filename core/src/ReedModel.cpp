// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "sax/ReedModel.h"
#include <limits>
namespace sax {
namespace { constexpr double nan = std::numeric_limits<double>::quiet_NaN(); }
double regularisedMinOpening(double, double) noexcept { return nan; }
double contactForce(double, const ReedParams&) noexcept { return nan; }
double reedFlow(double, double, double, double, double) noexcept { return nan; }
}
