// SPDX-License-Identifier: Apache-2.0
// Offline signal analysis (never on the audio thread). YIN: de Cheveigne & Kawahara (2002).
#include "sax/Analysis.h"
#include "sax/Errors.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace sax {
namespace {

using Cx = std::complex<double>;

std::size_t nextPow2(std::size_t n) {
    std::size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

void fft(std::vector<Cx>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const double ang = -2.0 * std::numbers::pi / static_cast<double>(len);
        const Cx wl(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            Cx w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; ++k) {
                const Cx u = a[i + k];
                const Cx v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wl;
            }
        }
    }
}

double rms(std::span<const float> x) {
    if (x.empty()) return 0.0;
    double s = 0.0;
    for (float v : x) s += static_cast<double>(v) * static_cast<double>(v);
    return std::sqrt(s / static_cast<double>(x.size()));
}

/// Hann-windowed power spectrum zero-padded to >= padFactor * N. Returns |X|^2 for bins 0..M/2 and bin width.
struct Spectrum {
    std::vector<double> mag; // |X|
    double binHz = 0.0;
};

Spectrum spectrum(std::span<const float> x, double fs, std::size_t padFactor) {
    const std::size_t n = x.size();
    const std::size_t m = nextPow2(n * padFactor);
    std::vector<Cx> a(m, Cx(0.0, 0.0));
    for (std::size_t i = 0; i < n; ++i) {
        const double w = 0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(n));
        a[i] = Cx(static_cast<double>(x[i]) * w, 0.0);
    }
    fft(a);
    Spectrum s;
    s.mag.resize(m / 2 + 1);
    for (std::size_t k = 0; k <= m / 2; ++k) s.mag[k] = std::abs(a[k]);
    s.binHz = fs / static_cast<double>(m);
    return s;
}

} // namespace

double estimateF0(std::span<const float> x, double sampleRate) noexcept {
    try {
        if (x.size() < 64 || sampleRate <= 0.0) return 0.0;
        // use at most the last 1.0 s
        const std::size_t cap = static_cast<std::size_t>(sampleRate);
        std::span<const float> s = x.size() > cap ? x.subspan(x.size() - cap) : x;
        if (rms(s) < 1e-4) return 0.0;

        const std::size_t tauMin = std::max<std::size_t>(2, static_cast<std::size_t>(sampleRate / 2000.0));
        const std::size_t tauMax = std::min<std::size_t>(static_cast<std::size_t>(sampleRate / 50.0), s.size() / 2 - 1);
        if (tauMax <= tauMin + 2) return 0.0;
        const std::size_t W = std::min<std::size_t>(s.size() - tauMax, 8192);
        if (W < 32) return 0.0;
        const std::size_t off = (s.size() - tauMax - W) / 2; // centre the window

        std::vector<double> d(tauMax + 2, 0.0);
        for (std::size_t tau = 1; tau <= tauMax + 1 && tau + W + off <= s.size(); ++tau) {
            double acc = 0.0;
            for (std::size_t j = 0; j < W; ++j) {
                const double diff = static_cast<double>(s[off + j]) - static_cast<double>(s[off + j + tau]);
                acc += diff * diff;
            }
            d[tau] = acc;
        }
        // cumulative mean normalised difference
        std::vector<double> dn(tauMax + 2, 1.0);
        double run = 0.0;
        for (std::size_t tau = 1; tau <= tauMax + 1; ++tau) {
            run += d[tau];
            dn[tau] = run > 0.0 ? d[tau] * static_cast<double>(tau) / run : 1.0;
        }
        std::size_t best = 0;
        for (std::size_t tau = tauMin; tau <= tauMax; ++tau) {
            if (dn[tau] < 0.1) {
                while (tau + 1 <= tauMax && dn[tau + 1] < dn[tau]) ++tau;
                best = tau;
                break;
            }
        }
        if (best == 0) {
            double mn = std::numeric_limits<double>::max();
            for (std::size_t tau = tauMin; tau <= tauMax; ++tau)
                if (dn[tau] < mn) { mn = dn[tau]; best = tau; }
            if (mn > 0.4) return 0.0; // no periodicity
        }
        // parabolic interpolation of the minimum
        double tauF = static_cast<double>(best);
        if (best > 1 && best + 1 <= tauMax + 1) {
            const double a = dn[best - 1], b = dn[best], c = dn[best + 1];
            const double den = a - 2.0 * b + c;
            if (den > 1e-12) tauF += 0.5 * (a - c) / den;
        }
        return sampleRate / tauF;
    } catch (...) {
        return 0.0;
    }
}

