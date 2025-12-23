# Goal
Convert ArduLCDpp from a legacy Arduino-IDE sketch into a full PlatformIO project so firmware variants can be built, tested, and released consistently across boards.

# Background
The current repo ships only `sketch/sketch.ino`, which depends on the Arduino IDE’s implicit prototypes and manual library installs. PlatformIO gives us reproducible builds, explicit dependency tracking, and multi-environment configs (e.g., Uno vs Mega vs future OLED boards). Other backlog items already mention PlatformIO flags, so migrating the core project eliminates duplicated effort when new backends arrive.

# Requirements
- Initialize PlatformIO in the repo root with a canonical `platformio.ini` and the standard `src/`, `include/`, and `lib/` layout.
- Move the existing sketch into `src/main.cpp`, adding `#include <Arduino.h>` and any missing prototypes so it compiles as C++.
- Define PlatformIO environments for the supported boards (e.g., Uno, Mega2560) with shared settings for baud rate, monitor speed, and `DISPLAY_BACKEND` build flags.
- Declare third-party dependencies (LiquidCrystal and any others) via `lib_deps`; custom helpers should live under `lib/`.
- Update documentation (README + build section) to describe PlatformIO workflows (`pio run`, `pio run -t upload`, `pio device monitor`) and note the new directory structure.
- Ensure `.gitignore` already covers `.pio/` output; add entries if new artifacts surface.

# Dependencies
- Display backend abstraction work (e.g., `FEATURE-20251223-backend-config-switch.md`) so build flags remain consistent.
- Smoke-test matrix (`FEATURE-20251223-smoke-test-matrix.md`) to validate both PlatformIO environments post-migration.
- Research notes in `RESEARCH-20251223-lcdproc-mapping.md` for guidance on serial handling under PlatformIO.

# Implementation Plan
1. Run `pio project init --board uno` (adjust board list as needed) at repo root to scaffold PlatformIO files.
2. Move `sketch/sketch.ino` into `src/main.cpp`, wrap any header-only constants in `include/config.h`, and verify the code compiles under PlatformIO.
3. Add additional environments to `platformio.ini` for each target MCU plus any feature toggles (HD44780 vs OLED) using `build_flags`.
4. Populate `lib_deps` with required libraries and migrate local helper code into the `lib/` tree.
5. Refresh README/build instructions to document PlatformIO commands, environment selection, and how to adjust backend flags.
6. Run `pio run` for every environment and document results in Validation Notes or the smoke-test ticket.

# Acceptance Criteria
- `pio run` succeeds for all defined environments without manual IDE steps.
- `src/main.cpp` (and any headers) compile without Arduino IDE-specific quirks; the project structure matches PlatformIO conventions.
- Documentation clearly explains how to build, upload, and monitor firmware using PlatformIO, including backend flag usage.
- Legacy Arduino IDE instructions are either updated to point at PlatformIO or clearly marked as deprecated.

# Validation Notes
- Capture the `pio run` commands and target environments exercised (e.g., `pio run -e uno`, `pio run -e mega2560`).
- Note any follow-up issues (missing libraries, upload failures) if they block completion so they can be logged as BUG tickets.
