# Goal
Make the OLED backend present as a strict 16x4 character display so LCDproc configurations remain unchanged.

# Background
lcd2oled lets us define the logical rows/columns via `begin(width, height)`, even though the underlying panel is 128x32 pixels. Mapping the full 32-pixel height to four character rows (8 pixels each) exactly matches the intended 16x4 HD44780 footprint used by los-panel clients.

# Requirements
- Initialize the OLED backend with `begin(16, 4)` and clamp all cursor positions to 0..15 by 0..3.
- Update protocol parsing so commands targeting rows/columns outside the 16x4 window are safely clipped or rejected.
- Document the physical page mapping (4 SSD1306 pages -> 4 character rows) so future work on taller panels stays consistent.

# Dependencies
- Display interface abstraction and OLED backend in place.
- Baseline research documenting current geometry constants (LCDW/LCDH) for comparison.

# Implementation Plan
1. Define shared geometry constants for both backends; set OLED constants to 16x4.
2. Apply range checks either in the protocol layer or within `OLEDDisplay::setCursor` to prevent out-of-bounds writes.
3. Add comments/docs clarifying the physical-to-logical row mapping to avoid future regressions.

# Acceptance Criteria
- LCDproc configured for Width 16 / Height 4 renders correctly on the OLED with no wrapping or ghost rows.
- Cursor writes beyond the 16x4 bounds are ignored or clipped without crashing.
- Manual smoke test shows all four rows render where expected with no unused space.

# Validation Notes
- Use LCDproc demo screens that exercise writes to all rows and columns, watching for wraparound.
- Capture photos/screenshots proving all four logical rows render correctly.

## Notes (2026-01-08)
- QA validation (Dual, COM6):
  - T3 cursor sweep filled all 4 rows correctly on both LCD and OLED (20x4 parity).
  - T4 full-screen fill now reliably fills all 4 rows even when preceded by `FC 10 <mode>` (streaming mode toggle).
