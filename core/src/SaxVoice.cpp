// SPDX-License-Identifier: Apache-2.0
// Real-time monophonic saxophone voice (D-005, D-006, D-007, D-008, D-010, D-011, D-014).
//
// Signal flow, per host sample:
//   smoothed parameters -> (every kCtrlBlock samples) control update: resonator interpolation (portamento),
//   pitch scaling (bend / vibrato / A4), modal coefficients E,G  ->  K internal steps of the Colinot reed +
//   modal bore model at K * fs  ->  half-band decimation cascade  ->  radiation/output stage  -> tanh.
// No allocation, locks or exceptions after prepare() (D-010).
#include "sax/SaxVoice.h"
#include "VoiceTuning.h"
#include "sax/Errors.h"
#include "sax/Fingering.h"
#include "sax/Pitch.h"
#include "sax/ReedModel.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace sax {
namespace {

namespace T = tuning;

constexpr int kMaxModes = 16;
constexpr int kNumRes = kHighestWritten - kLowestWritten + 1; // 33
constexpr double kPi = std::numbers::pi;

inline double clampd(double v, double lo, double hi) noexcept { return std::min(std::max(v, lo), hi); }

// ----------------------------------------------------------------------------- resonator data
struct Reso {
    int n = 0;
    std::array<double, kMaxModes> sRe{}, sIm{}, cRe{}, cIm{};
    double tuning = 1.0;
    bool firstRegisterFingering = true; // hole == None (overblow warp applies)
};

Reso makeReso(const ResonatorParams& p, bool noHole) {
    Reso r;
    r.n = static_cast<int>(std::min<std::size_t>(p.modes.size(), kMaxModes));
    for (int i = 0; i < r.n; ++i) {
        const auto& m = p.modes[static_cast<std::size_t>(i)];
        r.sRe[static_cast<std::size_t>(i)] = m.pole.real();
        r.sIm[static_cast<std::size_t>(i)] = m.pole.imag();
        r.cRe[static_cast<std::size_t>(i)] = m.residue.real();
        r.cIm[static_cast<std::size_t>(i)] = m.residue.imag();
    }
    r.tuning = p.tuningScale;
    r.firstRegisterFingering = noHole;
    return r;
}

// ----------------------------------------------------------------------------- half-band decimator
class HalfBand {
public:
    static constexpr int kTaps = 31;
    static constexpr int kHalf = (kTaps - 1) / 2; // 15

    void design(double beta = 8.0) {
        auto i0 = [](double x) {
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 40; ++k) {
                term *= (x / (2.0 * k)) * (x / (2.0 * k));
                sum += term;
            }
            return sum;
        };
        double total = 0.5;
        for (int j = 0; j < kHalf / 2 + 1; ++j) {
            const int m = 2 * j + 1; // odd offsets 1,3,...,15
            const double sinc = std::sin(kPi * m / 2.0) / (kPi * m / 2.0);
            const double r = static_cast<double>(m) / kHalf;
            const double w = i0(beta * std::sqrt(1.0 - r * r)) / i0(beta);
            c_[static_cast<std::size_t>(j)] = 0.5 * sinc * w;
            total += 2.0 * c_[static_cast<std::size_t>(j)];
        }
        for (auto& v : c_) v /= total;
        center_ = 0.5 / total;
        reset();
    }
    void reset() noexcept {
        hist_.fill(0.0);
        pos_ = 0;
        phase_ = false;
    }
    /// Feed one input sample; returns true on every second call with the output in y.
    bool push(double x, double& y) noexcept {
        hist_[static_cast<std::size_t>(pos_)] = x;
        pos_ = (pos_ + 1) & 31;
        phase_ = !phase_;
        if (phase_) return false;
        const int newest = pos_ - 1;
        const int mid = newest - kHalf;
        double acc = center_ * hist_[static_cast<std::size_t>(mid & 31)];
        for (int j = 0; j < kHalf / 2 + 1; ++j) {
            const int m = 2 * j + 1;
            acc += c_[static_cast<std::size_t>(j)] *
                   (hist_[static_cast<std::size_t>((mid - m) & 31)] + hist_[static_cast<std::size_t>((mid + m) & 31)]);
        }
        y = acc;
        return true;
    }

private:
    std::array<double, 32> hist_{};
    std::array<double, 8> c_{};
    double center_ = 0.5;
    int pos_ = 0;
    bool phase_ = false;
};

