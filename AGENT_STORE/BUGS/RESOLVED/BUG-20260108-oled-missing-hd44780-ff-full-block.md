# BUG-20260108-oled-missing-hd44780-ff-full-block

### Summary
On HD44780 displays, byte `0xFF` renders as a solid “full block” character. The OLED emulation treated bytes `> 0x7F` as invalid and converted them to spaces, so host apps that rely on `0xFF` for solid fills render incorrectly on OLED.

### Environment
- Firmware: ArduLCDpp OLED or dual builds
- Library: `lib/lcd2oled`
- Repro: `scripts/pc_clock.py` big-digit fills use `0xFF`

### Steps to Reproduce
1. Flash `nano168_dual` and run `python scripts/pc_clock.py --port COM6`.
2. Observe that “filled rectangle” segments render as blanks on the OLED while the LCD shows solid fills.

### Expected/Actual Results
- Expected: `0xFF` renders as a solid filled 5x8 cell on OLED (matching HD44780).
- Actual: OLED renders blanks because `lcd2oled::Write()` clamps `>0x7F` to space.

### Fix Plan
- Map `0xFF` to `OLED_CHAR_ALLON` (0x80) inside `lcd2oled::Write()` and allow `0x80` through the validation clamp.

### Verification Notes
- Reflash and re-run `scripts/pc_clock.py`; confirm the solid-fill segments appear on OLED and match the LCD.
- Resolved by `lib/lcd2oled` commit `6b2c11f` (referenced by ArduLCDpp `57023a2`).
