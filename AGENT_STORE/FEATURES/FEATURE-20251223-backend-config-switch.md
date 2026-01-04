# Goal
Add a compile-time configuration switch that selects the active display mode (HD44780, OLED, or the new dual-parity build) along with related macro hooks.

# Background
The MVP deployment story requires clear firmware binaries: one for character LCD shields, one for the OLED build, and now a dual-display parity build so UX can compare panels without reflashing. Toggling preprocessor flags is the simplest approach while we still rely on Arduino/PlatformIO builds.

# Requirements
- Introduce a `DISPLAY_BACKEND` macro (or equivalent config header) with allowed values `HD44780`, `OLED`, and `DUAL`; invalid values should trigger a compile-time error.
- Selecting `DUAL` must instantiate both concrete backends and route every `IDisplay` call through the fan-out helper (see FEATURE-20260104-dual-display-parity).
- Provide companion macros for OLED specifics (`OLED_RESET_PIN`, `OLED_I2C_ADDR`, optional brightness defaults) with sensible defaults that developers can override.
- Ensure build scripts / documentation show how to set the flag for both Arduino IDE and PlatformIO.
- Verify all three configurations compile without dead-code warnings.

# Dependencies
- `IDisplay` abstraction and both backend implementations available.

# Implementation Plan
1. Create a central `display_config.h` (or similar) defining `DISPLAY_BACKEND` defaults and validation/`static_assert` logic.
2. Update the sketch entry point (or `display_factory`) to instantiate the appropriate backend via `#if DISPLAY_BACKEND == ...` logic, including a composite type for the dual mode.
3. Document flag usage in README/docs and, if applicable, extend CI or local scripts to build all variants.
4. Run builds for every backend mode to ensure there are no missing symbols or warnings.

# Acceptance Criteria
- Setting the macro (via compiler flags or header edits) selects the correct backend/mode with no code changes elsewhere.
- LCD-only, OLED-only, and dual builds compile successfully; continuous integration or manual scripts perform all three builds.
- Invalid `DISPLAY_BACKEND` values emit a compile-time error with a clear message.

# Validation Notes
- Record the exact build commands in the ticket or associated docs for future regression testing.