// ----------------------------------------------------------------------------- simple filters
struct OnePoleHP {
    double a = 0.0, x1 = 0.0, y1 = 0.0;
    void set(double fc, double fs) { a = std::exp(-2.0 * kPi * fc / fs); }
    double process(double x) noexcept {
        const double y = a * (y1 + x - x1);
        x1 = x;
        y1 = y;
        return y;
    }
    void reset() noexcept { x1 = y1 = 0.0; }
};
struct OnePoleLP {
    double a = 0.0, y1 = 0.0;
    void set(double fc, double fs) { a = std::exp(-2.0 * kPi * fc / fs); }
    double process(double x) noexcept {
        y1 = (1.0 - a) * x + a * y1;
        return y1;
    }
    void reset() noexcept { y1 = 0.0; }
};
struct Biquad {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    double process(double x) noexcept {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void reset() noexcept { z1 = z2 = 0.0; }
    /// RBJ high shelf, slope S = 1.
    void highShelf(double fs, double f0, double gainDb) {
        const double A = std::pow(10.0, gainDb / 40.0);
        const double w0 = 2.0 * kPi * f0 / fs;
        const double cw = std::cos(w0), sw = std::sin(w0);
        const double alpha = sw / 2.0 * std::sqrt(2.0);
        const double sq = 2.0 * std::sqrt(A) * alpha;
        const double a0 = (A + 1) - (A - 1) * cw + sq;
        b0 = A * ((A + 1) + (A - 1) * cw + sq) / a0;
        b1 = -2.0 * A * ((A - 1) + (A + 1) * cw) / a0;
        b2 = A * ((A + 1) + (A - 1) * cw - sq) / a0;
        a1 = 2.0 * ((A - 1) - (A + 1) * cw) / a0;
        a2 = ((A + 1) - (A - 1) * cw - sq) / a0;
    }
};

struct Noise {
    std::uint32_t s = 0x5A5A5A5Au;
    double next() noexcept { // white, uniform in [-1, 1)
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        return static_cast<double>(s) / 2147483648.0 - 1.0;
    }
};

struct Held {
    int note = -1;
    float vel = 0.0f;
};

enum class Phase { Idle, Playing, Releasing };

} // namespace

// ============================================================================= Impl
struct SaxVoice::Impl {
    explicit Impl(std::shared_ptr<const ResonatorTable> t) : table(std::move(t)) {
        if (!table) throw std::invalid_argument("SaxVoice: null resonator table");
        for (int w = kLowestWritten; w <= kHighestWritten; ++w) {
            const Fingering& f = standardFingering(w);
            cache[static_cast<std::size_t>(w - kLowestWritten)] = makeReso(table->lookup(w, f.hole), f.hole == RegisterHole::None);
        }
        for (auto& h : half) h.design();
    }

    std::shared_ptr<const ResonatorTable> table;
    std::array<Reso, kNumRes> cache{};

    // configuration
    bool prepared = false;
    double fs = 48000.0, fInt = 192000.0, dt = 1.0 / 192000.0;
    int K = 4, stages = 2, latency = 0;
    std::array<HalfBand, 3> half{};

    // parameters (clamped target) and smoothed copies
    VoiceParameters par{};
    double oS = 0, brS = 0.5, nzS = 0.3, vdS = 0, vrS = 5.5, gainS = 1.0, bendS = 0, atS = 0, velS = 0.7, hardS = 0.5, bS = 0;
    double bendT = 0, atT = 0, bT = -1.0, velT = 0.7;
    bool breathPresent = false;
    double smoothA = 0.01, breathA = 0.02;

