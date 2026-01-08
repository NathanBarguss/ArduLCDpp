#include "display/OLEDDisplay.h"

OLEDDisplay::OLEDDisplay(uint8_t resetPin, uint8_t i2cAddress)
    : oled_(resetPin), i2cAddress_(i2cAddress) {}

uint8_t OLEDDisplay::clampColumn(uint8_t column) const {
	if (columns_ == 0) {
		return 0;
	}
	if (column >= columns_) {
		return static_cast<uint8_t>(columns_ - 1);
	}
	return column;
}

uint8_t OLEDDisplay::clampRow(uint8_t row) const {
	if (rows_ == 0) {
		return 0;
	}
	if (row >= rows_) {
		return static_cast<uint8_t>(rows_ - 1);
	}
	return row;
}

void OLEDDisplay::begin(uint8_t width, uint8_t height) {
	Serial.println(F("oled: begin entry"));
	oled_.SetAddress(i2cAddress_);
	Serial.println(F("oled: address set"));
	columns_ = width;
	rows_ = height;
	oled_.begin(columns_, rows_);
	Serial.println(F("oled: driver begin done"));
	oled_.home();
	Serial.println(F("oled: home done"));
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
	oled_.setCursor(clampColumn(column), clampRow(row));
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
	if (level == 0) {
		oled_.SetBrightness(0);
		return;
	}

	const uint8_t minBrightness = static_cast<uint8_t>(OLED_BRIGHTNESS_MIN);
	const uint8_t maxBrightness = static_cast<uint8_t>(OLED_BRIGHTNESS_MAX);

	if (maxBrightness <= minBrightness) {
		oled_.SetBrightness(maxBrightness);
		return;
	}

	const uint16_t span = static_cast<uint16_t>(maxBrightness - minBrightness);
	const uint16_t scaled = static_cast<uint16_t>(minBrightness) +
	                        (static_cast<uint16_t>(level) * span) / 255U;
	oled_.SetBrightness(static_cast<uint8_t>(scaled));
}
