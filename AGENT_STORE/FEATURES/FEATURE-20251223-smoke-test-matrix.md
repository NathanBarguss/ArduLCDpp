# Goal
Publish a smoke-test matrix and simple demo scripts covering both HD44780 and OLED backends to prevent regressions as new display sizes are added.

# Background
With multiple display targets, manual spot checks are easy to forget. A repeatable checklist (even if manual) ensures parity for cursor bounds, custom glyphs, backlight, and USB serial stability.

# Requirements
- Create a test plan document (e.g., `/docs/display_smoke_tests.md`) enumerating cases: clear/home/setCursor bounds, full-screen fills, custom chars 0-7, backlight/brightness toggles, and USB serial connect/disconnect behavior.
- Provide simple Arduino/PlatformIO demo scripts or instructions for executing each test on both backends.
- Ensure the plan is referenced in PR checklists or AGENT_STORE items so it is exercised before merging.

# Dependencies
- Display interface and both backends implemented.
- Documentation feature (wiring/build) to point to when running tests.

# Implementation Plan
1. Draft the smoke-test doc with a matrix (rows = tests, columns = HD44780/OLED) and expected outcomes.
2. Add or reuse example sketches that drive the scenarios (bars, cursor sweeps, brightness toggles) and document how to run them.
3. Link the matrix in CONTRIBUTING or README to keep it visible for reviewers.

# Acceptance Criteria
- Test plan file exists in `/docs/` and covers all listed scenarios for both backends.
- Reviewers can reference the matrix to confirm a PR ran the appropriate checks.
- Demo scripts or instructions are runnable without extra hardware assumptions beyond the documented wiring.

# Validation Notes
- Run through the checklist once on each backend and note any timing issues or missing instrumentation.
