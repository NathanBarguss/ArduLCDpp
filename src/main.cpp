#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include <DisplayConfig.h>

#include "display/display_factory.h"

#if DISPLAY_BACKEND != HD44780
#error "Only the HD44780 backend is implemented; update the firmware before selecting another backend."
#endif

byte cmd; //will hold our sent command

static IDisplay &display = getDisplay();

void setup() {
	// set up the LCD's number of columns and rows:
	display.begin(LCDW, LCDH);
	display.setBacklight(STARTUP_BRIGHTNESS);
	// set up serial
	Serial.begin(BAUDRATE);
	display.display();
	display.clear();
	char welcome[LCDW];
	sprintf(welcome, "%dx%d Ready", LCDW, LCDH);
	display.write(welcome);
	display.home();

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
