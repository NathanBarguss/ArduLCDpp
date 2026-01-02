#pragma once

#define LED_PIN 11           // PWM pin driving the LCD backlight (matches Nano wiring)
#define STARTUP_BRIGHTNESS 2 // Initial duty cycle for the backlight
#define BAUDRATE 57600       // Serial baud rate used for LCDproc bridge
#define LCDW 20              // LCD column count
#define LCDH 4               // LCD row count

#define HD44780 0
#define OLED 1

#ifndef DISPLAY_BACKEND
#define DISPLAY_BACKEND HD44780
#endif