    // notes
    std::array<Held, 16> held{};
    int nHeld = 0;
    int curNote = -1; // note whose resonator is active (also during release)
    bool curHarmonic = false;
    Phase phase = Phase::Idle;
    double tAtt = 0.0, relGain = 0.0;
    int quiet = 0;

    // resonators
    Reso from{}, to{}, eff{};
    double portProg = 1.0, portInc = 1.0;

    // modal + reed state
    std::array<double, kMaxModes> pr{}, pi{}, er{}, ei{}, gr{}, gi{};
    int nModes = 0;
    double x = 0.0, xp = 0.0;
    double zeta = 0.6;
    double kA = 0, c1 = 0, invDen = 1;
    ReedParams reed{};
    double uAcc = 0.0, uEnv = 0.0;

    // control state
    int ctrlCount = 0;
    double vibPhase = 0.0, vib = 0.0, tauG = T::kTauBase, oEff = 0.0;
    double gammaTarget = 0.0;

    // output stage
    OnePoleHP hp, dcb, nzHp;
    OnePoleLP nzLp;
    Biquad shelf;
    double shelfDb = 1e9;
    Noise noise;

    // UI
    std::atomic<std::uint32_t> keysMask{0};
    std::atomic<int> noteAtomic{-1};

    // ------------------------------------------------------------------------- setup
    void prepare(double sampleRate) {
        fs = sampleRate;
        const int k = static_cast<int>(std::ceil(176400.0 / fs));
        K = 1;
        while (K < k) K <<= 1;
        K = std::min(K, 8);
        stages = (K == 1) ? 0 : (K == 2 ? 1 : (K == 4 ? 2 : 3));
        fInt = fs * K;
        dt = 1.0 / fInt;
        double lat = 0.0; // latency of the cascade in host samples
        for (int s = 0; s < stages; ++s) lat += HalfBand::kHalf * std::pow(2.0, s) / K;
        latency = static_cast<int>(std::ceil(lat));

        reed.omegaR = T::kReedOmega;
        kA = dt * dt * reed.omegaR * reed.omegaR;
        c1 = 0.5 * reed.qR * reed.omegaR * dt;
        invDen = 1.0 / (1.0 + c1);
        smoothA = 1.0 - std::exp(-1.0 / (T::kParamSmoothSec * fs));
        breathA = 1.0 - std::exp(-1.0 / (T::kBreathSmoothSec * fs));
        hp.set(T::kHpHz, fs);
        dcb.set(T::kDcHz, fs);
        nzHp.set(T::kNoiseLoHz, fs);
        nzLp.set(std::min(T::kNoiseHiHz, 0.45 * fs), fs);
        prepared = true;
        resetAll();
    }

    void resetAll() noexcept {
        pr.fill(0);
        pi.fill(0);
        er.fill(0);
        ei.fill(0);
        gr.fill(0);
        gi.fill(0);
        x = xp = 0.0;
        uAcc = uEnv = 0.0;
        for (auto& h : half) h.reset();
        hp.reset();
        dcb.reset();
        nzHp.reset();
        nzLp.reset();
        shelf.reset();
        shelfDb = 1e9;
        noise = Noise{};
        nHeld = 0;
        curNote = -1;
        curHarmonic = false;
        phase = Phase::Idle;
        tAtt = 0.0;
        relGain = 0.0;
        quiet = 0;
        portProg = 1.0;
        ctrlCount = 0;
        vibPhase = 0.0;
        vib = 0.0;
        oS = par.overblow;
        brS = par.brightness;
        nzS = par.breathNoise;
        vdS = par.vibratoDepth;
        vrS = par.vibratoRateHz;
        gainS = std::pow(10.0, static_cast<double>(par.outputGainDb) / 20.0);
        hardS = par.reedHardness;
        bendS = bendT;
        atS = atT;
        bS = breathPresent ? std::max(0.0, bT) : 0.0;
        velS = velT;
        publish(-1, KeySet{});
    }

