#pragma once

#include <stdint.h>

#include "display/IDisplay.h"

class Hd44780CommandTranslator {
public:
	explicit Hd44780CommandTranslator(IDisplay &display);

	void reset();
	void handleCommand(uint8_t value);
	// Returns true if the byte was consumed (e.g., CGRAM programming).
	bool handleData(uint8_t value);

private:
	void handleClear();
	void handleHome();
	void handleEntryMode(uint8_t value);
	void handleDisplayControl(uint8_t value);
	void handleFunctionSet(uint8_t value);
	void handleCursorShift(uint8_t value);
	void handleSetCgramAddress(uint8_t value);
	void handleSetDdramAddress(uint8_t value);
	void exitCgramMode();
	bool decodeDdramAddress(uint8_t address, uint8_t &row, uint8_t &column) const;
	uint8_t encodeDdramAddress(uint8_t row, uint8_t column) const;
	void updateCgram(uint8_t value);
	void advanceCgramAddress();
	void advanceDdramAddress();

	IDisplay &display_;
	bool increment_;
	bool shift_on_write_;
	bool display_enabled_;
	bool cursor_enabled_;
	bool blink_enabled_;
	uint8_t ddram_address_;
	bool cgram_active_;
	uint8_t cgram_address_;
	uint8_t cgram_cache_[8][8];
	uint8_t logical_row_;
	uint8_t logical_column_;
};
