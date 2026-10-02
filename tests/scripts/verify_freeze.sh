#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# T-028: fails if any frozen artifact changed. Run from the repository root.
set -euo pipefail
sha256sum --check --strict --quiet tests/FROZEN_MANIFEST.sha256
echo "freeze OK"
