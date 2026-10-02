# Environment (pinned; setup verified in the planning sandbox on 2026-10-01)

## Toolchain
| Component | Version / pin | Notes |
|---|---|---|
| OS (verified) | Ubuntu 24.04 (x86-64) | CI also: macos-15, windows-2025 (A-018) |
| C++ compiler | GCC 13.3.0 (Ubuntu), Apple Clang (Xcode default on macos-15), MSVC 2022 (windows-2025) | C++20 |
| CMake | 4.3.4 (`pip install cmake==4.3.4` verified); any ≥ 3.22 accepted | |
| Ninja | 1.13.x (`pip install ninja`) | Windows: VS generator acceptable |
| JUCE | 9.0.3 @ `be29c81492b6151c8ea8d14c840e1311963b3a83` | FetchContent |
| Catch2 | v3.16.0 @ `317ac1ed4c0bb6e6b91eafc817e05c488feffcb3` | FetchContent, tests only |
| nlohmann/json | v3.12.0 @ `55f93686c01528224f448c19128836e7df245f72` | FetchContent, core (private) |
| pluginval | v1.0.4, zip SHA-256s in `tests/scripts/run_pluginval.sh` | CI/test only |
| Python | 3.12.3 | tools + Python tests |
| Python packages | `tools/requirements.txt` (numpy 2.5.3, scipy 1.18.1, soundfile 0.14.0, pytest 9.1.1, pip-audit 2.10.1); full lock `tools/requirements.lock` | |

## Linux system packages (verified install)
```bash
sudo apt-get update
sudo apt-get install -y g++ libasound2-dev libjack-jackd2-dev ladspa-sdk libfreetype-dev \
  libfontconfig1-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
  libxrandr-dev libxrender-dev libxi-dev libcurl4-openssl-dev xvfb
```
(`JUCE_WEB_BROWSER=0` and `JUCE_USE_CURL=0` are set, so webkit and curl are not needed at link time; the
curl dev package is harmless.) Planning-sandbox note: an unrelated broken apt source (nodesource) had to be
disabled first; `libxi-dev` was initially missing and is required (`X11/extensions/XInput2.h`).

## Python tools (verified)
```bash
python3 -m venv .venv && . .venv/bin/activate
pip install -r tools/requirements.txt
```

## Build commands (verified for core + tests; plugin build: see "Verification log")
```bash
pip install cmake==4.3.4 ninja           # if no system CMake >= 3.22
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release            # plugin ON by default
cmake --build build -j
ctest --test-dir build -LE perf --output-on-failure
ctest --test-dir build -L perf --output-on-failure                 # Release only (T-016)
python -m pytest tests/python -k "not tinysol"
bash tests/scripts/verify_freeze.sh
```
Core-only (fast) configure: add `-DSAX_BUILD_PLUGIN=OFF`.

## Verification log (planning sandbox, 1 vCPU, 3 GB RAM)
- Debug core + tests configure/build: OK (141 targets). Red run: unit 17/17 cases fail, integration 14/14,
  operational 4/4, alloc 1/1 fail; perf skipped in Debug (fails at construction in Release).
- Python venv install from pins: OK; red run 21 fail / 2 pass (the two D-018 guard tests).
- Release build with plugin (JUCE 9.0.3, LTO): OK at `-j1` (an earlier `-j2` run died, likely memory). Artefacts: `build/plugin/SaxophoneVST_artefacts/Release/VST3/Saxophone.vst3`, `.../Standalone/Saxophone`.
- Plugin tests under `xvfb-run -a`: 3 cases, 2 fail (red), 1 guard passes (D-018).
- pluginval 1.0.4 strictness 10 on the stub VST3 under `xvfb-run -a`: SUCCESS; without a display it segfaults. Details: `research/spikes/plugin_build.log`.
- Release `sax_perf_tests`: fails (red) at voice construction.
- Low-memory machines: build with `-j1`; LTO link of the plugin takes several minutes.
