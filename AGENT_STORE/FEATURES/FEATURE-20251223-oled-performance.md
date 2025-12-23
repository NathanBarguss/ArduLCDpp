# Goal
Prevent visible flicker or stalls when the OLED backend processes frequent LCDproc updates.

# Background
lcd2oled mentions that certain operations (scrolling, rapid refresh) can be expensive. LCDproc often streams incremental updates, so naive redraws might flicker or block the MCU.

# Requirements
- Evaluate lcd2oled's default buffering behavior under typical LCDproc traffic.
- Confirm `clear()` (currently hardcoded to 1024 bytes in upstream lcd2oled) does not overrun 128x32 panels; patch or wrap so only the active page range is cleared.
- If flicker is observed, implement a lightweight batching strategy (e.g., dirty flag + periodic flush, or coalescing writes) to keep updates smooth.
- Ensure any added buffering does not introduce noticeable latency for cursor moves or full clears.

# Dependencies
- OLED backend functioning and geometry enforced.

# Implementation Plan
1. Profile OLED refresh behavior using LCDproc demo screens; capture whether writes cause visible flicker.
2. Depending on findings, either document that lcd2oled buffering is sufficient (with measurements) or implement a deferred flush timer (e.g., flush every 50 ms or after clear/home).
3. Add configuration knobs for flush interval if a custom policy is introduced.
4. Incorporate performance observations into the smoke-test matrix so regressions are caught.

# Acceptance Criteria
- Default LCDproc screens render on OLED without per-character flicker.
- `clear()` leaves the display in a consistent state on both 128x32 and larger panels (no ghost rows, no controller lockups).
- MCU remains responsive; no multi-second stalls observed during heavy updates.
- Any buffering policy is documented and covered by tests/manual verifications.

# Validation Notes
- Capture video or oscilloscope traces if needed to demonstrate smooth updates.
