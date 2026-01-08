# BUG-20260108-oled-custom-char-row7-truncated

### Summary
OLED backend drops the 8th row (bottom pixel row) of custom characters, causing big-digit glyphs (clock demo) to render with missing segments on SSD1306 even though the HD44780 LCD renders correctly.

### Environment
- Firmware: ArduLCDpp dual build (`nano168_dual` observed)
- Display(s): HD44780 20x4 + SSD1306 I2C OLED (128x32)
- Host: `scripts/pc_clock.py` (big-digit 24h clock)

### Steps to Reproduce
1. Flash `nano168_dual` and wire both LCD + OLED.
2. Run `python scripts/pc_clock.py --port COM6 --backlight 255`.
3. Observe big-digit segments on OLED vs LCD.

### Expected/Actual Results
- Expected: OLED matches LCD big-digit glyphs (all segments present).
- Actual: OLED shows partial/missing big-digit segments while colon animation still works.

### Fix Plan
- Update `lib/lcd2oled/lcd2oled.cpp` `lcd2oled::createChar()` to rotate all 8 rows of the HD44780 bitmap (5x8), not only 7.

### Verification Notes
- Rebuild and flash, then re-run `scripts/pc_clock.py`; confirm all big-digit segments render on OLED.
- Optionally re-run `scripts/t5_custom_chars.py` to validate CGRAM parity for slots `0..7` across both panels.
- Resolved by `lib/lcd2oled` commit `2b1cfcb` (referenced by ArduLCDpp `57023a2`).
