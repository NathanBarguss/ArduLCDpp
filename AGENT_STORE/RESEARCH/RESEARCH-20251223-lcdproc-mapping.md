# Objective
Document the current behavior of ArduLCDpp when it translates LCDproc/los-panel commands into LiquidCrystal operations so that adding non-HD44780 backends has clear requirements.

# Key Findings
- `sketch/sketch.ino` only touches a narrow LiquidCrystal surface: `begin`, `display`, `clear`, `write(const char*)`, `write(byte)`, `home`, and `command`. All higher-level behaviors (cursor moves, blink, custom chars) are exercised by LCDproc through the `command` passthrough.
- LCD geometry is compile-time via `#define LCDW 20` / `#define LCDH 4`, feeding both `lcd.begin` and the boot banner. README still claims a 24x2 default, so documentation and code disagree.
- los-panel bytes map as follows: `0xFE` plus the next byte becomes `lcd.command()`, `0xFD` plus the next byte drives PWM backlight via `analogWrite`, and any other byte streams straight into `lcd.write`. There is no additional buffering or validation.
- No direct GPIO toggling occurs outside the backlight PWM path, and the firmware never calls `lcd.createChar`; OLED backends must therefore honor raw `command` writes for CGRAM updates.
- Findings are summarized for future contributors in `docs/lcdproc_display_mapping.md`.

# Sources
- Firmware: `sketch/sketch.ino`
- Project README for configuration context
- `docs/lcdproc_display_mapping.md`
- LCDproc los-panel protocol documentation (links in README/resources)

# Open Questions
1. Need to capture a trace of LCDproc traffic to confirm which HD44780 opcodes are exercised most often (especially `createChar`) so the OLED backend can prioritize parity tests.
2. Determine whether the analogWrite 0-255 curve matches user expectations for brightness; documentation may need to specify linear vs perceptual behavior.
3. Reconcile the README's stated default geometry (24x2) with the actual 20x4 defines before publishing OLED instructions.
4. Validate that the blocking `serial_read()` loop does not starve other tasks when ported to PlatformIO/USB CDC.

# Recommended Next Steps
1. Link and reference `docs/lcdproc_display_mapping.md` from README/AGENT_STORE tickets so new contributors land on the definitive mapping.
2. Capture an LCDproc session log (or instrument the sketch) to measure command mix, custom char usage, and backlight frequency.
3. Decide on canonical default dimensions (20x4 vs 24x2) and update constants/documents together.
4. Prototype the display interface refactor with PlatformIO in mind, ensuring the `HardwareSerial` include path and blocking serial loop continue working.
