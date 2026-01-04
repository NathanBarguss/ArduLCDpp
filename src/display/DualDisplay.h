#pragma once

#include "display/IDisplay.h"

class DualDisplay : public IDisplay {
public:
	DualDisplay(IDisplay &primary, IDisplay &secondary);

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
	IDisplay &primary_;
	IDisplay &secondary_;
};

