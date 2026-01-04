# Goal
Add an `OLEDDisplay` backend using riban-bw/lcd2oled so the bridge can render onto a 128x32 I2C OLED panel.

# Background
ArduLCDpp must stay protocol-compatible with LCDproc/los-panel while expanding the physical displays it can drive. The lcd2oled library mirrors the LiquidCrystal API, so it is the fastest path to an OLED backend without rewriting rendering logic.

# Requirements
- Vendor/import lcd2oled into the repo (per `lib/` conventions) and ensure dependencies such as Wire are referenced correctly.
- Implement `OLEDDisplay` that satisfies the `IDisplay` contract by delegating to lcd2oled (constructor accepts reset pin, optional I2C address).
- Initialize lcd2oled via `begin(width, height)` during `OLEDDisplay::begin` and verify every interface call is supported (`clear`, `home`, `setCursor`, `write`, `createChar`, optional `setBacklight`).
- Provide a simple self-test (e.g., prints "hello" on boot) to confirm the OLED pipeline works when this backend is active.

# Dependencies
- Display-backend interface feature completed.
- A separate ticket will introduce the compile-time switch that chooses HD44780 vs OLED.

# Implementation Plan
1. Add lcd2oled sources to `lib/` (or submodule) and wire them into the build.
2. Create `OLEDDisplay` class implementing `IDisplay`; handle constructor parameters for reset pin and I2C address (default macros are acceptable).
3. Ensure `createChar` and other calls forward directly to lcd2oled (or provide shims where APIs differ).
4. Add an integration smoke sketch path that instantiates `OLEDDisplay` and writes a static banner for validation.

# Acceptance Criteria
- Firmware links successfully with the new `OLEDDisplay` backend (even before the config switch lands).
- Running the firmware in OLED mode displays a "hello" banner via shared `IDisplay` calls.
- No direct LiquidCrystal dependencies remain in OLED-specific code.

# Validation Notes
- Connect an OLED module and verify the banner prints without glitches.
- Sanity check I2C traffic with a logic analyzer if issues arise.
- 2026-01-04: `OLEDDisplay` boots cleanly on the bench (hello self-test visible, startup banner renders when the host is idle). Command parity is now handled by `Hd44780CommandTranslator`; continue exercising `docs/display_smoke_tests.md` T1-T6 on every OLED change to ensure we don't regress cursor or CGRAM handling.
