// SPDX-License-Identifier: Apache-2.0
// Non-frozen tuning constants of the voice (D-006, D-011, D-014). These may be retuned during S-016.
#pragma once

namespace sax::tuning {

// control / smoothing
inline constexpr int    kCtrlBlock       = 16;      // host samples per control update
inline constexpr double kParamSmoothSec  = 0.010;
inline constexpr double kBreathSmoothSec = 0.005;

// blowing pressure gamma (D-011)
inline constexpr double kGammaBase   = 0.40;       // no breath controller: base + slope * velocity
inline constexpr double kGammaVel    = 0.22;
inline constexpr double kBreathBase  = 0.38;       // breath controller: base + slope * breath
inline constexpr double kBreathSlope = 0.24;
inline constexpr double kGammaMax    = 1.10;
inline constexpr double kBreathThreshold = 0.02;

// reed
inline constexpr double kReedOmega = 8448.0;   // rad/s (2 x Table 1): embouchure-adjusted reed frequency, needed above ~600 Hz
inline constexpr double kZetaBase = 0.80;          // zeta = base - hard * hardness
inline constexpr double kZetaHard = 0.40;

// attack: gamma(t) = 0.5 (1 + tanh((t - delay*tau)/tau)), tau = tauBase * (1 - kOverTau * overblow)
inline constexpr double kTauBase      = 0.005;
inline constexpr double kAttackDelay  = 3.0;
inline constexpr double kReleaseSec   = 0.040;
inline constexpr double kIdleQuietSec = 0.050;
inline constexpr double kIdleLevel    = 1e-5;

// overblow (D-006) and harmonic mode (D-007)
inline constexpr double kOverGamma  = 0.6;
inline constexpr double kOverZeta   = 0.25;
inline constexpr double kOverWarp   = 0.8;
inline constexpr double kOverTau    = 0.9;
inline constexpr double kOverNoise  = 1.0;
inline constexpr double kHarmMinOverblow = 0.6;
inline constexpr double kHarmMode1Scale  = 0.1;

// expression (D-011)
inline constexpr double kVibratoCents   = 15.0;
inline constexpr double kVibratoGamma   = 0.04;
inline constexpr double kBendSemitones  = 2.0;
inline constexpr double kMinPortamentoSec = 0.003;

// output stage (D-014)
inline constexpr double kHpHz          = 60.0;
inline constexpr double kDcHz          = 5.0;
inline constexpr double kShelfHz       = 800.0;
inline constexpr double kShelfBaseDb   = -6.0;
inline constexpr double kShelfBrightDb = 9.0;
inline constexpr double kOverBrightDb  = 12.0;      // extra high-shelf gain at overblow = 1
inline constexpr double kNoiseGain     = 0.05;
inline constexpr double kNoiseLoHz     = 1000.0;
inline constexpr double kNoiseHiHz     = 6000.0;
inline constexpr double kOutScale      = 0.6;

} // namespace sax::tuning
