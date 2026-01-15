### Goal
Add a protocol-focused regression suite that validates LOS-PANEL stability, Matrix Orbital MVP behavior, and protocol auto-detection without relying on manual bench runs.

### Background
ArduLCDpp's core constraint is "never break existing functionality." The fastest way to protect that constraint while adding a second protocol is to make the host-side test harness the source of truth for protocol behavior, cursor math, CGRAM parity, and brightness mapping.

### Requirements
- Provide repeatable Python tests that can run locally (and later in CI) against a connected device.
- Test coverage includes:
  - LOS-PANEL: clear, cursor, CGRAM, backlight.
  - Matrix Orbital: clear, cursor, text, CGRAM, backlight. (Buttons optional in Phase 2.)
  - Protocol detection: correct selection given representative "first bytes" for each host.
- Every new Matrix Orbital capability includes:
  - At least one protocol-specific test.
  - At least one "LOS-PANEL still works" test that runs in the same suite.
- Tests can run in two modes:
  - "Instrumented firmware" (optional) that reports display state over serial for assertions.
  - "Manual/visual" fallback that logs expected state + captures raw RX/TX bytes for review.

### Dependencies
- `FEATURE-20260102-automated-test-harness` (existing harness foundation).
- `FEATURE-20260108-protocol-autodetection` and `FEATURE-20260108-matrix-orbital-mvp`.
- A stable test naming/layout convention under `tests/` (recommended for host-driven tests; keep PlatformIO's `test/` for Unity suites).

### Implementation Plan
1. Define the test folder structure for multi-protocol coverage (LOS, MO, detection).
2. Add fixtures:
   - "First 16 bytes" traces for LCDproc and LCD Smartie.
   - Known-good sequences for clear/cursor/cgram/backlight per protocol.
3. Implement device helpers:
   - Open port, wait after DTR reset, send sequences, optionally read debug responses.
4. Add an explicit regression policy to docs (what must be added per feature, what must pass).

### Acceptance Criteria
- A single command runs the suite and reports pass/fail for:
  - LOS-PANEL baseline.
  - Matrix Orbital MVP.
  - Detection heuristics.
- Adding a Matrix Orbital behavior change requires adding/updating a test fixture.
- The suite is structured so future CI enablement is straightforward (no hardcoded COM ports).

### Validation Notes
- Run the suite against:
  - Nano328 environment(s).
  - Nano168 environment(s) (memory and timing edge cases).
