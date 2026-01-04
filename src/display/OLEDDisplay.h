#pragma once

#include <Arduino.h>
#include <lcd2oled.h>

#include <DisplayConfig.h>

#include "display/IDisplay.h"

class OLEDDisplay : public IDisplay {
public:
	explicit OLEDDisplay(uint8_t resetPin = OLED_RESET_PIN,
	                     uint8_t i2cAddress = OLED_DEFAULT_I2C_ADDRESS);

	void begin(uint8_t width, uint8_t height) override;
	void clear() override;
	void home() override;
	void display() override;
	void setCursor(uint8_t column, uint8_t row) override;
	size_t write(uint8_t value) override;
	void createChar(uint8_t slot, const uint8_t bitmap[8]) override;
	void command(uint8_t value) override;
	void setBacklight(uint8_t level) override;

private:
	lcd2oled oled_;
	uint8_t i2cAddress_;
};

