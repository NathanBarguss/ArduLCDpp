# ArduLCDpp

ArduLCDpp is our actively maintained fork of the classic ArduLCD USB bridge. We pair an Arduino Nano with an HD44780 (and future OLED) display, speak lcdproc’s los-panel protocol, and keep the firmware modernized with PlatformIO, automated smoke tests, and a shared backlog under `AGENT_STORE/`.

This README focuses on the current bench-proven hardware, how to build and test the firmware, and how contributors can sync up with the work queue.

---

## Project Focus
- **Healthy "power-on" UX:** Every boot shows a deterministic startup banner (`ArduLCDpp Ready / Waiting for host... / bit.ly/4plthUv*`) until the first serial byte arrives. *Third line appears on displays with three or more rows.*
- **Modern toolchain:** PlatformIO drives all builds (`src/main.cpp` fed through `src/display/IDisplay.h`), with formal smoke tests documented in `docs/display_smoke_tests.md`.
- **Transparent backlog:** Specs, bugs, and research live in `AGENT_STORE/`, mirroring a lightweight JIRA so agents and automation stay aligned.
- **Dual-display parity option:** Beyond the LCD-only/OLED-only binaries, we now support a dual build that drives both panels simultaneously so UX can compare rendering without reflashing; update older specs if they still insist on mutually exclusive builds.

---

## Hardware Snapshot (2026 refresh)
| Signal                | LCD Pin | Nano Pin |
|-----------------------|---------|----------|
| RS                    | 4       | D12      |
| E                     | 6       | D2       |
| D0 … D7               | 7–14    | D3–D10   |
| Backlight (PWM)       | 16      | D11 (via BC337 low-side switch) |
| RW                    | 5       | Tied low |
| VSS / VDD / VO        | 1 / 2 / 3 | GND / +5 V / contrast pot |

**Contrast is critical:** If the panel looks blank, adjust the VO trimmer before assuming a firmware fault.

**Schematics:** `resources/wiring_schematic.sch` and `.png` still show the legacy wiring. Use the table above until `FEATURE-20260102-refresh-schematics` lands.

---

## Repo Layout
- `src/` – Firmware (`main.cpp`, display abstraction, HD44780 driver)
- `include/DisplayConfig.h` – Dimensions, baud rate, LED pin, backend selectors
- `docs/` – Smoke tests, lcdproc mapping, helper guides
- `AGENT_STORE/` – Backlog (FEATURES, BUGS, RESEARCH + RESOLVED archives)
- `lib/lcd2oled/` – Submodule used by upcoming OLED backend
- `resources/` – Schematics, wiring photos, lcdproc sample config

---

## Getting Started
```powershell
git clone --recursive https://github.com/NathanBarguss/ArduLCDpp.git
cd ArduLCDpp
git submodule update --init --recursive
```

We build with PlatformIO (VS Code extension or CLI). On Windows the CLI lives under `%USERPROFILE%\.platformio\penv\Scripts\pio.exe`.

---

## Building & Uploading (PlatformIO)
| Environment          | Board / Notes                                  |
|----------------------|-----------------------------------------------|
| `uno_hd44780`        | Arduino Uno reference build (default)         |
| `mega2560_hd44780`   | Arduino Mega 2560                             |
| `nano_hd44780`       | Nano ATmega328P (new bootloader)              |
| `nano168_hd44780`    | Nano ATmega168 (lab hardware)                 |

```powershell
# Build default environment
& $env:USERPROFILE\.platformio\penv\Scripts\pio.exe run

# Build/upload Nano168 (bench hardware on COM6)
& $env:USERPROFILE\.platformio\penv\Scripts\pio.exe run -t upload -e nano168_hd44780 --upload-port COM6
```

**Serial monitors reset the Nano:** Wait 2–3 seconds after opening a port before sending bytes, and keep the connection open a few seconds so the LCD state is observable. This applies to the manual smoke scripts and any host tooling.

---

## Firmware Behavior
- `display_startup_screen()` centers the banner and holds it until `serial_read()` receives the first byte.
- Host commands mirror lcdproc’s los-panel driver: `0xFE` for commands, `0xFD` for backlight, raw ASCII otherwise.
- Backlight PWM currently maps duty cycle directly to `analogWrite(D11, level)`. `FEATURE-20260102-backlight-calibration` tracks improvements so `FD 00/80/FF` give wider visual spread.

## Display Modes
- `DISPLAY_BACKEND=HD44780` remains the default LCD-only build for production installs.
- `DISPLAY_BACKEND=OLED` targets the SSD1306 bridge once the OLED backend tickets close.
- `DISPLAY_BACKEND=DUAL` (FEATURE-20260104-dual-display-parity) mirrors every `IDisplay` call to both panels so UX/QA can compare glyphs live; keep both displays wired and confirm power budget before long bench sessions.

Any feature documentation authored before 2026-01-04 assumed mutually exclusive builds—update those tickets when you touch them so they explicitly account for the dual option.

---

## Manual Smoke Tests
All benches should run the checklist in `docs/display_smoke_tests.md`. Highlights:

1. **T1 – Power-on banner:** Ensure the welcome message appears before host traffic.
2. **T2–T6 – Core protocol:** Clear/home, address sweep, full-screen fill, CGRAM uploads (remember `FE 80` before writing glyph slots), and PWM backlight control.
3. **T7 – USB reconnect:** Mark as N/A when USB is also power; requires external supply or automation hook.
4. **T8 – Stress burst:** 1 KB mixed payload to confirm no hangs or corruption.

Capture PASS/FAIL in commits or AGENT_STORE entries so everyone knows which hardware was exercised.

---

## Backlog & Collaboration
- New specs/bugs/research live under `AGENT_STORE/` using the template naming scheme (`TYPE-YYYYMMDD-slug.md`).
- When editing feature specs created before 2026-01-04, revise any “single display only” assumptions so the new dual-mode plan stays consistent throughout the backlog.
- Keep tickets updated as you work, and move files into the relevant `RESOLVED/` folder once merged. Example: `FEATURE-20251223-startup-screen` documents the commit IDs that delivered the banner.
- When in doubt, load `AGENT_STORE/README.md` for workflow and template guidance.

Active “next up” items (see `AGENT_STORE/FEATURES/PRIORITY.md`):
1. Finalize OLED backend layers (command translator, CGRAM shim, geometry clamp).
2. Land the dual-display parity build (FEATURE-20260104-dual-display-parity) once both concrete backends are solid.
3. Extend the compile-time selector + docs so LCD, OLED, and dual builds are all first-class (ticket updates required if they still assume mutually exclusive outputs).
4. Deliver automated smoke-test harness so manual serial poking isn’t the bottleneck.
5. Refresh schematics/backlight calibration per the new hardware reality.

---

## Resources
- `docs/lcdproc_display_mapping.md` – Byte-level mapping between los-panel commands and firmware actions.
- `docs/display_smoke_tests.md` – Repro scripts for T1–T8 scenarios.
- `resources/LCDd.conf` – Sample lcdproc configuration targeting this firmware.
- Photo references live under `resources/` for enclosure ideas.

Questions? Open an issue/feature ticket in `AGENT_STORE/`, mention which hardware you tested on (Uno, Nano 328, Nano 168), and link relevant smoke-test runs. Contributions that keep the backlog updated and the manual tests green are easiest to review and merge.
