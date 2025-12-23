#include "display/HD44780Display.h"

HD44780Display::HD44780Display(uint8_t rs, uint8_t enable,
                               uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                               uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                               uint8_t backlightPin)
    : lcd_(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7), backlightPin_(backlightPin) {}

void HD44780Display::begin(uint8_t width, uint8_t height) {
	pinMode(backlightPin_, OUTPUT);
	lcd_.begin(width, height);
}

void HD44780Display::clear() {
	lcd_.clear();
}

void HD44780Display::home() {
	lcd_.home();
}

void HD44780Display::display() {
	lcd_.display();
}

void HD44780Display::setCursor(uint8_t column, uint8_t row) {
	lcd_.setCursor(column, row);
}

size_t HD44780Display::write(uint8_t value) {
	return lcd_.write(value);
}

size_t HD44780Display::write(const char *str) {
	if (!str) {
		return 0;
	}
	return lcd_.write(str);
}

void HD44780Display::createChar(uint8_t slot, const uint8_t bitmap[8]) {
	// LiquidCrystal expects a mutable pointer, so the const_cast is safe because the API never mutates the data.
	lcd_.createChar(slot, const_cast<uint8_t *>(bitmap));
}

void HD44780Display::command(uint8_t value) {
	lcd_.command(value);
}

void HD44780Display::setBacklight(uint8_t level) {
	analogWrite(backlightPin_, level);
}
