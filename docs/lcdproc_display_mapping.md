# LCDproc Display Mapping

## Purpose
ArduLCDpp acts as a USB/serial bridge for LCDproc's `los-panel` driver. This note summarizes how incoming protocol bytes are translated into LiquidCrystal calls so alternate display backends (e.g., OLED) can preserve behavior without reading the entire sketch.

## Hardware + Dimensions
- `LiquidCrystal lcd(12, 11, 2, 3, 4, 5, 6, 7, 8, 9);` configures a full 8-bit parallel bus: RS=12, RW=11, enable=2, data pins D0-D7 map to Arduino pins 3-9 (`sketch/sketch.ino`).
- Geometry is compile-time via `#define LCDW 20` / `#define LCDH 4`; no runtime overrides. These feed both `lcd.begin(LCDW, LCDH);` and the boot banner string (`"%dx%d Ready"`).
- Code assumes the display actually matches `LCDW`/`LCDH`. LCDproc must therefore be configured with the same Width/Height.

## Startup Sequence
1. Configure `LED_PIN` (Nano D11 per wiring doc) for PWM backlight control and immediately set brightness using `STARTUP_BRIGHTNESS` (8-bit value passed to `analogWrite`).
2. Call `lcd.begin(LCDW, LCDH)` to initialize the controller.
3. Initialize serial at `BAUDRATE` (default 57600) for los-panel traffic.
4. Issue `lcd.display()`, `lcd.clear()`, print the `%dx%d Ready` banner via `lcd.write()` and return `lcd.home()`.

## los-panel Command Handling
- Main loop blocks on `serial_read()` until a byte arrives.
- `0xFE`: treated as an escape prefix; the next byte is passed directly to `lcd.command()`, giving LCDproc raw access to HD44780 instructions (set cursor, clear, cursor blink, etc.). No filtering or validation occurs.
- `0xFD`: interpreted as backlight control; the following byte is forwarded to `set_backlight()` (0–255 PWM duty cycle).
- Any other byte is treated as printable data and written with `lcd.write(cmd)`.

## LiquidCrystal API Surface in Use
| Operation | Call Site | Notes |
|-----------|-----------|-------|
| `begin(width, height)` | `setup()` | Uses `LCDW`/`LCDH` macros; must be mirrored by new backends. |
| `display()` | `setup()` | Ensures screen is unblanked after init. |
| `clear()` | `setup()` | Clears display before welcome text. |
| `write(const char*)` | `setup()` | Prints boot banner sized to LCDW; later `loop()` uses `write(byte)` for stream data. |
| `home()` | `setup()` | Returns cursor to (0,0) after banner. |
| `command(byte)` | `loop()` when 0xFE prefix arrives | Pass-through for raw HD44780 instructions; required for LCDproc cursor moves, custom chars, blink, etc. |

No `createChar`, `setCursor`, `scrollDisplay*`, or `blink/cursor` helpers are called directly; LCDproc invokes those behaviors through the `command` passthrough.

## Backlight Behavior
- `set_backlight(int value)` does a single `analogWrite(LED_PIN, value)` with the byte that followed `0xFD` (Nano D11).
- LCDproc can therefore dim the panel in 0–255 steps; there is no rate limiting or smoothing.
- Startup brightness defaults to `STARTUP_BRIGHTNESS` (currently `2`) until LCDproc overrides it.

## Custom Characters & CGRAM
- Firmware never calls `lcd.createChar`; LCDproc must push custom glyph bitmaps itself via the `0xFE` command path. Any backend replacement must therefore support `command()` dispatches for CGRAM writes.

## Error Handling & Edge Cases
- Serial parsing blocks until data arrives; there is no timeout.
- Out-of-range brightness values are passed straight to `analogWrite` (the los-panel spec already limits to 0–255).
- Because `lcd.command()` accepts whatever byte LCDproc sends, invalid or unsupported opcodes are simply forwarded to the hardware.

## Implications for New Backends
- Replicate the `command(byte)` escape behavior so LCDproc retains full control over cursor positioning, custom chars, and blink state.
- Provide equivalents for the LiquidCrystal API calls listed above. Anything not explicitly used can be stubbed for now.
- Respect compile-time geometry; an OLED backend should translate the same `LCDW`/`LCDH` values into its logical window.
- Implement a backlight/brightness hook that reacts to the `0xFD` command just like `set_backlight` currently does.

## References
- Implementation: `sketch/sketch.ino`
- Protocol background: lcdproc `los-panel` documentation (`resources/LCDd.conf`, README links)
