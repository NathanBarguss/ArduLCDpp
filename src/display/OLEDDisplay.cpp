#include "display/OLEDDisplay.h"

OLEDDisplay::OLEDDisplay(uint8_t resetPin, uint8_t i2cAddress)
    : oled_(resetPin), i2cAddress_(i2cAddress) {}

void OLEDDisplay::begin(uint8_t width, uint8_t height) {
	oled_.SetAddress(i2cAddress_);
	oled_.begin(width, height);
	oled_.home();
	oled_.write("hello");
}

void OLEDDisplay::clear() {
	oled_.clear();
}

void OLEDDisplay::home() {
	oled_.home();
}

void OLEDDisplay::display() {
	oled_.display();
}

void OLEDDisplay::setCursor(uint8_t column, uint8_t row) {
	oled_.setCursor(column, row);
}

size_t OLEDDisplay::write(uint8_t value) {
	return oled_.write(value);
}

void OLEDDisplay::createChar(uint8_t slot, const uint8_t bitmap[8]) {
	oled_.createChar(slot, const_cast<uint8_t *>(bitmap));
}

void OLEDDisplay::command(uint8_t value) {
	// TODO(FEATURE-20251223-oled-command-translator): translate HD44780 commands.
	(void)value;
}

void OLEDDisplay::setBacklight(uint8_t level) {
	oled_.SetBrightness(level);
}

