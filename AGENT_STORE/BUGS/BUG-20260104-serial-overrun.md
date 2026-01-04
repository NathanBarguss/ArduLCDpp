# Summary
Translator-driven dual/OLED builds miss the last row during bulk writes because the UART RX buffer overruns when 80 bytes arrive without pacing.

# Environment
- Firmware: `feature/FEATURE-20251223-oled-backend` (nano168_dual env)
- Hardware: Bench Nano ATmega168 with HD44780 + SSD1306 connected
- Host tooling: Manual los-panel smoke scripts via Python/pyserial on COM6 @ 57,600 bps

# Steps to Reproduce
1. Reset the Nano (opening the serial port resets automatically). Wait 2-3 seconds for the splash screen.
2. Send `FE 01`, `FE 02`, followed immediately by 80 data bytes (e.g., `0x5A` repeated) without any delay between bytes.
3. Close the serial port as soon as the bytes are transmitted.

# Expected Results
- All four rows fill with the streamed character (row order 0,1,2,3) on both LCD and OLED, matching HD44780 behavior.

# Actual Results
- Rows 0-2 fill correctly, but the fourth row remains blank on both displays.
- If the same payload is throttled with ~10 ms between bytes—or the host keeps the port open a few seconds after writing—the full screen renders, indicating the device never received the final bytes during the fast burst.

# Fix Plan
- Reduce per-byte overhead in `Hd44780CommandTranslator::handleData` so sequential DDRAM writes do not call `setCursor` on every character. Track whether the next byte is contiguous and skip redundant cursor updates to keep up with incoming serial traffic.
- Consider buffering host bytes or reintroducing a small host-side delay only as a temporary workaround; the long-term solution should keep pace with lcdproc output without special pacing requirements.

# Verification Notes
- Re-run T4 from `docs/display_smoke_tests.md` at full speed after the translator change; confirm all four rows fill without adding host delays.
- Capture the same test on both displays (photos acceptable) and note the PASS result in `FEATURE-20260104-dual-display-parity.md`.
