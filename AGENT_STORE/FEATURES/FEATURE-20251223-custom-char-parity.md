# Goal
Guarantee that custom character slots (CGRAM 0-7) behave identically on both HD44780 and OLED backends so LCDproc bargraphs/icons render correctly.

# Background
LCDproc relies heavily on custom glyphs for progress bars and status indicators. lcd2oled claims LiquidCrystal compatibility, but we must verify and, if necessary, shim behaviors so glyph writes remain deterministic.

# Requirements
- Confirm that the current HD44780 command translation path correctly converts incoming CGRAM writes (via `0xFE` commands) into `IDisplay::createChar(slot, bitmap)` calls.
- Confirm lcd2oled correctly stores and renders glyphs for slots 0-7; if not, introduce a small cache/translation layer (or fix) in the OLED path so subsequent writes of bytes `0x00..0x07` render the expected glyphs.
- Ensure dual mode mirrors CGRAM updates to both panels (or defers them safely) so LCD + OLED stay in parity even when burst-safe queueing is enabled.
- Provide a repeatable validation procedure (and a small host-side helper script) that loads all eight slots with distinctive bitmaps and demonstrates they can be written out by character code 0-7.

# Dependencies
- OLED backend implementation in place.
- HD44780 command translator in place (including CGRAM support).

# Implementation Plan
1. Document the current CGRAM path:
   - Incoming `0xFE 0x40|addr` sets CGRAM address mode.
   - Next 8 bytes per slot row update a cached bitmap, and the translator calls `IDisplay::createChar(slot, bitmap)` to update the backend.
2. Validate LCD-only, OLED-only, and Dual builds with an explicit 0..7 slot test:
   - Use `docs/display_smoke_tests.md` T5, or run the helper script `scripts/t5_custom_chars.py`.
3. If dual parity breaks under burst-safe queueing:
   - Adjust the dual display implementation so `createChar()` updates reach the secondary display (either write-through, or "defer and replay" on idle).
4. Capture baseline parity evidence (photos) for a representative LCDproc screen that uses bargraphs/icons.

# Acceptance Criteria
- LCDproc screens that rely on custom glyphs (bars/icons) render correctly on OLED without corruption.
- A host-driven test demonstrates:
  - Programming slots 0-7 via the los-panel CGRAM write sequence.
  - Rendering bytes `0x00..0x07` on-screen with the expected glyph shapes.
- No regressions for HD44780 behavior.
- Dual mode renders the same glyphs on both panels (or clearly documents if burst-safe queueing defers CGRAM updates until idle).

# Validation Notes
- Use `docs/display_smoke_tests.md` (T5) as the canonical procedure.
- Quick host-side validation (recommended):
  - `python scripts/t5_custom_chars.py --port COM6`
  - Expected: both panels show 8 distinct glyphs for slots 0..7 (ideally on row 0), and the shapes match between LCD and OLED.
- Capture photos comparing OLED vs HD44780 outputs for at least one bargraph example (ideally the same LCDproc dashboard on both panels).

# Notes (current status / risk)
- Current CGRAM translation: `src/display/Hd44780CommandTranslator.cpp` caches CGRAM row bytes and calls `createChar(slot, bitmap)` as the host programs CGRAM.
- Burst-safe dual queueing caveat: when `ENABLE_DUAL_QUEUE` is enabled and queueing is active, `DualDisplay::createChar()` currently does not forward to the secondary display. If LCDproc sends CGRAM updates during a session, the OLED could miss glyph updates until this is addressed.