    void setParams(const VoiceParameters& p) noexcept {
        const VoiceParameters c = clamped(p);
        const bool harmChanged = c.harmonicMode != par.harmonicMode;
        par = c;
        if (harmChanged && prepared && nHeld > 0) retarget(false);
    }

    void publish(int note, const KeySet& keys) noexcept {
        noteAtomic.store(note, std::memory_order_relaxed);
        keysMask.store(static_cast<std::uint32_t>(keys.to_ulong()), std::memory_order_relaxed);
    }

    // ------------------------------------------------------------------------- notes
    void noteOn(int n, float v) noexcept {
        if (!isInRange(n) || !prepared) return;
        removeHeld(n);
        if (nHeld == static_cast<int>(held.size())) {
            for (int i = 1; i < nHeld; ++i) held[static_cast<std::size_t>(i - 1)] = held[static_cast<std::size_t>(i)];
            --nHeld;
        }
        held[static_cast<std::size_t>(nHeld++)] = {n, v};
        const bool wasSilent = (phase != Phase::Playing);
        velT = clampd(static_cast<double>(v), 0.0, 1.0);
        if (phase == Phase::Idle) velS = velT;
        retarget(wasSilent);
    }

    void noteOff(int n) noexcept {
        if (!prepared) return;
        const int before = nHeld;
        removeHeld(n);
        if (before == nHeld) return;
        if (nHeld > 0) {
            velT = clampd(static_cast<double>(held[static_cast<std::size_t>(nHeld - 1)].vel), 0.0, 1.0);
            retarget(false);
        } else if (phase == Phase::Playing) {
            phase = Phase::Releasing;
            quiet = 0;
            publish(-1, KeySet{});
        }
    }

    void allNotesOff() noexcept {
        nHeld = 0;
        if (phase == Phase::Playing) {
            phase = Phase::Releasing;
            quiet = 0;
        }
        publish(-1, KeySet{});
    }

    void removeHeld(int n) noexcept {
        int w = 0;
        for (int i = 0; i < nHeld; ++i)
            if (held[static_cast<std::size_t>(i)].note != n) held[static_cast<std::size_t>(w++)] = held[static_cast<std::size_t>(i)];
        nHeld = w;
    }

    /// Resolve the top held note to a resonator and fingering; start / continue the sound.
    void retarget(bool freshAttack) noexcept {
        if (nHeld == 0) return;
        const int n = held[static_cast<std::size_t>(nHeld - 1)].note;
        const auto res = resolveNote(n, par.harmonicMode);
        if (!res) return;
        const int written = res->fingering.writtenMidi;
        Reso target = cache[static_cast<std::size_t>(written - kLowestWritten)];
        if (res->harmonicOverblow && target.n >= 2) { // D-007
            target.sIm[1] = 2.0 * target.sIm[0];
            target.cRe[0] *= T::kHarmMode1Scale;
            target.cIm[0] *= T::kHarmMode1Scale;
        }
        curHarmonic = res->harmonicOverblow;
        curNote = n;

        if (phase == Phase::Idle) {
            resetStates();
            from = to = eff = target;
            portProg = 1.0;
        } else {
            from = eff;
            to = target;
            const double sec = std::max(T::kMinPortamentoSec, static_cast<double>(par.portamentoMs) * 0.001);
            portInc = (T::kCtrlBlock / fs) / sec;
            portProg = 0.0;
        }
        if (freshAttack || phase == Phase::Idle) {
            tAtt = 0.0;
            relGain = 1.0;
        }
        phase = Phase::Playing;
        quiet = 0;
        publish(n, res->fingering.keys);
        ctrlCount = 0; // refresh coefficients on the next sample
    }

    void resetStates() noexcept {
        pr.fill(0);
        pi.fill(0);
        x = xp = 0.0;
        uEnv = uAcc = 0.0;
        hp.reset();
        dcb.reset();
        shelf.reset();
        nzHp.reset();
        nzLp.reset();
        for (auto& h : half) h.reset();
    }

