# Goal
Add a compile-time configuration switch that selects the active display mode (HD44780, OLED, or the dual-parity build) along with related macro hooks.

# Background
The MVP deployment story requires clear firmware binaries: one for character LCD shields, one for the OLED build, and a dual-display parity build so UX/QA can compare panels without reflashing.

# Requirements
- Introduce a `DISPLAY_BACKEND` macro with allowed values `HD44780`, `OLED`, and `DUAL`; invalid values should trigger a compile-time error.
- Selecting `DUAL` must instantiate both concrete backends and route every `IDisplay` call through the fan-out helper.
- Provide companion macros for OLED specifics (`OLED_RESET_PIN`, `OLED_DEFAULT_I2C_ADDRESS`, brightness mapping) with sensible defaults and override support via build flags.
- Ensure build scripts / documentation show how to set the flag for PlatformIO environments.
- Verify all three configurations compile.

# Dependencies
- `IDisplay` abstraction and both backend implementations available.

# Implementation Plan
1. Centralize config in `include/DisplayConfig.h` with `DISPLAY_BACKEND` defaults and validation.
2. Use `src/display/display_factory.cpp` to instantiate the requested backend via `#if DISPLAY_BACKEND == ...`.
3. Expose PlatformIO envs for LCD-only, OLED-only, and Dual.

# Acceptance Criteria
- Setting the macro (via build flags) selects the correct backend/mode with no code changes elsewhere.
- LCD-only, OLED-only, and dual builds compile successfully.
- Invalid `DISPLAY_BACKEND` values emit a compile-time error.

# Validation Notes
- PlatformIO envs:
  - `nano168_hd44780`, `nano168_oled`, `nano168_dual`, `nano168_dual_serial`

## Verification (2026-01-08, QA)
- Built and flashed `nano168_dual_serial` and completed T1–T8 smoke suite on `COM6` (T7 N/A due to USB-only power).

