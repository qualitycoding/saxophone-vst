# Pre-mortem round 1 (in-context reviewer; substitution for fresh-context Opus agents, A-016)

Prompt: "This plan was executed exactly and failed. Why?"

1. Every frozen test passes but the instrument still sounds synthetic: an 8-mode model with a generic radiation filter lacks the body/bell colour and articulation noises of a real sax. → R-001
2. The "simplified alto" geometry (soprano × 1.5) puts resonances or register-hole effects in the wrong places, so registers break wrongly or timbre is off. → R-002
3. A highlighted fingering is wrong (single text source for the chart). → R-003
4. Realism thresholds (A-012) are arbitrary: could pass a bad sound or fail a good one. → R-004
5. The explicit coupling scheme goes unstable at extreme overblow/hard reed → NaNs or blow-ups. → R-005
6. Multistability makes low notes crack into the second register at ordinary velocities. → R-006
7. pluginval 1.0.4 (2024) fails on JUCE 9 plugins for reasons unrelated to the plugin. → R-007
8. TinySOL download fails or the metadata layout differs from the mirdata description. → R-008
9. The headless editor test crashes on CI without a display. → R-009
10. 4× oversampling + decimation exceeds the CPU budget on slower CI runners. → R-010
11. AGPLv3 obligations surprise the human when distributing binaries. → R-011
12. Pitch depends on the host sample rate because the internal rate differs (176.4 k vs 192 k). → R-012
13. The implementing agent loosens a frozen test to get green. → R-013
14. The PAT used for planning leaks or is reused. → R-014