    void goIdle() noexcept {
        phase = Phase::Idle;
        resetStates();
        relGain = 0.0;
        publish(-1, KeySet{});
    }

    // ------------------------------------------------------------------------- control rate
    void smooth() noexcept {
        oS += smoothA * (par.overblow - oS);
        brS += smoothA * (par.brightness - brS);
        nzS += smoothA * (par.breathNoise - nzS);
        vdS += smoothA * (par.vibratoDepth - vdS);
        vrS += smoothA * (par.vibratoRateHz - vrS);
        hardS += smoothA * (par.reedHardness - hardS);
        gainS += smoothA * (std::pow(10.0, static_cast<double>(par.outputGainDb) / 20.0) - gainS);
        bendS += smoothA * (bendT - bendS);
        atS += smoothA * (atT - atS);
        velS += smoothA * (velT - velS);
        bS += breathA * ((breathPresent ? std::max(0.0, bT) : 0.0) - bS);
    }

    void controlUpdate() noexcept {
        const double ctrlDt = T::kCtrlBlock / fs;
        vibPhase += 2.0 * kPi * vrS * ctrlDt;
        if (vibPhase > 2.0 * kPi) vibPhase -= 2.0 * kPi;
        const double depthEff = vdS * (0.3 + 0.7 * atS);
        vib = std::sin(vibPhase) * depthEff;

        if (portProg < 1.0) portProg = std::min(1.0, portProg + portInc);
        const double a = portProg;
        eff.n = std::max(from.n, to.n);
        for (int i = 0; i < eff.n; ++i) {
            const auto k = static_cast<std::size_t>(i);
            eff.sRe[k] = (1 - a) * from.sRe[k] + a * to.sRe[k];
            eff.sIm[k] = (1 - a) * from.sIm[k] + a * to.sIm[k];
            eff.cRe[k] = (1 - a) * from.cRe[k] + a * to.cRe[k];
            eff.cIm[k] = (1 - a) * from.cIm[k] + a * to.cIm[k];
        }
        eff.tuning = (1 - a) * from.tuning + a * to.tuning;
        eff.firstRegisterFingering = to.firstRegisterFingering;
        nModes = eff.n;

        oEff = curHarmonic ? std::max(oS, T::kHarmMinOverblow) : oS;
        double sIm1 = eff.n >= 2 ? eff.sIm[1] : 0.0;
        if (eff.firstRegisterFingering && !curHarmonic && eff.n >= 2) // D-006: pull f2/f1 toward 2
            sIm1 -= T::kOverWarp * oEff * (eff.sIm[1] - 2.0 * eff.sIm[0]);

        const double m = eff.tuning * (static_cast<double>(par.tuningA4Hz) / 440.0) *
                         std::exp2(bendS * T::kBendSemitones / 12.0) * std::exp2(T::kVibratoCents * vib / 1200.0);
        for (int i = 0; i < nModes; ++i) {
            const auto k = static_cast<std::size_t>(i);
            const double re = eff.sRe[k];
            const double im = (i == 1 ? sIm1 : eff.sIm[k]) * m;
            const double ex = std::exp(re * dt);
            const double ang = im * dt;
            const double Er = ex * std::cos(ang), Ei = ex * std::sin(ang);
            er[k] = Er;
            ei[k] = Ei;
            const double nr = eff.cRe[k] * (Er - 1.0) - eff.cIm[k] * Ei;
            const double ni = eff.cRe[k] * Ei + eff.cIm[k] * (Er - 1.0);
            const double den = re * re + im * im;
            gr[k] = (nr * re + ni * im) / den;
            gi[k] = (ni * re - nr * im) / den;
        }
        for (int i = nModes; i < kMaxModes; ++i) {
            const auto k = static_cast<std::size_t>(i);
            er[k] = ei[k] = gr[k] = gi[k] = 0.0;
            pr[k] = pi[k] = 0.0;
        }

        zeta = (T::kZetaBase - T::kZetaHard * hardS) * (1.0 + T::kOverZeta * oEff);
        tauG = T::kTauBase * (1.0 - T::kOverTau * oEff);
        double base;
        if (breathPresent) base = bS < T::kBreathThreshold ? 0.0 : T::kBreathBase + T::kBreathSlope * bS;
        else base = T::kGammaBase + T::kGammaVel * velS;
        gammaTarget = std::min(T::kGammaMax, base * (1.0 + T::kOverGamma * oEff) * (1.0 + T::kVibratoGamma * vib));

        const double db = T::kShelfBaseDb + T::kShelfBrightDb * brS;
        if (std::abs(db - shelfDb) > 0.02) {
            shelf.highShelf(fs, std::min(T::kShelfHz, 0.4 * fs), db);
            shelfDb = db;
        }
    }

