#pragma once

#define LED_PIN 11           // PWM pin driving the LCD backlight (matches Nano wiring)
#define STARTUP_BRIGHTNESS 2 // Initial duty cycle for the backlight
#define BAUDRATE 57600       // Serial baud rate used for LCDproc bridge
#define LCDW 20              // LCD column count
#define LCDH 4               // LCD row count

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

#ifndef ENABLE_VERBOSE_DEBUG_LOGS
#define ENABLE_VERBOSE_DEBUG_LOGS 0
#endif

#ifndef ENABLE_DUAL_DEBUG
#define ENABLE_DUAL_DEBUG 0
#endif

#ifndef ENABLE_LCD2OLED_DEBUG
#define ENABLE_LCD2OLED_DEBUG 0
#endif

#ifndef ENABLE_DUAL_QUEUE
#define ENABLE_DUAL_QUEUE 0
#endif

#ifndef OLED_RESET_PIN
#define OLED_RESET_PIN 0xFF
#endif

#ifndef OLED_DEFAULT_I2C_ADDRESS
#define OLED_DEFAULT_I2C_ADDRESS 0x3C
#endif

// OLED "backlight" mapping (SSD1306 contrast).
// The los-panel protocol uses 0..255 backlight bytes; on OLED we map those to
// contrast. A very low STARTUP_BRIGHTNESS (tuned for an LCD backlight PWM pin)
// can make the OLED appear completely off, so we apply a visible floor for any
// non-zero level. Override per-environment via `platformio.ini` build_flags.
#ifndef OLED_BRIGHTNESS_MIN
#define OLED_BRIGHTNESS_MIN 32
#endif

#ifndef OLED_BRIGHTNESS_MAX
#define OLED_BRIGHTNESS_MAX 255
#endif

#define HD44780 0
#define OLED 1
#define DUAL 2

#ifndef DISPLAY_BACKEND
#define DISPLAY_BACKEND HD44780
#endif
