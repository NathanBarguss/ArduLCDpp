# Summary
Backlight PWM pin is hard-coded as D10 in firmware, but the documented Nano wiring (and physical prototype) use D11, causing the backlight control line to collide with LCD data bit D7.

# Environment
- Branch: `start_port`
- Hardware: Arduino Nano prototype per `AGENTS.MD`
- Firmware: PlatformIO (`uno_hd44780`, `mega2560_hd44780`)

# Steps to Reproduce
1. Flash current firmware to the Nano with both HD44780 LCD data bus and backlight transistor wired per the documented schematic (LCD D7 on D10, backlight PWM on D11).
2. Power the board.
3. Observe the LCD flicker or show corrupted data when backlight commands are issued, and note that PWM changes have no effect.

# Expected Results
- PWM backlight control should be driven from D11 as described in `AGENTS.MD`, leaving D10 dedicated to LCD data bit D7.

# Actual Results
- Firmware drives backlight via D10, clashing with the LCD D7 line; brightness control is ineffective and can corrupt display writes.

# Fix Plan
1. Update `LED_PIN` definition in `include/DisplayConfig.h` (and any other references) from D10 to D11.
2. Ensure the display factory wiring map aligns with the corrected backlight pin.
3. Update documentation/README/AGENTS as needed to note the fix.
4. Rebuild firmware (`pio run -e uno_hd44780` and `-e mega2560_hd44780`) to confirm successful compilation.

# Verification Notes
- After the change, confirm the backlight responds to `0xFD` brightness commands without affecting LCD data integrity.
