# Goal
Guarantee that custom character slots (CGRAM 0-7) behave identically on both HD44780 and OLED backends so LCDproc bargraphs/icons render correctly.

# Background
LCDproc relies heavily on custom glyphs for progress bars and status indicators. lcd2oled claims LiquidCrystal compatibility, but we must verify and, if necessary, shim behaviors so glyph writes remain deterministic.

# Requirements
- Audit current `createChar` usage (number of slots, refresh cadence) from the baseline research.
- Confirm lcd2oled correctly stores and renders glyphs for slots 0-7; if not, introduce a small cache/translation layer inside `OLEDDisplay`.
- Provide a simple sketch/test that loads all eight slots with distinctive bitmaps and demonstrates they can be written out by character code 0-7.

# Dependencies
- OLED backend implementation in place.
- Baseline research doc enumerating current custom character usage.

# Implementation Plan
1. Instrument both backends to log/createChar calls to understand usage patterns (optional but helpful).
2. Implement any necessary caching/shims to ensure repeated createChar calls work even if lcd2oled stores glyphs differently.
3. Add a manual test sketch (or extend smoke test) that draws bargraph glyphs on both displays and confirm parity visually.

# Acceptance Criteria
- LCDproc screens that rely on custom glyphs (bars/icons) render correctly on OLED without corruption.
- Unit/integration sketch demonstrates writing slots 0-7 and reading them back via the display.
- No regressions for HD44780 behavior.

# Validation Notes
- Capture photos comparing OLED vs HD44780 outputs for at least one bargraph example.
