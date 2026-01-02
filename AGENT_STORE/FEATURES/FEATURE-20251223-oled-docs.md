# Goal
Document the OLED wiring, firmware build flags, and LCDproc configuration so new users can reproduce the 16x4 OLED setup end-to-end.

# Background
The "product" is a los-panel bridge, so documentation is part of the user experience. Contributors need a single source of truth covering hardware wiring, compile-time switches, and LCDd.conf settings.

# Requirements
- Create `/docs/oled_i2c_setup.md` describing wiring (SDA/SCL/reset/power), required components, and photos or diagrams if available.
- Include instructions for compiling the firmware with `DISPLAY_BACKEND=OLED`, plus macros like `OLED_RESET_PIN` and `OLED_I2C_ADDR`.
- Document the HD44780 command translation differences (e.g., blink is best-effort, CGRAM writes are emulated) so users know which los-panel features behave differently.
- Provide an LCDd.conf snippet showing Width 16, Height 4, and any los-panel tuning.
- List current limitations (e.g., cursor blink best-effort, slower scrolling) so expectations are clear.

# Dependencies
- OLED backend feature completed so docs reflect actual behavior.
- Confirmed pin assignments/addresses from implementation.

# Implementation Plan
1. Draft `/docs/oled_i2c_setup.md` with wiring tables, build command examples, and configuration snippets.
2. Update README or AGENT_STORE references to link to the new doc for visibility.
3. Add troubleshooting notes (e.g., blank screen causes) based on early testing.

# Acceptance Criteria
- A new developer following the doc can wire the OLED, build firmware, and display LCDproc output within ~30 minutes.
- Document includes compile-time flag instructions and LCDd.conf example.
- Limitations/known issues are clearly called out.

# Validation Notes
- Have a teammate (or future self) run through the instructions once to confirm no missing steps.
