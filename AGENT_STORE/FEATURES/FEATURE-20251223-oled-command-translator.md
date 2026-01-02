# Goal
Handle `0xFE` command bytes for the OLED backend by translating HD44780 opcodes into lcd2oled calls, ensuring los-panel semantics remain intact even though lcd2oled lacks `command()`.

# Background
LiquidCrystal exposes `command(byte)`; lcd2oled does not. Today ArduLCDpp forwards every `0xFE <byte>` pair directly to `lcd.command()`, giving LCDproc raw control over cursor moves, DDRAM addressing, clears, and CGRAM writes. Without a translation layer, the OLED backend would ignore most control traffic.

# Requirements
- Intercept `0xFE` sequences when the OLED backend is active and decode key HD44780 opcodes (clear/home, display control, DDRAM and CGRAM addressing, entry mode, etc.).
- Map decoded operations to the appropriate `IDisplay` methods (`clear`, `home`, `setCursor`, `createChar`, `display`/`noDisplay`, cursor/blink toggles as best-effort).
- Maintain a state machine that tracks whether incoming data bytes should be treated as DDRAM writes or CGRAM bitmap bytes.
- Leave the HD44780 backend untouched (still uses LiquidCrystal::command).

# Dependencies
- Display interface abstraction
- OLED backend implementation hooks to call the translator when selected

# Implementation Plan
1. Introduce a `Hd44780CommandTranslator` that consumes bytes and emits calls on an `IDisplay`+backlight helper.
2. Implement opcode handling for: `0x01` clear, `0x02` home, `0x04-0x07` entry mode (record increment/decrement + shift flag), `0x08-0x0F` display/cursor/blink, `0x10-0x1F` cursor/display shift (MVP: ignore or log), `0x20-0x3F` function set (track 4/8-bit? optional), `0x40-0x7F` CGRAM address, `0x80-0xFF` DDRAM address.
3. Expose hooks so the translator can request `setCursor` or enter CGRAM-write mode as needed. When not in CGRAM mode, subsequent non-command bytes should remain printable data passed to `write`.
4. Write unit-style tests (or a simulator sketch) that feeds known command sequences (clear, set cursor, custom char writes) and asserts the translator emits the right method calls.

# Acceptance Criteria
- OLED builds accept LCDproc traffic without needing lcd2oled::command; cursor positioning and clears behave like HD44780 builds.
- Regression tests or logs show the translator receiving well-formed opcodes from LCDproc demo screens.
- HD44780 backend path is unchanged.

# Validation Notes
- Capture serial traces while running LCDproc with both backends and compare visual output.
- Add tracing (guarded by `#ifdef`) to confirm rarely used opcodes are either supported or safely ignored.
