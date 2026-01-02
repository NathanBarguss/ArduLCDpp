#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include <DisplayConfig.h>

#include "display/display_factory.h"

#if DISPLAY_BACKEND != HD44780
#error "Only the HD44780 backend is implemented; update the firmware before selecting another backend."
#endif

byte cmd; //will hold our sent command

static IDisplay &display = getDisplay();
static bool host_active = false;
static bool startup_screen_visible = false;

static void write_centered_line(const char *text, uint8_t row) {
	if (!text || row >= LCDH) {
		return;
	}

	const size_t len = strlen(text);
	const size_t slice = len > LCDW ? LCDW : len;
	char buffer[LCDW + 1];
	memcpy(buffer, text, slice);
	buffer[slice] = '\0';

	uint8_t column = 0;
	if (slice < LCDW) {
		column = static_cast<uint8_t>((LCDW - slice) / 2);
	}
	display.setCursor(column, row);
	display.write(buffer);
}

static void display_startup_screen() {
	startup_screen_visible = true;
	display.clear();
	display.home();
	write_centered_line("ArduLCD Ready", 0);
	if (LCDH > 1) {
		write_centered_line("Waiting for host...", 1);
	}
}

static void dismiss_startup_screen() {
	if (!startup_screen_visible) {
		return;
	}
	startup_screen_visible = false;
	display.clear();
	display.home();
}

void setup() {
	// set up the LCD's number of columns and rows:
	display.begin(LCDW, LCDH);
	display.setBacklight(STARTUP_BRIGHTNESS);
	// set up serial
	Serial.begin(BAUDRATE);
	display.display();
	display_startup_screen();

}

int serial_read() {
	int result = -1;
	while(result == -1) {
		if(Serial.available() > 0) {
			result = Serial.read();
		}
	}
	return result;
}

/**
 * From : https://github.com/lcdproc/lcdproc/blob/master/server/drivers.c
 * All the serial connections supported by driver
 *

static const struct hd44780_SerialInterface serial_interfaces[] = {
//        type                  instr ms  data     v     ^   pre bitrate bits  K   esc   cmd  B  Besc  Boff   Bon Multi  End
	{ HD44780_CT_PICANLCD,      0x11,  0, 0x12, 0x00, 0x1F,    0,   9600,   8, 0, 0x00,    0, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LCDSERIALIZER, 0xFE,  0,    0, 0x00, 0x00,    0,   9600,   8, 0, 0x00,    0, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LOS_PANEL,     0xFE,  0,    0, 0x00, 0x00,    0,   9600,   4, 1, 0xFE,    0, 1, 0xFD,    0, 0xFF,   0,    0 },
	{ HD44780_CT_VDR_LCD,       0xFE,  0,    0, 0x00, 0x00,    0,   9600,   4, 0, 0x00,    0, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_VDR_WAKEUP,    0xC0,  0, 0xC4, 0xC0, 0xCF,    0,   9600,   4, 0, 0x00,    0, 1,    0, 0xC9, 0xC8,   1, 0xCF },
	{ HD44780_CT_PERTELIAN,     0xFE,  0,    0, 0x00, 0x00,    0,   9600,   8, 0, 0x00,    0, 1, 0xFE, 0x02, 0x03,   0,    0 },
	{ HD44780_CT_EZIO,          0xFE, 40,    0, 0x00, 0x00, 0x28,   2400,   4, 1, 0xFD, 0x06, 0,    0,    0,    0,   0,    0 }
};

Additional resources:

https://github.com/lcdproc/lcdproc/blob/master/server/drivers/hd44780-serial.c
https://lcdproc.sourceforge.net/docs/lcdproc-0-5-6-user.html#los-panel
*/

void loop() {
	cmd = serial_read();
	if (!host_active) {
		host_active = true;
		dismiss_startup_screen();
	}
	switch(cmd) {
			case 0xFE:
				display.command(serial_read());
				break;
			case 0xFD:
				// backlight control
				display.setBacklight(serial_read());
				break;
			default:
				// By default we write to the LCD
				display.write(cmd);
				break;
	}
}