Regime classifyRegime(std::span<const float> x, double sampleRate, double expectedR1Hz) noexcept {
    try {
        if (x.empty()) return Regime::Silent;
        const std::span<const float> h = x.subspan(x.size() / 2);
        if (rms(h) < 1e-4) return Regime::Silent;
        const double f0 = estimateF0(h, sampleRate);
        if (!(f0 > 0.0) || !(expectedR1Hz > 0.0)) return Regime::Other;
        const double r = f0 / expectedR1Hz;
        if (r >= 0.94 && r <= 1.06) return Regime::FirstRegister;
        if (r >= 1.88 && r <= 2.20) return Regime::SecondRegister;
        return Regime::Other;
    } catch (...) {
        return Regime::Other;
    }
}

std::vector<double> harmonicLevelsDb(std::span<const float> x, double sampleRate, double f0, int n) {
    if (!(f0 > 0.0) || n < 1) throw std::invalid_argument("harmonicLevelsDb: f0 must be > 0 and n >= 1");
    if (x.empty()) throw std::invalid_argument("harmonicLevelsDb: empty signal");
    const Spectrum sp = spectrum(x, sampleRate, 4);
    const std::size_t nb = sp.mag.size();
    std::vector<double> amp(static_cast<std::size_t>(n), 0.0);
    for (int h = 1; h <= n; ++h) {
        const double fc = f0 * h;
        if (fc >= sampleRate / 2.0) continue;
        const std::size_t lo = static_cast<std::size_t>(std::max(1.0, std::floor(fc * 0.97 / sp.binHz)));
        const std::size_t hi = std::min<std::size_t>(nb - 2, static_cast<std::size_t>(std::ceil(fc * 1.03 / sp.binHz)));
        std::size_t pk = lo;
        for (std::size_t k = lo; k <= hi; ++k)
            if (sp.mag[k] > sp.mag[pk]) pk = k;
        double a = sp.mag[pk];
        if (pk > 0 && pk + 1 < nb && sp.mag[pk - 1] > 0.0 && sp.mag[pk + 1] > 0.0 && a > 0.0) { // log-parabolic peak height
            const double l = std::log(sp.mag[pk - 1]), c = std::log(a), r = std::log(sp.mag[pk + 1]);
            const double den = l - 2.0 * c + r;
            if (den < -1e-12) a = std::exp(c - 0.125 * (r - l) * (r - l) / den);
        }
        amp[static_cast<std::size_t>(h - 1)] = a;
    }
    const double mx = *std::max_element(amp.begin(), amp.end());
    std::vector<double> out(amp.size());
    for (std::size_t i = 0; i < amp.size(); ++i)
        out[i] = (mx > 0.0 && amp[i] > 0.0) ? 20.0 * std::log10(amp[i] / mx) : -200.0;
    return out;
}

double spectralCentroidHz(std::span<const float> x, double sampleRate) noexcept {
    try {
        if (x.empty() || sampleRate <= 0.0) return -1.0;
        const Spectrum sp = spectrum(x, sampleRate, 1);
        double num = 0.0, den = 0.0;
        for (std::size_t k = 0; k < sp.mag.size(); ++k) {
            const double f = static_cast<double>(k) * sp.binHz;
            if (f < 20.0) continue;
            const double p = sp.mag[k] * sp.mag[k];
            num += f * p;
            den += p;
        }
        return den > 0.0 ? num / den : -1.0;
    } catch (...) {
        return -1.0;
    }
}

double cents(double a, double b) noexcept { return 1200.0 * std::log2(a / b); }

} // namespace sax