    // ------------------------------------------------------------------------- audio rate
    inline double internalStep(double g) noexcept {
        double ps = 0.0;
        for (int i = 0; i < nModes; ++i) ps += pr[static_cast<std::size_t>(i)];
        const double p = 2.0 * ps;
        double o = x + 1.0;
        const double mn = 0.5 * (o - std::sqrt(o * o + reed.eta));
        const double Fc = reed.Kc * mn * mn;
        const double xn = (kA * (p - g + Fc - x) + 2.0 * x - xp + c1 * xp) * invDen;
        xp = x;
        x = xn;
        o = x + 1.0;
        const double mx = 0.5 * (o + std::sqrt(o * o + reed.eta));
        const double d = g - p;
        const double u = zeta * mx * d / std::sqrt(std::sqrt(d * d + reed.eta));
        for (int i = 0; i < nModes; ++i) {
            const auto k = static_cast<std::size_t>(i);
            const double r = er[k] * pr[k] - ei[k] * pi[k] + gr[k] * u;
            const double im = er[k] * pi[k] + ei[k] * pr[k] + gi[k] * u;
            pr[k] = r;
            pi[k] = im;
        }
        uAcc += std::abs(u);
        return p;
    }

    bool cascade(double xin, double& y) noexcept {
        double v = xin;
        for (int s = 0; s < stages; ++s)
            if (!half[static_cast<std::size_t>(s)].push(v, v)) return false;
        y = v;
        return true;
    }

    void process(float* out, int n) noexcept {
        if (!out || n <= 0) return;
        if (!prepared) {
            std::fill(out, out + n, 0.0f);
            return;
        }
        for (int i = 0; i < n; ++i) {
            smooth();
            if (phase == Phase::Idle) {
                out[i] = 0.0f;
                continue;
            }
            if (ctrlCount == 0) controlUpdate();
            ctrlCount = (ctrlCount + 1) % T::kCtrlBlock;

            // blowing pressure envelope
            const double env = 0.5 * (1.0 + std::tanh((tAtt - T::kAttackDelay * tauG) / tauG));
            if (phase == Phase::Releasing) relGain = std::max(0.0, relGain - 1.0 / (T::kReleaseSec * fs));
            else tAtt += 1.0 / fs;
            const double g = gammaTarget * env * relGain;

            uAcc = 0.0;
            double y = 0.0;
            for (int k = 0; k < K; ++k) {
                const double p = internalStep(g);
                double yy;
                if (cascade(p, yy)) y = yy;
            }
            uEnv += 0.002 * (uAcc / K - uEnv);

            if (!std::isfinite(y) || std::abs(y) > 50.0) { // numerical safety net
                goIdle();
                out[i] = 0.0f;
                continue;
            }
            if (phase == Phase::Releasing && relGain <= 0.0) {
                quiet = (std::abs(y) < T::kIdleLevel) ? quiet + 1 : 0;
                if (quiet >= static_cast<int>(T::kIdleQuietSec * fs)) {
                    goIdle();
                    out[i] = 0.0f;
                    continue;
                }
            }

            double s = hp.process(y);
            s = dcb.process(s);
            s = shelf.process(s);
            const double velGain = breathPresent ? 1.0 : 0.5 + 0.5 * velS;
            const double nz = nzLp.process(nzHp.process(noise.next()));
            const double noiseAmp = nzS * T::kNoiseGain * (1.0 + T::kOverNoise * oEff) * uEnv;
            const double v = std::tanh(T::kOutScale * gainS * velGain * s + nz * noiseAmp);
            out[i] = static_cast<float>(v);
        }
    }
};

