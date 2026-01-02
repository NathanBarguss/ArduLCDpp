# Goal
Convert HD44780 CGRAM writes from LCDproc into lcd2oled `createChar` calls so custom glyphs (bars/icons) render identically on OLED.

# Background
LCDproc programs custom characters by sending `0xFE 0x40|addr` followed by 8 data bytes per slot. The existing firmware simply forwards those bytes to `LiquidCrystal::command`, but the OLED backend must capture and reinterpret them because lcd2oled relies on `createChar(slot, bitmap)` APIs instead of raw CGRAM memory access.

# Requirements
- Detect when the translator enters CGRAM-address mode (0x40–0x7F) and determine the 3-bit slot index plus internal row pointer.
- Buffer eight 5-bit column values (LSBs of each byte) per slot, then call `createChar(slot, bitmap)` once the slot is fully written.
- Handle back-to-back CGRAM definitions (LCDproc often defines multiple glyphs sequentially) without losing synchronization.
- Ensure writes to slots 0–7 wrap correctly, mirroring HD44780 behavior.

# Dependencies
- Command translator feature for the OLED backend.
- `IDisplay` interface exposes `createChar`.

# Implementation Plan
1. Extend the translator with a CGRAM state struct: current slot index, row pointer, temporary bitmap buffer.
2. On receiving a CGRAM data byte, mask to 5 bits (HD44780 uses only the lower 5) and store in the buffer; advance row pointer.
3. Once 8 rows collected, invoke `createChar(slot, buffer)` and reset row pointer. If LCDproc continues writing (more than 8 bytes), treat it as next slot automatically (HD44780 increments address).
4. Add tests/demos that feed LCDproc bargraph glyphs and verify OLED output matches HD44780 photographs/videos.

# Acceptance Criteria
- OLED renders LCDproc bargraphs/icons using slots 0–7 with no corruption.
- Stress tests redefining glyphs at runtime show the state machine keeps pace (no stale data).
- Implementation tolerates partial CGRAM writes (e.g., LCDproc cancels mid-write) without crashes.

# Validation Notes
- Compare OLED output to a reference HD44780 display for at least one screen that animates custom glyphs.
