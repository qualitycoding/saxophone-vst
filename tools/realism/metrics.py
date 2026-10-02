# SPDX-License-Identifier: Apache-2.0
"""Timbre metrics for the realism comparison (D-015). Mirrors core/src/Analysis.cpp."""
from __future__ import annotations

import numpy as np


def _rms(x: np.ndarray) -> float:
    return float(np.sqrt(np.mean(np.square(x)))) if len(x) else 0.0


def f0_yin(x: np.ndarray, fs: float) -> float:
    """YIN f0 in Hz over the whole signal (threshold 0.1); 0.0 for silence (RMS < 1e-4)."""
    x = np.asarray(x, dtype=np.float64)
    x = x[-int(fs):] if len(x) > int(fs) else x
    if len(x) < 64 or _rms(x) < 1e-4:
        return 0.0
    tau_min = max(2, int(fs / 2000.0))
    tau_max = min(int(fs / 50.0), len(x) // 2 - 1)
    if tau_max <= tau_min + 2:
        return 0.0
    w = min(len(x) - tau_max, 8192)
    if w < 32:
        return 0.0
    off = (len(x) - tau_max - w) // 2
    seg = x[off:off + w + tau_max + 1]
    a = seg[:w]
    n_fft = 1 << int(np.ceil(np.log2(len(seg) + w)))
    cross = np.fft.irfft(np.fft.rfft(seg, n_fft) * np.conj(np.fft.rfft(a, n_fft)), n_fft)[:tau_max + 2]
    cs = np.concatenate(([0.0], np.cumsum(seg * seg)))
    e0 = cs[w] - cs[0]
    taus = np.arange(tau_max + 2)
    e_tau = cs[np.minimum(taus + w, len(seg))] - cs[np.minimum(taus, len(seg))]
    d = e0 + e_tau - 2.0 * cross
    d[0] = 0.0
    run = np.cumsum(d[1:])
    dn = np.ones(tau_max + 2)
    dn[1:] = np.where(run > 0, d[1:] * np.arange(1, tau_max + 2) / np.where(run > 0, run, 1.0), 1.0)
    best = 0
    t = tau_min
    while t <= tau_max:
        if dn[t] < 0.1:
            while t + 1 <= tau_max and dn[t + 1] < dn[t]:
                t += 1
            best = t
            break
        t += 1
    if best == 0:
        rng = dn[tau_min:tau_max + 1]
        best = tau_min + int(np.argmin(rng))
        if dn[best] > 0.4:
            return 0.0
    tau_f = float(best)
    if 1 < best <= tau_max:
        a_, b_, c_ = dn[best - 1], dn[best], dn[best + 1]
        den = a_ - 2 * b_ + c_
        if den > 1e-12:
            tau_f += 0.5 * (a_ - c_) / den
    return float(fs / tau_f)


def harmonic_levels_db(x: np.ndarray, fs: float, f0: float, n: int) -> np.ndarray:
    """Levels of harmonics 1..n in dB relative to the strongest, Hann-windowed FFT peak picking
    within +-3 % of h*f0. Raises ValueError for f0 <= 0 or n < 1."""
    if not f0 > 0 or n < 1:
        raise ValueError("f0 must be > 0 and n >= 1")
    x = np.asarray(x, dtype=np.float64)
    m = 1 << int(np.ceil(np.log2(len(x) * 4)))
    mag = np.abs(np.fft.rfft(x * np.hanning(len(x) + 1)[:-1], m))
    bin_hz = fs / m
    amp = np.zeros(n)
    for h in range(1, n + 1):
        fc = f0 * h
        if fc >= fs / 2:
            continue
        lo = max(1, int(np.floor(fc * 0.97 / bin_hz)))
        hi = min(len(mag) - 2, int(np.ceil(fc * 1.03 / bin_hz)))
        pk = lo + int(np.argmax(mag[lo:hi + 1]))
        a = mag[pk]
        if mag[pk - 1] > 0 and mag[pk + 1] > 0 and a > 0:
            l, c, r = np.log(mag[pk - 1]), np.log(a), np.log(mag[pk + 1])
            den = l - 2 * c + r
            if den < -1e-12:
                a = float(np.exp(c - 0.125 * (r - l) ** 2 / den))
        amp[h - 1] = a
    mx = amp.max()
    out = np.full(n, -200.0)
    ok = amp > 0
    if mx > 0:
        out[ok] = 20 * np.log10(amp[ok] / mx)
    return out


def spectral_centroid_hz(x: np.ndarray, fs: float) -> float:
    """Power-spectrum centroid over 20 Hz .. fs/2."""
    x = np.asarray(x, dtype=np.float64)
    m = 1 << int(np.ceil(np.log2(len(x))))
    p = np.abs(np.fft.rfft(x * np.hanning(len(x) + 1)[:-1], m)) ** 2
    f = np.arange(len(p)) * fs / m
    sel = f >= 20.0
    den = p[sel].sum()
    return float((f[sel] * p[sel]).sum() / den) if den > 0 else -1.0


def _envelope(x: np.ndarray, fs: float) -> np.ndarray:
    """10 ms RMS envelope, centred window with edge normalisation."""
    win = max(1, int(round(0.010 * fs)))
    c = np.concatenate(([0.0], np.cumsum(np.asarray(x, dtype=np.float64) ** 2)))
    idx = np.arange(len(x))
    lo = np.clip(idx - win // 2, 0, len(x))
    hi = np.clip(idx - win // 2 + win, 0, len(x))
    return np.sqrt((c[hi] - c[lo]) / np.maximum(hi - lo, 1))


def attack_time_s(x: np.ndarray, fs: float) -> float:
    """Time from 10 % to 90 % of the maximum of the 10 ms RMS envelope."""
    env = _envelope(x, fs)
    mx = env.max()
    if mx <= 0:
        return 0.0
    i10 = int(np.argmax(env >= 0.1 * mx))
    i90 = i10 + int(np.argmax(env[i10:] >= 0.9 * mx))
    return float((i90 - i10) / fs)


def steady_segment(x: np.ndarray, fs: float) -> np.ndarray:
    """The segment where the 10 ms RMS envelope is >= 50 % of its maximum, trimmed by 100 ms at
    both ends; raises ValueError if the trimmed segment is shorter than 200 ms."""
    x = np.asarray(x, dtype=np.float64)
    env = _envelope(x, fs)
    above = np.nonzero(env >= 0.5 * env.max())[0]
    if len(above) == 0:
        raise ValueError("no steady segment")
    trim = int(round(0.1 * fs))
    a, b = above[0] + trim, above[-1] + 1 - trim
    if b - a < int(round(0.2 * fs)):
        raise ValueError("steady segment shorter than 200 ms")
    return x[a:b]


def compare(ref: np.ndarray, syn: np.ndarray, fs: float) -> dict:
    """{"harm_mad_db", "centroid_ratio", "attack_ratio", "f0_ref", "f0_syn"} (D-015)."""
    sr, ss = steady_segment(ref, fs), steady_segment(syn, fs)
    f0_ref, f0_syn = f0_yin(sr, fs), f0_yin(ss, fs)
    if f0_ref <= 0:
        raise ValueError("reference has no measurable f0")
    lr = harmonic_levels_db(sr, fs, f0_ref, 10)
    ls = harmonic_levels_db(ss, fs, f0_ref, 10)
    mad = float(np.mean(np.abs(lr[1:] - ls[1:])))
    cr, cs_ = spectral_centroid_hz(sr, fs), spectral_centroid_hz(ss, fs)
    ar, as_ = attack_time_s(ref, fs), attack_time_s(syn, fs)
    return {
        "harm_mad_db": mad,
        "centroid_ratio": cs_ / cr if cr > 0 else float("nan"),
        "attack_ratio": max(as_, 1e-3) / max(ar, 1e-3),
        "f0_ref": f0_ref,
        "f0_syn": f0_syn,
        "centroid_ref_hz": cr,
        "centroid_syn_hz": cs_,
    }
