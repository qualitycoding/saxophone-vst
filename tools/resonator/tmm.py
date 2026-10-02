# SPDX-License-Identifier: Apache-2.0
"""Transfer-matrix model of a simplified saxophone and modal extraction (D-009).

Geometry: cylindrical mouthpiece (L_cyl = R1 / (3 tan phi), the volume-equivalent of the missing apex
of the cone) + truncated cone, optional register side hole at distance L1 from the cone input
(Szwarcberg et al. 2025, Fig. 1). Visco-thermal losses by Zwikker-Kosten, unflanged radiation by the
Silva et al. (2009) approximation. Impedances are dimensionless: Z / Zc1, Zc1 = rho c / (pi R1^2).
Time convention exp(+j w t); waves exp(-j k x).
"""
from __future__ import annotations

from dataclasses import dataclass, replace

import numpy as np
from scipy.special import jve

C0 = 343.37      # m/s at 20 degC
RHO = 1.2041     # kg/m^3 at 20 degC
ETA = 1.8205e-5  # dynamic viscosity, Pa s
PR = 0.71        # Prandtl number
GAMMA = 1.4


@dataclass(frozen=True)
class Geometry:
    r1: float                 # cone input radius, m
    half_angle_deg: float     # cone half angle
    length: float             # cone length L, m
    hole_l1: float | None = None   # register hole distance from cone input, m (None = no hole)
    hole_radius: float = 0.0
    hole_chimney: float = 0.0
    hole_open: bool = False
    lossless: bool = False
    radiation: bool = True


# --------------------------------------------------------------------------- fluid / losses
def _bessel_ratio_f(x: np.ndarray) -> np.ndarray:
    """F(x) = 2 J1(x) / (x J0(x)) for complex x, evaluated with exponentially scaled Bessels."""
    return 2.0 * jve(1, x) / (x * jve(0, x))


def _fluid(radius: float, w: np.ndarray, lossless: bool) -> tuple[np.ndarray, np.ndarray]:
    """Complex wavenumber and effective density of a tube of the given radius (Zwikker-Kosten)."""
    if lossless:
        return w / C0, np.full_like(w, RHO, dtype=complex)
    xv = radius * np.sqrt(-1j * w * RHO / ETA)
    fv = _bessel_ratio_f(xv)
    ft = _bessel_ratio_f(xv * np.sqrt(PR))
    rho_c = RHO / (1.0 - fv)
    kc = (w / C0) * np.sqrt((1.0 + (GAMMA - 1.0) * ft) / (1.0 - fv))
    return kc, rho_c


def _radiation_unflanged(k: np.ndarray, a: float) -> np.ndarray:
    """Dimensionless radiation impedance Z_R / Zc of an unflanged pipe (Silva et al. 2009)."""
    ka = k * a
    mod_r = (1 + 0.2 * ka - 0.084 * ka**2) / (1 + 0.2 * ka + (0.5 - 0.084) * ka**2)
    ell = a * (0.6133 + 0.027 * ka**2) / (1 + 0.19 * ka**2)
    r = -mod_r * np.exp(-2j * k * ell)
    return (1 + r) / (1 - r)


