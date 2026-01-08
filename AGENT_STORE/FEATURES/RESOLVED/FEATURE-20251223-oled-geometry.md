# Goal
Make the OLED backend present as a strict `LCDW` x `LCDH` character display so LCDproc configurations remain unchanged.

# Background
`lcd2oled` lets us define the logical rows/columns via `begin(width, height)`, even though the underlying panel is 128x32 pixels.

This firmware targets parity with the HD44780 builds using the shared geometry constants:
- `LCDW = 20`
- `LCDH = 4`

The SSD1306 128x32 panel can technically fit 21 columns at 5x7+spacing, but we intentionally clamp to 20 columns so LCD and OLED layouts match.

# Requirements
- Initialize the OLED backend with `begin(LCDW, LCDH)` and clamp all cursor positions to `0..(LCDW-1)` by `0..(LCDH-1)`.
- Update protocol parsing so commands targeting rows/columns outside the `LCDW` x `LCDH` window are safely clipped or rejected.
- Document the physical page mapping (4 SSD1306 pages -> 4 character rows) so future work on taller panels stays consistent.

# Dependencies
- Display interface abstraction and OLED backend in place.
- Command translator covers DDRAM/CGRAM addressing.

# Implementation Plan
1. Use `LCDW/LCDH` for OLED `begin()`.
2. Clamp `OLEDDisplay::setCursor()` to the active geometry.
3. Ensure multi-row DDRAM streams can advance rows reliably (no “stuck on row 0”).

# Acceptance Criteria
- LCDproc configured for Width `LCDW` / Height `LCDH` renders correctly on the OLED with no wrapping or ghost rows.
- Cursor writes beyond the `LCDW` x `LCDH` bounds are ignored or clipped without crashing.
- Manual smoke test shows all four rows render where expected with no unused space.

# Validation Notes
- Use the smoke tests in `docs/display_smoke_tests.md` to validate addressing and burst behavior.

## Verification (2026-01-08, QA)
- Bench: Nano ATmega168 dual build on `COM6` (`nano168_dual_serial`)
- PASS T3: DDRAM sweep fills all 4 rows correctly on both LCD and OLED (20x4 parity).
- PASS T4: unpaced full-screen fill ends with 4 full rows (including when preceded by `FC 10 <mode>`).

