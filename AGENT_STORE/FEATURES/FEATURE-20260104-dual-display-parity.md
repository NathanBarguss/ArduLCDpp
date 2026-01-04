# Goal
Deliver a dual-display firmware mode that mirrors every lcdproc command to both the HD44780 and SSD1306 panels simultaneously so UX can compare output parity on the bench.

# Background
Until now we treated the LCD and OLED as mutually exclusive compile-time targets. That made sense while bringing the OLED online, but it blocks rapid UX evaluation because we have to reflash between builds to see rendering differences. The wiring already keeps both panels connected, so the missing piece is firmware support for driving them in parallel without regressing the existing single-backend builds.

# Requirements
- Introduce a `DUAL` display mode (name TBD) that instantiates both concrete backends and mirrors every `IDisplay` method call, including `write`, `command`, `createChar`, `clear`, and `setBacklight`.
- Ensure brightness commands map sensibly on both panels (e.g., LCD PWM vs OLED contrast) without diverging UX cues.
- Maintain the ability to ship LCD-only and OLED-only binaries; the dual path should be opt-in and compile-time selectable just like the dedicated builds.
- Document wiring, power considerations, and host workflow so UX/QA know when to use the dual mode versus the single-backend binaries.
- Update smoke-test guidance to include a parity spot-check (even if manual at first) so we catch any divergence between the two displays.

# Dependencies
- HD44780 backend (current default) and OLED backend (in-progress tickets) must both expose matching `IDisplay` behavior.
- Backlight calibration work should define mappings the dual mode can reuse.

# Implementation Plan
1. Add a composite implementation (e.g., `DualDisplay`) that wraps two `IDisplay` instances and forwards every call, handling any per-backend quirks internally.
2. Extend `display_factory` to understand the new dual mode, instantiating both concrete displays once and wiring them into the composite.
3. Decide how `DISPLAY_BACKEND` (or successor flag) selects between LCD, OLED, and DUAL to avoid ad-hoc build flags.
4. Update docs/readme + AGENT_STORE guidance to explain when dual mode is useful and any hardware caveats (power draw, startup timing, etc.).
5. Exercise manual smoke tests with both panels connected; capture any timing or current issues before wider adoption.

# Acceptance Criteria
- Building with the new dual mode flag results in both displays showing identical startup banners and host-driven content without recompiling between tests.
- Brightness commands affect both panels predictably, with documented mapping.
- Existing single-backend builds continue to compile and run without regression.
- README/AGENT instructions tell contributors how to enable the dual build and why they might use it.

# Validation Notes
- Run the standard docs/display_smoke_tests checklist while watching both panels, noting any mismatches or ghosting in the ticket.
- Record supply-current observations (USB-powered vs. external) so future UX runs know if extra power is required.
