# Goal
Deliver a dual-display firmware mode that mirrors every lcdproc command to both the HD44780 and SSD1306 panels simultaneously so UX/QA can compare output parity on the bench.

# Background
Treating LCD and OLED as mutually exclusive compile-time targets blocks rapid evaluation (reflash required). Since both panels are wired on the bench, dual mode provides parity validation without swapping binaries.

# Requirements
- Introduce a `DUAL` display mode that instantiates both concrete backends and mirrors every `IDisplay` method call (`write`, `command`, `createChar`, `clear`, `setBacklight`, etc.).
- Maintain LCD-only and OLED-only binaries; dual must be opt-in via `DISPLAY_BACKEND`.
- Ensure los-panel byte streams drive both panels identically (DDRAM + CGRAM parity).
- Document host workflow and caveats (auto-reset on serial open, burst-safe mode).

# Dependencies
- `IDisplay` abstraction.
- OLED backend + translator.
- Serial-overrun mitigation for Nano168 dual+debug.

# Implementation Plan
1. Implement `DualDisplay` composite that fans out calls to two `IDisplay` instances.
2. Extend `display_factory` to select LCD, OLED, or DUAL by `DISPLAY_BACKEND`.
3. Add a burst-safe path for Nano168 dual+debug that defers OLED/LCD updates during bursts and refreshes during idle.
4. Validate with smoke suite T1–T8 on the bench.

# Acceptance Criteria
- Dual build shows identical boot banners and host-driven content on both panels.
- Brightness/backlight bytes affect both panels predictably (LCD PWM + OLED contrast mapping).
- Burst tests T4/T8 do not reset and end in parity.

# Validation Notes
- Use `docs/display_smoke_tests.md` and scripts under `scripts/`.

## Verification (2026-01-08, QA)
- Bench: Nano ATmega168 dual build on `COM6` (`nano168_dual_serial`)
- T1 PASS (boot banner)
- T2 PASS (clear/home + `A` at 0,0)
- T3 PASS (DDRAM sweep 20x4)
- T4 PASS (unpaced full-screen fill; ends with 4 full rows on LCD+OLED)
- T5 PASS (custom glyph parity slots 0–7)
- T6 PASS (brightness 00/80/FF affects both LCD and OLED)
- T7 N/A (USB-only power; unplug resets board)
- T8 PASS (unpaced 1KB stress burst; no reset; displays remain in parity)

## Notes
- Dual burst-safe builds defer display refresh until RX is idle (~20ms) to prevent UART overruns on Nano168.

