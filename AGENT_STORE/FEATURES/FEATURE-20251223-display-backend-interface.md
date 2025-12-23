# Goal
Introduce a display-backend interface (e.g., `IDisplay`) so all LCDproc rendering routes through a single abstraction before adding new hardware targets.

# Background
ArduLCDpp currently targets HD44780-compatible character LCDs via LiquidCrystal calls sprinkled through protocol-handling code. To support an OLED backend without destabilizing los-panel parsing, we need a clear contract that mimics "character LCD" semantics.

# Requirements
- Define an abstract interface with the operations LCDproc relies on: `begin(width, height)`, `clear()`, `home()`, `setCursor(x, y)`, `write(char)`, `createChar(slot, bitmap[8])`, and an optional `setBacklight(level)`.
- Provide a concrete `HD44780Display` implementation that wraps the current LiquidCrystal usage and becomes the default backend.
- Ensure all protocol/bridge modules call the interface only—no stray `LiquidCrystal` references remain outside the implementation class.

# Dependencies
- Baseline research documenting which LiquidCrystal calls are currently used (see `RESEARCH-20251223-lcdproc-mapping`).
- Existing LiquidCrystal library already bundled with the sketch.

# Implementation Plan
1. Define `IDisplay` (or similar) in a shared header with pure-virtual methods listed in Requirements.
2. Move current LiquidCrystal member/logic into `HD44780Display`, implementing every method with equivalent behavior.
3. Update sketch/protocol code to accept an `IDisplay&` and remove direct LiquidCrystal includes.
4. Provide a factory/helper that instantiates `HD44780Display` by default to avoid behavioral changes.

# Acceptance Criteria
- Firmware builds successfully with no behavior change on HD44780 hardware.
- Static analysis/search shows no direct LiquidCrystal calls outside `HD44780Display`.
- Existing CI/manual smoke tests pass.

# Validation Notes
- Run baseline LCDproc screens against an HD44780 module to ensure there are no regressions in blinking, cursor placement, or custom characters.
