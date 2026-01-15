### Goal
Define and enforce "LOS-PANEL behavior is sacred" compatibility gates so new multi-protocol work cannot regress existing LCDproc users.

### Background
ArduLCDpp's most important constraint is preserving existing, working LOS-PANEL behavior. As the firmware gains protocol auto-detection and a second parser, we need explicit compatibility gates (tests + review checklist) that are run continuously during development.

### Requirements
- Maintain bit-for-bit LOS-PANEL behavior for:
  - `0xFE <byte>` raw HD44780 command pass-through semantics.
  - `0xFD <byte>` backlight brightness mapping.
  - ASCII streaming behavior (including burst-safe refresh timing behavior already landed).
- Establish a "compatibility gate" definition:
  - A minimum set of automated checks that must pass before merging multi-protocol changes.
  - A manual smoke fallback checklist for bench verification when automation isn't available.
- Clearly document the "do not change" surface area and what constitutes an intentional behavior change.

### Dependencies
- Existing smoke tests in `docs/display_smoke_tests.md` and supporting scripts (e.g., `scripts/t4_with_logs.py`).
- The automated harness work in `FEATURE-20260102-automated-test-harness` (for repeatability).

### Implementation Plan
1. Define a LOS-PANEL contract test list (clear, home, cursor, CGRAM slots 0-7, backlight levels, burst scenarios).
2. Add/extend Python scripts to replay those sequences consistently and record results.
3. Add a short "compatibility checklist" section to the multi-protocol FEATURE specs:
   - What was revalidated?
   - What fixtures were added/updated?
4. Ensure the default/timeout behavior of protocol detection always prefers LOS-PANEL when ambiguous.

### Acceptance Criteria
- There is an explicit, written compatibility gate that multi-protocol work must satisfy.
- The gate can be executed repeatedly (scripted preferred) and produces an unambiguous pass/fail.
- After Matrix Orbital features land, a LOS-PANEL-only user sees no changes in behavior, performance, or output.

### Validation Notes
- Validate on both Nano328 and Nano168, since timing/memory constraints can expose subtle LOS-PANEL regressions.