# --------------------------------------------------------------------------- 2x2 helpers
def _mm(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    return np.matmul(a, b)


def _inv(m: np.ndarray) -> np.ndarray:
    a, b, c, d = m[:, 0, 0], m[:, 0, 1], m[:, 1, 0], m[:, 1, 1]
    det = a * d - b * c
    out = np.empty_like(m)
    out[:, 0, 0], out[:, 0, 1], out[:, 1, 0], out[:, 1, 1] = d / det, -b / det, -c / det, a / det
    return out


def _cyl_tm(radius: float, length: float, w: np.ndarray, lossless: bool) -> np.ndarray:
    kc, rho_c = _fluid(radius, w, lossless)
    s = np.pi * radius**2
    zc = w * rho_c / (s * kc)
    kl = kc * length
    tm = np.empty((len(w), 2, 2), dtype=complex)
    tm[:, 0, 0] = np.cos(kl)
    tm[:, 0, 1] = 1j * zc * np.sin(kl)
    tm[:, 1, 0] = 1j * np.sin(kl) / zc
    tm[:, 1, 1] = np.cos(kl)
    return tm


def _cone_basis(x: float, w: np.ndarray, kc: np.ndarray, rho_c: np.ndarray, tan_phi: float) -> np.ndarray:
    """Matrix mapping spherical-wave amplitudes (a, b) to (P, U) at apex distance x."""
    s = np.pi * (x * tan_phi) ** 2
    ea, eb = np.exp(-1j * kc * x), np.exp(1j * kc * x)
    m = np.empty((len(w), 2, 2), dtype=complex)
    m[:, 0, 0] = ea / x
    m[:, 0, 1] = eb / x
    pre = -s / (1j * w * rho_c)
    m[:, 1, 0] = pre * ea * (-1j * kc / x - 1.0 / x**2)
    m[:, 1, 1] = pre * eb * (1j * kc / x - 1.0 / x**2)
    return m


def _cone_tm(x_in: float, x_out: float, w: np.ndarray, tan_phi: float, lossless: bool) -> np.ndarray:
    r_mean = 0.5 * (x_in + x_out) * tan_phi
    kc, rho_c = _fluid(r_mean, w, lossless)
    return _mm(_cone_basis(x_in, w, kc, rho_c, tan_phi), _inv(_cone_basis(x_out, w, kc, rho_c, tan_phi)))


def _shunt_tm(z_h: np.ndarray) -> np.ndarray:
    tm = np.zeros((len(z_h), 2, 2), dtype=complex)
    tm[:, 0, 0] = 1.0
    tm[:, 1, 1] = 1.0
    tm[:, 1, 0] = 1.0 / z_h
    return tm


def _hole_impedance(g: Geometry, w: np.ndarray) -> np.ndarray:
    a, t = g.hole_radius, g.hole_chimney
    kc, rho_c = _fluid(a, w, g.lossless)
    zch = w * rho_c / (np.pi * a**2 * kc)
    kt = kc * t
    if g.hole_open:
        ka = (w / C0) * a
        z_r = ka**2 / 2.0 + 1j * 0.8216 * ka          # flanged, low ka
        tan_kt = np.tan(kt)
        return zch * (z_r + 1j * tan_kt) / (1.0 + 1j * z_r * tan_kt)
    return zch * (-1j / np.tan(kt))


# --------------------------------------------------------------------------- public API
def input_impedance(g: Geometry, f: np.ndarray) -> np.ndarray:
    """Dimensionless complex input impedance at the mouthpiece plane for frequencies f (Hz)."""
    f = np.asarray(f, dtype=float)
    w = 2.0 * np.pi * f
    tan_phi = np.tan(np.radians(g.half_angle_deg))
    x1 = g.r1 / tan_phi
    x2 = x1 + g.length
    l_cyl = g.r1 / (3.0 * tan_phi)
    zc1 = RHO * C0 / (np.pi * g.r1**2)

    # load at the open end
    if g.radiation:
        r_end = x2 * tan_phi
        z_load = (RHO * C0 / (np.pi * r_end**2)) * _radiation_unflanged(w / C0, r_end)
    else:
        z_load = np.zeros_like(w, dtype=complex)
    state = np.stack([z_load, np.ones_like(z_load)], axis=1)[:, :, None]   # (P, U) column

    # propagate backwards: cone (with optional hole), then mouthpiece cylinder
    if g.hole_l1 is not None and 0.0 < g.hole_l1 < g.length:
        xh = x1 + g.hole_l1
        state = _mm(_cone_tm(xh, x2, w, tan_phi, g.lossless), state)
        state = _mm(_shunt_tm(_hole_impedance(g, w)), state)
        state = _mm(_cone_tm(x1, xh, w, tan_phi, g.lossless), state)
    else:
        state = _mm(_cone_tm(x1, x2, w, tan_phi, g.lossless), state)
    state = _mm(_cyl_tm(g.r1, l_cyl, w, g.lossless), state)
    return (state[:, 0, 0] / state[:, 1, 0]) / zc1


def cylinder_only_impedance(radius: float, length: float, f: np.ndarray, lossless: bool, radiation: bool) -> np.ndarray:
    """Closed-open cylinder (used for analytic verification). Z / Zc with Zc = rho c / (pi r^2)."""
    f = np.asarray(f, dtype=float)
    w = 2.0 * np.pi * f
    zc0 = RHO * C0 / (np.pi * radius**2)
    z_load = zc0 * _radiation_unflanged(w / C0, radius) if radiation else np.zeros_like(w, dtype=complex)
    state = np.stack([z_load, np.ones_like(z_load)], axis=1)[:, :, None]
    state = _mm(_cyl_tm(radius, length, w, lossless), state)
    return (state[:, 0, 0] / state[:, 1, 0]) / zc0


def modal_impedance(poles: np.ndarray, residues: np.ndarray, f: np.ndarray) -> np.ndarray:
    """Evaluate the modal sum of eq. 11: sum C/(jw - s) + conj(C)/(jw - conj(s))."""
    jw = 1j * 2.0 * np.pi * np.asarray(f, dtype=float)[:, None]
    s = np.asarray(poles, dtype=complex)[None, :]
    c = np.asarray(residues, dtype=complex)[None, :]
    return np.sum(c / (jw - s) + np.conj(c) / (jw - np.conj(s)), axis=1)


def modal_fit(f: np.ndarray, z: np.ndarray, n_modes: int) -> tuple[np.ndarray, np.ndarray]:
    """Poles s_n (rad/s, Re<0, sorted by Im) and residues C_n such that
    Z(w) ~= sum C_n/(jw - s_n) + conj(C_n)/(jw - conj(s_n)) (Colinot 2021 eq. 11).

    Vector fitting (Gustavsen & Semlyen 1999) on Hermitian-symmetric data with 2*n_modes complex poles
    (n_modes conjugate pairs). Raises ValueError if the fit does not yield n_modes stable pole pairs.
    """
    f = np.asarray(f, dtype=float)
    z = np.asarray(z, dtype=complex)
    w = 2.0 * np.pi * f
    sk = np.concatenate([-1j * w[::-1], 1j * w])          # two-sided: Z(-jw) = conj Z(jw)
    zk = np.concatenate([np.conj(z[::-1]), z])
    scale = float(np.max(np.abs(zk)))
    zk = zk / scale
    # starting poles: lightly damped conjugate pairs spread over the band
    w_lo, w_hi = w[0], w[-1]
    im0 = np.linspace(w_lo + 0.05 * (w_hi - w_lo), w_hi - 0.05 * (w_hi - w_lo), n_modes)
    poles = np.concatenate([-0.01 * im0 + 1j * im0, -0.01 * im0 - 1j * im0])
    n = len(poles)
    for _ in range(40):
        a = 1.0 / (sk[:, None] - poles[None, :])
        # unknowns: r (n), rt (n):  sum r/(s-p) - zk * sum rt/(s-p) = zk   (pure modal model: no constant term)
        mat = np.concatenate([a, -zk[:, None] * a], axis=1)
        col_scale = np.linalg.norm(mat, axis=0)
        sol = np.linalg.lstsq(mat / col_scale, zk, rcond=None)[0] / col_scale
        rt = sol[n:]
        new = np.linalg.eigvals(np.diag(poles) - np.ones((n, 1)) @ rt[None, :])
        new = np.where(new.real > 0, -new.real + 1j * new.imag, new)
        # enforce conjugate symmetry of the pole set
        up = new[new.imag > 0]
        poles = np.concatenate([up, np.conj(up)]) if len(up) == n_modes else new
        if len(up) != n_modes:
            break
    a = 1.0 / (sk[:, None] - poles[None, :])
    sol = np.linalg.lstsq(a, zk, rcond=None)[0]
    res = sol * scale
    up_idx = np.where(poles.imag > 0)[0]
    if len(up_idx) != n_modes or np.any(poles.real >= 0):
        raise ValueError("modal_fit: could not find %d stable conjugate pole pairs" % n_modes)
    order = up_idx[np.argsort(poles[up_idx].imag)]
    return poles[order], res[order]


def first_peak_hz(g: Geometry, f_guess: float, span: float = 0.2, step: float = 0.05) -> float:
    """Frequency of the maximum of |Z| within +-span of f_guess (parabolic refinement)."""
    f = np.arange(f_guess * (1 - span), f_guess * (1 + span), step)
    m = np.abs(input_impedance(g, f))
    i = int(np.argmax(m))
    if 0 < i < len(f) - 1:
        a, b, c = np.log(m[i - 1]), np.log(m[i]), np.log(m[i + 1])
        den = a - 2 * b + c
        if den < 0:
            return float(f[i] + 0.5 * step * (a - c) / den)
    return float(f[i])


# soprano parameters of Szwarcberg et al. 2025 Table 1 scaled x1.5 for the alto (D-009, C-016, C-017)
_ALTO = dict(r1=0.0069, half_angle_deg=1.74)
_HOLES = {"body": (0.195, 0.0021, 0.006), "neck": (0.060, 0.001275, 0.006)}
TUNING_OFFSET_CENTS = 25.0   # playing frequency sits below f1 (C-013)


def alto_geometry(written_midi: int, hole: str) -> Geometry:
    """Simplified alto: cone length solved so that the closed-hole first resonance equals the D-008
    target for the note. For hole != 'none' the geometry is that of (written - 12) with the register
    hole open (D-009)."""
    if hole not in ("none", "body", "neck"):
        raise ValueError("hole must be 'none', 'body' or 'neck'")
    if not 58 <= written_midi <= 90:
        raise ValueError("written note outside 58..90")
    note = written_midi if hole == "none" else written_midi - 12
    target = 440.0 * 2.0 ** ((note - 9 - 69) / 12.0) * 2.0 ** (TUNING_OFFSET_CENTS / 1200.0)
    tan_phi = np.tan(np.radians(_ALTO["half_angle_deg"]))
    x1 = _ALTO["r1"] / tan_phi
    lo, hi = 0.2, 1.8
    for _ in range(60):
        mid = 0.5 * (lo + hi)
        g = Geometry(length=mid, **_ALTO)
        f_guess = C0 / (2.0 * (x1 + mid)) * 0.95
        if first_peak_hz(g, f_guess, span=0.25) > target:
            lo = mid
        else:
            hi = mid
        if hi - lo < 1e-6:
            break
    g = Geometry(length=0.5 * (lo + hi), **_ALTO)
    if hole != "none":
        l1, rh, lh = _HOLES[hole]
        g = replace(g, hole_l1=l1, hole_radius=rh, hole_chimney=lh, hole_open=True)
    return g
