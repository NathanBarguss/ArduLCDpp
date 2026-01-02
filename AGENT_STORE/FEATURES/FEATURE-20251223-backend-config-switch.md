# Goal
Add a single compile-time configuration switch that selects the active display backend (HD44780 vs OLED) along with related macro hooks.

# Background
The MVP deployment story requires clear firmware binaries: one for character LCD shields and one for the new OLED build. Toggling preprocessor flags is the simplest approach while we still rely on Arduino/PlatformIO builds.

# Requirements
- Introduce a `DISPLAY_BACKEND` macro (or equivalent config header) with allowed values `HD44780` and `OLED`; invalid values should trigger a compile-time error.
- Provide companion macros for OLED specifics (`OLED_RESET_PIN`, `OLED_I2C_ADDR`, optional brightness defaults) with sensible defaults that developers can override.
- Ensure build scripts / documentation show how to set the flag for both Arduino IDE and PlatformIO.
- Verify both configurations compile without dead-code warnings.

# Dependencies
- `IDisplay` abstraction and both backend implementations available.

# Implementation Plan
1. Create a central `display_config.h` (or similar) defining `DISPLAY_BACKEND` defaults and validation logic.
2. Update the sketch entry point to instantiate the appropriate backend via `#if DISPLAY_BACKEND == ...` logic.
3. Document flag usage in README/docs and, if applicable, extend CI or local scripts to build both variants.
4. Run builds for each backend to ensure there are no missing symbols or warnings.

# Acceptance Criteria
- Setting the macro (via compiler flags or header edits) selects the correct backend with no code changes elsewhere.
- Both builds compile successfully; continuous integration or manual scripts perform both builds.
- Invalid `DISPLAY_BACKEND` values emit a compile-time error with a clear message.

# Validation Notes
- Record the exact build commands in the ticket or associated docs for future regression testing.
