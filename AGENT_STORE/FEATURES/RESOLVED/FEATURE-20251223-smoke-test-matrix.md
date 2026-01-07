# Goal
Publish a smoke-test matrix and simple demo scripts covering the HD44780 build, the OLED build, and the dual-display parity mode to prevent regressions as new display sizes are added.

# Background
With multiple display targets (and now a mode that drives both simultaneously), manual spot checks are easy to forget. A repeatable checklist (even if manual) ensures parity for cursor bounds, custom glyphs, backlight, and USB serial stability - plus a dedicated column that calls out when the LCD and OLED disagree while running the dual firmware.

# Requirements
- Create a test plan document (e.g., `/docs/display_smoke_tests.md`) enumerating cases: clear/home/setCursor bounds, full-screen fills, custom chars 0-7, backlight/brightness toggles, and USB serial connect/disconnect behavior.
- Provide simple Arduino/PlatformIO demo scripts or instructions for executing each test on the LCD-only build, the OLED-only build, and the dual mode (noting that dual cells must confirm parity between panels).
- Ensure the plan is referenced in PR checklists or AGENT_STORE items so it is exercised before merging.

# Dependencies
- Display interface, both backends, and the dual-mode feature implemented.
- Documentation feature (wiring/build) to point to when running tests.

# Implementation Plan
1. Draft the smoke-test doc with a matrix (rows = tests, columns = HD44780/OLED/Dual) and expected outcomes, explicitly annotating parity expectations in the dual column.
2. Add or reuse example sketches that drive the scenarios (bars, cursor sweeps, brightness toggles) and document how to run them across all modes.
3. Link the matrix in CONTRIBUTING or README to keep it visible for reviewers.

# Acceptance Criteria
- Test plan file exists in `/docs/` and covers all listed scenarios for LCD-only, OLED-only, and dual builds.
- Reviewers can reference the matrix to confirm a PR ran the appropriate checks, including parity verification when the dual mode is used.
- Demo scripts or instructions are runnable without extra hardware assumptions beyond the documented wiring.

# Validation Notes
- Run through the checklist once on each build mode and note any timing issues or missing instrumentation.
- 2026-01-07: Updated `docs/display_smoke_tests.md` to include expected outcomes for OLED + Dual (including burst-safe "lag then catch up" behavior in `nano168_dual_serial`) and a pointer to `scripts/t4_with_logs.py` for repeatable T4/T8 burst runs.
