# Goal
Display a clear “welcome/loading” screen whenever the device boots and keep it visible until the first byte of serial data is received from the host.

# Background
Test sessions showed the LCD remains blank (or retains stale characters) between power-on and the first LCDproc command, which is confusing when debugging multiple display backends. Showing a deterministic startup banner confirms the firmware is alive even if the host software is offline. This behavior should apply to both HD44780 and future OLED backends.

# Requirements
- Define a startup message (e.g., “ArduLCD Ready” or “Waiting for host…”) and render it immediately after `lcd.begin`.
- Maintain the message until actual serial traffic arrives; once data is processed, switch to normal streaming mode without residual text.
- Ensure the behavior is backend-agnostic (conditional compilation shouldn’t skip it).
- Optional nicety: blink or scroll indicator so users know the device is waiting.

# Dependencies
- PlatformIO migration work completed; code now lives in `src/main.cpp`.
- Potential ties into the display backend abstraction once OLED support ships.

# Implementation Plan
1. Add a `display_startup_screen()` helper that clears the display, prints the welcome message, and optionally shows an activity indicator.
2. Track a `bool host_active` flag; set it to `true` after the first successful `serial_read` returns data.
3. Keep the startup message visible (no writes) until `host_active` becomes true; once active, proceed with normal command handling.
4. Update README/Changelog if user-facing behavior changes.

# Acceptance Criteria
- Fresh power-on shows the startup message without waiting for host commands.
- As soon as serial bytes arrive, LCD switches to passthrough mode with no leftover banner text.
- The new behavior works for both defined PlatformIO environments without compile errors or warnings.

# Validation Notes
- Boot the firmware with LCDproc disconnected and confirm the message persists until data is sent manually via serial monitor.
- 2025-12-23: Implemented centered “ArduLCD Ready / Waiting for host...” banner plus host-activity flag in `src/main.cpp`. Cleared automatically after first serial byte.

# Status
- 2026-01-02: Verified on Nano168 hardware during T1–T8 smoke tests; README and AGENTS updated accordingly. Resolved via commits `4e1f957` and `77c84e5`.
