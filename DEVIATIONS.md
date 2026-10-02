# Deviations

| Step | Situation | Choice | Rationale |
|---|---|---|---|
| S-007 | D-008/D-009 specify a per-entry `tuning_scale`, but `ResonatorParams` (public header) had no field for it. | Added `double tuningScale = 1.0;` to `ResonatorParams` (additive; no existing signature changed; frozen tests unaffected). | Smallest reversible way to carry the calibrated scale from the table to the voice without baking it into poles (T-026 checks raw poles). |
| S-007 | Plan said "CMake custom command" to embed the table. | Embedding is done at configure time with `file(READ ... HEX)` + `file(CONFIGURE ...)` and `CMAKE_CONFIGURE_DEPENDS`. | Same result, fewer moving parts; a byte array avoids MSVC string-literal limits. |
