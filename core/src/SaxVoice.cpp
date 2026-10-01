// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "sax/SaxVoice.h"
#include "sax/Errors.h"
#include <algorithm>
namespace sax {
struct SaxVoice::Impl {};
VoiceParameters clamped(const VoiceParameters& p) noexcept { return p; }
SaxVoice::SaxVoice(std::shared_ptr<const ResonatorTable>) { throw NotImplemented("SaxVoice::SaxVoice"); }
SaxVoice::~SaxVoice() = default;
void SaxVoice::prepare(double, int) { throw NotImplemented("SaxVoice::prepare"); }
void SaxVoice::reset() noexcept {}
void SaxVoice::setParameters(const VoiceParameters&) noexcept {}
void SaxVoice::noteOn(int, float) noexcept {}
void SaxVoice::noteOff(int) noexcept {}
void SaxVoice::allNotesOff() noexcept {}
void SaxVoice::setBreath(float) noexcept {}
void SaxVoice::setPitchBend(float) noexcept {}
void SaxVoice::setAftertouch(float) noexcept {}
void SaxVoice::process(float* out, int n) noexcept { if (out && n > 0) std::fill(out, out + n, 0.0f); }
KeySet SaxVoice::currentKeys() const noexcept { return {}; }
int SaxVoice::currentConcertNote() const noexcept { return -1; }
}
