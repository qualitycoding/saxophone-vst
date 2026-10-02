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


def hole_for(written: int) -> str:
    """Register vent used by the standard fingering (D-004): none < 74, body 74..80, neck >= 81."""
    return "none" if written < 74 else ("body" if written <= 80 else "neck")


def _r6(x: float) -> float:
    return float(f"{x:.6g}")


def _peaks(m: np.ndarray) -> list[int]:
    return [i for i in range(1, len(m) - 1) if m[i] > m[i - 1] and m[i] >= m[i + 1]]


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
    return {
        "written": written,
        "hole": hole,
        "tuning_scale": 1.0,
        "modes": [{"re_s": _r6(p.real), "im_s": _r6(p.imag), "re_c": _r6(r.real), "im_c": _r6(r.imag)}
                  for p, r in zip(s, c)],
    }


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
