#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Regenerate data/alto_resonators.json: TMM + modal fit (Python), then per-note tuning calibration (C++).
# Usage: tools/build_table.sh [build-dir-with-sax_calibrate]   (default: build-core)
set -euo pipefail
cd "$(dirname "$0")/.."
BUILD="${1:-build-core}"
python -m tools.resonator.generate_table --out data/alto_resonators.json
cmake --build "$BUILD" --target sax_calibrate
"$BUILD/tools/render/sax_calibrate" --in data/alto_resonators.json --out data/alto_resonators.json
