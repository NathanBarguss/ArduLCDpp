#pragma once

#define LED_PIN 11           // PWM pin driving the LCD backlight (matches Nano wiring)
#define STARTUP_BRIGHTNESS 2 // Initial duty cycle for the backlight
#define BAUDRATE 57600       // Serial baud rate used for LCDproc bridge
#define LCDW 20              // LCD column count
#define LCDH 4               // LCD row count

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

#ifndef OLED_RESET_PIN
#define OLED_RESET_PIN 0xFF
#endif

#ifndef OLED_DEFAULT_I2C_ADDRESS
#define OLED_DEFAULT_I2C_ADDRESS 0x3C
#endif

#define HD44780 0
#define OLED 1
#define DUAL 2

#ifndef DISPLAY_BACKEND
#define DISPLAY_BACKEND HD44780
#endif