// ============================================================================= public API
VoiceParameters clamped(const VoiceParameters& p) noexcept {
    const VoiceParameters d{};
    auto cl = [](float v, float lo, float hi, float def) noexcept {
        if (std::isnan(v)) return def;
        return std::min(std::max(v, lo), hi);
    };
    VoiceParameters r;
    r.overblow = cl(p.overblow, 0.0f, 1.0f, d.overblow);
    r.harmonicMode = p.harmonicMode;
    r.reedHardness = cl(p.reedHardness, 0.0f, 1.0f, d.reedHardness);
    r.brightness = cl(p.brightness, 0.0f, 1.0f, d.brightness);
    r.breathNoise = cl(p.breathNoise, 0.0f, 1.0f, d.breathNoise);
    r.vibratoRateHz = cl(p.vibratoRateHz, 3.0f, 8.0f, d.vibratoRateHz);
    r.vibratoDepth = cl(p.vibratoDepth, 0.0f, 1.0f, d.vibratoDepth);
    r.portamentoMs = cl(p.portamentoMs, 0.0f, 500.0f, d.portamentoMs);
    r.tuningA4Hz = cl(p.tuningA4Hz, 415.0f, 466.0f, d.tuningA4Hz);
    r.outputGainDb = cl(p.outputGainDb, -24.0f, 12.0f, d.outputGainDb);
    return r;
}

SaxVoice::SaxVoice(std::shared_ptr<const ResonatorTable> table) : impl_(std::make_unique<Impl>(std::move(table))) {}
SaxVoice::~SaxVoice() = default;

void SaxVoice::prepare(double sampleRate, int maxBlockSize) {
    if (!(sampleRate >= 22050.0 && sampleRate <= 192000.0)) throw std::invalid_argument("SaxVoice::prepare: sampleRate outside [22050, 192000]");
    if (maxBlockSize < 1) throw std::invalid_argument("SaxVoice::prepare: maxBlockSize must be >= 1");
    impl_->prepare(sampleRate);
}
void SaxVoice::reset() noexcept {
    if (impl_->prepared) impl_->resetAll();
}
void SaxVoice::setParameters(const VoiceParameters& p) noexcept { impl_->setParams(p); }
void SaxVoice::noteOn(int concertMidi, float velocity) noexcept { impl_->noteOn(concertMidi, velocity); }
void SaxVoice::noteOff(int concertMidi) noexcept { impl_->noteOff(concertMidi); }
void SaxVoice::allNotesOff() noexcept { impl_->allNotesOff(); }
void SaxVoice::setBreath(float amount01) noexcept {
    if (std::isnan(amount01)) return;
    if (amount01 < 0.0f) {
        impl_->breathPresent = false;
        impl_->bT = -1.0;
    } else {
        impl_->breathPresent = true;
        impl_->bT = std::min(1.0, static_cast<double>(amount01));
    }
}
void SaxVoice::setPitchBend(float v) noexcept { impl_->bendT = std::isnan(v) ? 0.0 : clampd(static_cast<double>(v), -1.0, 1.0); }
void SaxVoice::setAftertouch(float v) noexcept { impl_->atT = std::isnan(v) ? 0.0 : clampd(static_cast<double>(v), 0.0, 1.0); }
void SaxVoice::process(float* out, int numSamples) noexcept { impl_->process(out, numSamples); }
int SaxVoice::latencySamples() const noexcept { return impl_->latency; }
KeySet SaxVoice::currentKeys() const noexcept { return KeySet(impl_->keysMask.load(std::memory_order_relaxed)); }
int SaxVoice::currentConcertNote() const noexcept { return impl_->noteAtomic.load(std::memory_order_relaxed); }

} // namespace sax
