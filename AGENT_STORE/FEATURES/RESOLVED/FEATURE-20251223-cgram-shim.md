# Goal
Convert HD44780 CGRAM writes from LCDproc into `IDisplay::createChar()` calls so custom glyphs (bars/icons) render identically on OLED.

# Background
LCDproc programs custom characters by sending `0xFE 0x40|addr` followed by 8 data bytes per slot. The OLED backend must capture and reinterpret those writes because `lcd2oled` relies on `createChar(slot, bitmap)` rather than raw CGRAM memory access.

# Requirements
- Detect when the translator enters CGRAM-address mode (0x40-0x7F) and determine the 3-bit slot index plus internal row pointer.
- Buffer 8 rows (5-bit values) per slot, then call `createChar(slot, bitmap)` as the CGRAM address advances.
- Handle back-to-back slot definitions without losing synchronization.
- Ensure slots 0-7 wrap correctly, mirroring HD44780 behavior.

# Dependencies
- OLED command translator.
- `IDisplay` exposes `createChar`.

# Implementation Plan
1. Extend the translator with CGRAM state (slot index + row pointer + cache).
2. Mask incoming CGRAM data bytes to 5 bits, store, and advance address.
3. Call `createChar()` as the cache updates so animated glyphs remain in sync.

# Acceptance Criteria
- OLED renders LCDproc custom glyph slots 0-7 with no corruption.
- Dual mode keeps LCD and OLED glyphs in parity even during burst-safe queueing.

# Validation Notes
- Use `scripts/t5_custom_chars.py` and compare LCD vs OLED.

## Verification (2026-01-08, QA)
- PASS T5 (Dual, `nano168_dual_serial`, `COM6`): both LCD and OLED show the same 8 glyphs on row 0 and row 1.

