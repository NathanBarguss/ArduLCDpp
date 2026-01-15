#pragma once

#include <stdint.h>

#include "display/IDisplay.h"

class MatrixOrbitalParser {
public:
	explicit MatrixOrbitalParser(IDisplay &display)
	    : display_(display) {}

	void reset() {
		// No internal state in the MVP parser; reserved for future extensions.
	}

	void handleByte(uint8_t value, uint8_t (*read_byte_blocking)()) {
		if (value != 0xFE) {
			display_.write(value);
			return;
		}

		const uint8_t cmd = read_byte_blocking();
		switch (cmd) {
			case 'X': // clear screen
				display_.clear();
				display_.home();
				break;
			case 'H': // home
				display_.home();
				break;
			case 'G': { // set cursor position (1-based)
				const uint8_t x = read_byte_blocking();
				const uint8_t y = read_byte_blocking();
				uint8_t col = x > 0 ? static_cast<uint8_t>(x - 1) : 0;
				uint8_t row = y > 0 ? static_cast<uint8_t>(y - 1) : 0;
				if (col >= 20) {
					col = 19;
				}
				if (row >= 4) {
					row = 3;
				}
				display_.setCursor(col, row);
				break;
			}
			case 0x4E: { // custom character definition
				const uint8_t slot = read_byte_blocking();
				uint8_t bitmap[8];
				for (uint8_t i = 0; i < 8; ++i) {
					bitmap[i] = read_byte_blocking() & 0x1F;
				}
				if (slot <= 7) {
					display_.createChar(slot, bitmap);
				}
				break;
			}
			case 0x98: { // brightness level (0-255)
				const uint8_t level = read_byte_blocking();
				display_.setBacklight(level);
				break;
			}
			case 'B': { // backlight on (matrix.dll sends param 0)
				(void)read_byte_blocking();
				display_.setBacklight(255);
				break;
			}
			case 'F': // backlight off
				display_.setBacklight(0);
				break;
			default:
				// Ignore unsupported commands in MVP.
				break;
		}
	}

private:
	IDisplay &display_;
};
