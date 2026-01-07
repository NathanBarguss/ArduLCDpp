# SSD1306 OLED (I2C) Setup

This guide documents how to wire an SSD1306 OLED to an Arduino Nano running ArduLCDpp, which environments to build, and how to point LCDproc at the firmware (los-panel protocol).

## Hardware

### Wiring (Nano -> SSD1306)

| Nano Pin | OLED Pin | Notes |
|----------|----------|-------|
| A4       | SDA      | I2C data |
| A5       | SCL      | I2C clock |
| A3       | RESET    | Optional; only if your module exposes reset |
| 5V / 3.3V | VCC     | Module-dependent; confirm your breakout |
| GND      | GND      | Common ground |

### Common pitfalls
- If the OLED is blank, double-check power (many breakouts are 3.3V-only).
- The most common SSD1306 I2C addresses are `0x3C` and `0x3D`.
- If you have RESET connected, ensure the chosen pin matches the firmware config.

## Firmware Configuration

ArduLCDpp exposes backends via PlatformIO environments in `platformio.ini`.

### Recommended environments
- OLED only: `nano168_oled`
- Dual parity (LCD + OLED): `nano168_dual`
- Dual + serial debug + burst-safe streaming (Nano168): `nano168_dual_serial`

### Build + upload

```powershell
# Replace COM6 with your board port.
& $env:USERPROFILE\.platformio\penv\Scripts\pio.exe run -t upload -e nano168_oled --upload-port COM6
```

### I2C address / reset pin overrides (optional)

Defaults live in `include/DisplayConfig.h`:
- `OLED_DEFAULT_I2C_ADDRESS` defaults to `0x3C`
- `OLED_RESET_PIN` defaults to `0xFF` (disabled)

To override for a specific environment, add build flags in `platformio.ini`, for example:

```ini
build_flags =
  -DDISPLAY_BACKEND=OLED
  -DOLED_DEFAULT_I2C_ADDRESS=0x3D
  -DOLED_RESET_PIN=A3
```

## LCDproc Configuration

ArduLCDpp speaks LCDproc's `los-panel` protocol over serial, so you can keep using the `hd44780` driver with `ConnectionType=los-panel`.

Minimum settings (adapt from `resources/LCDd.conf`):

```ini
[hd44780]
ConnectionType=los-panel
Device=/dev/ttyUSB0
Speed=57600
Size=20x4
```

Notes:
- `Size` must match the firmware geometry (`LCDW`/`LCDH` in `include/DisplayConfig.h`).
- On Windows, use the `Device=COM6` style in your host tooling, but LCDproc itself is typically Linux-based.

## Behavioral Notes / Limitations
- HD44780 command translation is best-effort: clear/home/cursor addressing and CGRAM uploads are supported; some less common opcodes (e.g., display shift) may be ignored.
- On the Nano ATmega168, unpaced host bursts (T4/T8) can require "burst-safe" behavior: in `nano168_dual_serial` the firmware may defer visible updates during the burst and then catch up once RX goes idle. See `docs/display_smoke_tests.md` and `AGENT_STORE/FEATURES/FEATURE-20260107-explicit-streaming-ux-mode.md`.
- Backlight bytes (`0xFD <level>`) map to SSD1306 contrast. Non-zero values are clamped to a visible floor so the OLED doesn't appear "off" when the firmware uses a very low LCD startup PWM value. Override via `OLED_BRIGHTNESS_MIN` / `OLED_BRIGHTNESS_MAX` in `platformio.ini`.

## Troubleshooting
- If the OLED never shows the boot banner in dual mode, verify:
  - OLED wiring (A4/A5) and I2C address.
  - The Nano target matches your hardware (`nano168_*` vs `nano_*`).
  - The OLED module power level (3.3V vs 5V).
- Run the smoke plan (`docs/display_smoke_tests.md`) after wiring changes to confirm parity across HD44780/OLED/Dual.
