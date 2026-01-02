# AGENTS

## Interaction Protocol
- At the start of every chat, ask: "Which role should I operate as: DEV, QA, or UX?" and wait for the answer before proceeding.
- Once the role is selected, note it explicitly (e.g., "Active role: DEV") and keep that perspective until the user changes it.
- If the user doesn't respond with a valid role, prompt again with the available options.

## Workspace Bootstrap
1. Verify the `AGENT_STORE/` hierarchy exists at the repo root. This repo intentionally mimics a lightweight JIRA: the folder tree is the shared backlog that every agent can read without external services. If any folder is missing, create it using the list below so automation and humans have the same structure across machines:
   - `AGENT_STORE/README.md`
   - `AGENT_STORE/RESEARCH/README.md`
   - `AGENT_STORE/BUGS/README.md` plus `AGENT_STORE/BUGS/RESOLVED/README.md`
   - `AGENT_STORE/FEATURES/README.md` plus `AGENT_STORE/FEATURES/RESOLVED/README.md`
1. Populate each README with the purpose, naming pattern `TYPE-YYYYMMDD-short-slug.md`, and the markdown headers defined later in this document. That way, a brand-new contributor can open any README and immediately know how to author a compliant entry without rereading `AGENTS.MD`.
1. Confirm the workspace has write access so you can add or move bug, feature, or research files. The workflow requires creating new markdown artifacts and moving them into `RESOLVED/` archives; lack of write access blocks that process.
1. Sync git submodules after cloning: `git submodule update --init --recursive`. The `lcd2oled` display helper library lives in `lib/lcd2oled/` as a submodule (source: https://github.com/NathanBarguss/lcd2oled) and must be present for OLED work.
1. If you are unsure what to place inside a README, mirror the templates defined below:
   - `AGENT_STORE/README.md` should explain the overall backlog concept, list each subfolder, and describe the workflow checklist for adding and resolving items.
   - `AGENT_STORE/RESEARCH/README.md` must include the research template headers (Objective, Key Findings, Sources, Open Questions, Recommended Next Steps) so exploratory work remains actionable.
   - `AGENT_STORE/BUGS/README.md` must spell out the defect template (Summary, Environment, Steps to Reproduce, Expected/Actual Results, Fix Plan, Verification Notes) and remind agents to move closed items to `BUGS/RESOLVED/`.
   - `AGENT_STORE/FEATURES/README.md` must document the feature template (Goal, Background, Requirements, Dependencies, Implementation Plan, Acceptance Criteria, Validation Notes) and direct agents to archive finished specs under `FEATURES/RESOLVED/`.
   - Both `RESOLVED/README.md` files only need a short note explaining that they are archival stores for completed work; no additional template content is required.

## Role Glossary
- **DEV** - Implementation-focused engineer handling code, integration, and refactors.
- **QA** - Tester responsible for verification, regression checks, and quality gates.
- **UX** - Designer concentrating on user flows, visual decisions, and interaction polish.

## Hardware Prototype Reference
- **MCU:** Arduino Nano (ATmega328P by default; ATmega168 board also present in the lab). Always confirm with the user which Nano/bootloader is connected before flashing so the correct PlatformIO environment is selected. Current environments:
  - `uno_hd44780` / `mega2560_hd44780` for dev builds.
  - `nano_hd44780` for ATmega328P Nanos (new bootloader).
  - `nano168_hd44780` for ATmega168 units—use this when uploads only succeed after switching boards in PlatformIO.
- **HD44780 character LCD:** 8-bit parallel, RW tied low, backlight driven through a BC337 low-side transistor from Arduino D11 (PWM). See pin map below.
- **OLED fallback:** SSD1306 I²C module (128×32 tested, 128×64 expected). Connected via I²C plus optional reset.
- **Display selection:** Only one display is active per firmware build (compile-time flag); both panels can remain physically attached.
- **Power/decoupling:** LCD logic runs off Nano 5 V pin with 100 nF decoupling near the module and an optional 10 µF bulk capacitor. OLED power depends on the breakout (5 V or 3.3 V).
- **Design assumptions:** The HD44780 busy flag is never read (write-only workflow), and USB provides the development power source.

### HD44780 Wiring (8-bit mode, write-only)

| Arduino Nano | LCD Pin | Signal        |
|--------------|---------|---------------|
| D12          | 4       | RS            |
| GND          | 5       | RW (tied low) |
| D2           | 6       | E             |
| D3           | 7       | D0            |
| D4           | 8       | D1            |
| D5           | 9       | D2            |
| D6           | 10      | D3            |
| D7           | 11      | D4            |
| D8           | 12      | D5            |
| D9           | 13      | D6            |
| D10          | 14      | D7            |
| 5 V          | 2       | VDD           |
| GND          | 1       | VSS           |
| Pot wiper    | 3       | VO (contrast) |

> **CRITICAL:** Always tweak the VO contrast trimmer when a “blank” screen is reported. The firmware may be running fine, but without the correct contrast level the welcome banner is invisible. Check contrast before assuming a hardware or code fault.

**Backlight wiring**
- LCD LED+ (pin 15) → +5 V
- LCD LED− (pin 16) → BC337 collector
- BC337 emitter → GND
- Arduino D11 (PWM) → BC337 base via ~1 kΩ–4.7 kΩ resistor

### SSD1306 OLED Wiring (I²C)

| Arduino Nano | OLED Pin |
|--------------|----------|
| A4           | SDA      |
| A5           | SCL      |
| A3           | RESET (optional) |
| 5 V / 3.3 V  | VCC (module-dependent) |
| GND          | GND      |

> Note: Most SSD1306 breakout boards are 5 V tolerant, but confirm the specific module before powering from 5 V.

**Hardware change protocol**
- Before running hardware tests or smoke suites, ask the user which board is attached (Uno, Nano 328, Nano 168, etc.) and update the PlatformIO environment accordingly.
- If the connected board differs from the previous session, note the change in your status update so other agents know which `platformio.ini` env to use and why (e.g., “Nano168 prototype requires `nano168_hd44780` because uploads to 328 targets fail to sync”).
