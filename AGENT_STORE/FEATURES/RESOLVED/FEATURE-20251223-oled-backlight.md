# Goal
Map los-panel backlight commands onto meaningful OLED brightness behavior so "backlight off" still produces a visible effect.

# Background
The HD44780 path controls a physical backlight pin, but OLEDs need brightness adjustments instead. lcd2oled exposes helpers like `SetBrightness(0-255)` that we can translate to.

# Requirements
- Keep existing HD44780 backlight handling unchanged.
- On the OLED backend, translate backlight on/off (and any intermediate levels if present) into brightness values (e.g., 0 for off, configured default for on).
- Ensure rapid toggling from LCDproc does not cause lockups or visible glitches.
- Document the chosen brightness defaults and how to override them via macros.

# Dependencies
- OLED backend and config-switch features in place.

# Implementation Plan
1. Define default brightness constants (e.g., `OLED_BRIGHTNESS_ON`, `OLED_BRIGHTNESS_OFF`).
2. Implement `OLEDDisplay::setBacklight(level)` to call the lcd2oled brightness API, clamping to valid ranges.
3. Stress test by scripting rapid backlight toggles from LCDproc or a test sketch; adjust debouncing if necessary.
4. Update docs/config headers to describe brightness customization.

# Acceptance Criteria
- Toggling backlight via LCDproc visibly dims/brightens the OLED.
- No crashes or hangs occur during rapid toggles.
- Developers can adjust default brightness levels through macros without editing code.

# Validation Notes
- Record video/photos showing the brightness change for documentation/testing.

## Notes (2026-01-07)
- Implemented OLED brightness mapping:
  - `OLEDDisplay::setBacklight(level)` maps los-panel `0xFD <level>` to SSD1306 contrast with a visible floor for non-zero values.
  - Defaults live in `include/DisplayConfig.h`: `OLED_BRIGHTNESS_MIN=32`, `OLED_BRIGHTNESS_MAX=255` (override via `platformio.ini` build flags).
- Bench sanity: rapid toggles `FD 00/20/80/FF` produce visible contrast changes without hangs.
