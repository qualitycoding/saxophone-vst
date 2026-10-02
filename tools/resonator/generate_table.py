# SPDX-License-Identifier: Apache-2.0
"""CLI: python -m tools.resonator.generate_table --out data/alto_resonators.json  (D-009).

Deterministic: fixed frequency grid, no randomness, sorted JSON keys, floats rounded to 6 significant digits.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

import numpy as np

from tools.resonator import tmm

N_MODES = 8
F_MIN, F_MAX, F_STEP = 20.0, 9000.0, 0.25
BAND_FACTOR = 1.1          # fit up to 1.1 x the 8th impedance peak
# Post-fit calibration of the simplified TMM (logged in DEVIATIONS.md):
VENT_MODE1_SCALE = 0.1     # an open register vent suppresses the first impedance peak far more than the TMM predicts
F2_F1_MIN = 2.07           # measured alto f2/f1 (Colinot 2021: 2.078); the TMM gives ~2.01 for the lowest notes

# Empirical corrections applied after the fit (logged in DEVIATIONS.md). The simplified TMM
#  (a) under-suppresses the first resonance when the register vent is open (|Z1|/|Z2| 0.3..1.4, whereas the
#      octave key on a real alto leaves the first peak well below the second), and
#  (b) over-estimates peak2/peak1 for the lowest unvented notes (1.8 vs 1.33 in Colinot's measured D# data),
#      which makes the lowest notes jump to the second register.
VENT_MODE1_FACTOR = 0.15
UNVENTED_PEAK_RATIO_CAP = 1.15


def hole_for(written: int) -> str:
    """Register vent used by the standard fingering (D-004): none < 74, body 74..80, neck >= 81."""
    return "none" if written < 74 else ("body" if written <= 80 else "neck")


def _r6(x: float) -> float:
    return float(f"{x:.6g}")


def _peaks(m: np.ndarray) -> list[int]:
    return [i for i in range(1, len(m) - 1) if m[i] > m[i - 1] and m[i] >= m[i + 1]]


def _height(mode: dict) -> float:
    return float(np.hypot(mode["re_c"], mode["im_c"]) / abs(mode["re_s"]))


def _shape(entry: dict) -> None:
    m = entry["modes"]
    if entry["hole"] != "none":
        k = VENT_MODE1_FACTOR
    else:
        k = max(1.0, _height(m[1]) / _height(m[0]) / UNVENTED_PEAK_RATIO_CAP)
    m[0]["re_c"], m[0]["im_c"] = _r6(m[0]["re_c"] * k), _r6(m[0]["im_c"] * k)


def entry_for(written: int) -> dict:
    hole = hole_for(written)
    g = tmm.alto_geometry(written, hole)
    f = np.arange(F_MIN, F_MAX, F_STEP)
    z = tmm.input_impedance(g, f)
    pk = _peaks(np.abs(z))
    if len(pk) < N_MODES:
        raise RuntimeError(f"written {written}: only {len(pk)} impedance peaks below {F_MAX} Hz")
    sel = f <= BAND_FACTOR * f[pk[N_MODES - 1]]
    s, c = tmm.modal_fit(f[sel], z[sel], N_MODES)
    entry = {
        "written": written,
        "hole": hole,
        "tuning_scale": 1.0,
        "modes": [{"re_s": _r6(p.real), "im_s": _r6(p.imag), "re_c": _r6(r.real), "im_c": _r6(r.imag)}
                  for p, r in zip(s, c)],
    }
    _shape(entry)
    return entry


def build() -> dict:
    try:
        sha = subprocess.run(["git", "rev-parse", "HEAD"], capture_output=True, text=True, check=True).stdout.strip()
    except Exception:
        sha = "unknown"
    return {
        "schema": "saxophone-vst/resonators@1",
        "instrument": "alto",
        "units": {"pole": "rad/s", "residue": "Z/Zc"},
        "provenance": {"generator": "tools/resonator/generate_table.py", "git": sha, "geometry": "D-009"},
        "entries": [entry_for(w) for w in range(58, 91)],
    }


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    args = ap.parse_args(argv)
    text = json.dumps(build(), indent=1, sort_keys=True) + "\n"
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    Path(args.out).write_text(text)
    print(f"wrote {args.out} ({len(text)} bytes)", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
