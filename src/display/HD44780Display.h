#pragma once

#include <Arduino.h>
#include <LiquidCrystal.h>

#include "display/IDisplay.h"

class HD44780Display : public IDisplay {
public:
	HD44780Display(uint8_t rs, uint8_t enable,
	               uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	               uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	               uint8_t backlightPin);

	void begin(uint8_t width, uint8_t height) override;
	void clear() override;
	void home() override;
	void display() override;
	void setCursor(uint8_t column, uint8_t row) override;
	size_t write(uint8_t value) override;
	size_t write(const char *str) override;
	void createChar(uint8_t slot, const uint8_t bitmap[8]) override;
	void command(uint8_t value) override;
	void setBacklight(uint8_t level) override;

private:
	LiquidCrystal lcd_;
	uint8_t backlightPin_;
};
