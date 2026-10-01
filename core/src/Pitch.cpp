// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "sax/Pitch.h"
#include "sax/Errors.h"
namespace sax {
bool isInRange(int) noexcept { return false; }
int writtenFromConcert(int) { throw NotImplemented("writtenFromConcert"); }
int concertFromWritten(int) { throw NotImplemented("concertFromWritten"); }
double equalTemperedHz(double, double) noexcept { return 0.0; }
}
