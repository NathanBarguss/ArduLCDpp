# Goal
Make the OLED backend present as a strict 16x3 character display so LCDproc configurations remain unchanged.

# Background
lcd2oled lets us define the logical rows/columns via `begin(width, height)`, even though the underlying panel is 128x32 pixels. We only need the upper-left 16x3 region to mimic the existing HD44780 footprint used by los-panel clients.

# Requirements
- Initialize the OLED backend with `begin(16, 3)` and clamp all cursor positions to 0..15 by 0..2.
- Update protocol parsing so commands targeting rows/columns outside the 16x3 window are safely clipped or rejected.
- Decide and document which physical rows on the OLED map to the logical third row (e.g., keep to the top three text rows and leave the fourth unused).

# Dependencies
- Display interface abstraction and OLED backend in place.
- Baseline research documenting current geometry constants (LCDW/LCDH) for comparison.

# Implementation Plan
1. Define shared geometry constants for both backends; set OLED constants to 16x3.
2. Apply range checks either in the protocol layer or within `OLEDDisplay::setCursor` to prevent out-of-bounds writes.
3. Add comments/docs clarifying the physical-to-logical row mapping to avoid future regressions.

# Acceptance Criteria
- LCDproc configured for Width 16 / Height 3 renders correctly on the OLED with no wrapping or ghost rows.
- Cursor writes beyond the 16x3 bounds are ignored or clipped without crashing.
- Manual smoke test shows row 0,1,2 content exactly where expected; the unused 4th OLED row stays blank.

# Validation Notes
- Use LCDproc demo screens that exercise writes to all rows and columns, watching for wraparound.
- Capture photos/screenshots proving logical row 2 (third row) renders correctly.
