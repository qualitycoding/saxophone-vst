# Active profiles

| Profile | Active | Justification |
|---|---|---|
| `software` | **yes** | Deliverable is a VST3/AU/Standalone plugin plus offline tools. |
| `math` | no | No mathematical statements are produced; published acoustics results are consumed as cited claims. |
| `computational` | no | Numerical results are not the deliverable; numerical behaviour is verified as software tests. |
| `publication` | no | No manuscript, preprint or archive deposit. |

Mode flags: `software.deploys = false` (no release, no package publication, no running service; CI builds artifacts only). `math.exploration` n/a.

## Not-applicable sections (produce no artifacts, impose no checks)
- Rule 7 (evidence classes for deliverables), Rule 8 (result-agnostic planning) — math/computational inactive (Rule 8 spirit still applied to realism calibration via D-015 decision rules).
- G-001 (headline-result gate) — math/computational inactive.
- 0.3.3, 0.3.4, 0.3.5 intake blocks; R2b novelty search; 2A (all); 2B.2 provenance contract; 2B.4 unknown-outcome tests; 2C figures; 2D manuscript.
- `plan/OPERATIONS.md` (deploys = false); deployment tests; operational forks for deployment.
- Layout entries: `research/NOVELTY.md`, `math/**`, `figures/SPEC.md`, `manuscript/**`.

## Applicable artifacts
HANDOFF.md, plan/{PROFILE,PLAN,ASSUMPTIONS,DECISIONS,GATES,ENVIRONMENT,TRACEABILITY}.md,
research/{QUESTIONS.md,claims.json,SOURCES.md,rounds/,spikes/}, tests/, tests/FROZEN_MANIFEST.sha256,
premortem/{round-N.md,RISK_REGISTER.md}, .checkpoints/state.json.
