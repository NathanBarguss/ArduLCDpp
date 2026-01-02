# Goal
Refresh the startup banner so it reflects the ArduLCDpp name and promotes the project link while still indicating host readiness.

# Background
After adding the initial “ArduLCD Ready / Waiting for host…” banner, we want to align the text with the ArduLCDpp brand and show a compact URL for contributors/users. The display is 20×4, so we can fit an extra line without affecting host passthrough behavior.

# Requirements
- Update the power-on screen to say “ArduLCDpp Ready” (line 1), retain the “Waiting for host…” status (line 2), and add `bit.ly/4plthUv` (line 3).
- Ensure the banner remains centered horizontally on each line when possible.
- Keep the existing behavior where the banner clears automatically after the first serial byte arrives.
- Maintain compatibility with both HD44780 and future OLED backends.

# Dependencies
- Current startup-screen implementation in `src/main.cpp`.
- Display geometry constants (`LCDW`, `LCDH`) defined in `include/DisplayConfig.h`.

# Implementation Plan
1. Extend `display_startup_screen()` to include the third line with the short URL, respecting available rows.
2. Confirm centering logic handles 20-column displays (truncate or left-align if necessary).
3. Update documentation (e.g., README changelog, smoke-test matrix) to reflect the new message.
4. Rebuild via PlatformIO environments to verify no regressions.

# Acceptance Criteria
- Booting the firmware shows the three-line message (“ArduLCDpp Ready”, “Waiting for host…”, `bit.ly/4plthUv`) until host data arrives.
- Text remains centered or gracefully truncated on smaller displays.
- No change to serial handling or backlight logic.

# Validation Notes
- Capture photos or logs from both Uno and Mega builds demonstrating the new banner prior to host traffic.
