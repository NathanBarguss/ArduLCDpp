# Goal
Create an automated test harness/library that can replay the `docs/display_smoke_tests.md` sequences against the firmware so we can run regression suites without manual serial poking each time.

# Background
`docs/display_smoke_tests.md` documents the byte sequences for T1–T8, but every run today is manual (PowerShell/python snippets + eyeballing the panel). Attempts to run `pio test` fail because there are no Unity suites, and more importantly most checks require hardware interaction. A host-driven harness that emits the los-panel bytes, captures simple assertions (e.g., backlight PWM level, snapshot of DDRAM state, etc.), and logs pass/fail would unblock CI and simplify bring-up for future hardware spins.

# Requirements
- Provide a reusable PC-side tool (Python preferred) that can execute each test case in `docs/display_smoke_tests.md` against a selected serial port.
- Log structured results (timestamp, env, pass/fail, notes); optionally capture photos or textual summaries for visual checks.
- Integrate with PlatformIO or a simple CLI so `pio run`/`pio test` can shell out to the harness when hardware is connected.
- Document how to extend the harness when new smoke tests are added.

# Dependencies
- Existing smoke matrix in `docs/display_smoke_tests.md`.
- Access to the prototype hardware (Nano + HD44780) for validation.

# Implementation Plan
1. Build a small Python package/script that parameterizes the serial sequences and expected outcomes per test ID.
2. Add configuration (e.g., `tests/config.json`) for port name, LCD geometry, photo capture options.
3. Wire the harness into a PlatformIO `extra_scripts` target or a standalone CLI invoked by CI/agents.
4. Update `docs/display_smoke_tests.md` with instructions on running the automated suite vs. manual steps.

# Acceptance Criteria
- Running the harness executes all T1–T8 steps and prints a summary with clear pass/fail.
- Failures provide actionable context (e.g., which byte sequence failed, what was observed).
- README or docs include usage instructions; future contributors can run the suite without reading the entire smoke-test doc.

# Validation Notes
- Start with HD44780 hardware; leave hooks/comments for future OLED expansion.
- Consider mocking modes for CI when hardware is absent (e.g., dry-run that only validates serial sequences).
