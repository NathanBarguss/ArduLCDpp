#include "display/Hd44780CommandTranslator.h"

#include <DisplayConfig.h>
#include <string.h>

Hd44780CommandTranslator::Hd44780CommandTranslator(IDisplay &display)
    : display_(display) {
	reset();
}

void Hd44780CommandTranslator::reset() {
	increment_ = true;
	shift_on_write_ = false;
	display_enabled_ = true;
	cursor_enabled_ = false;
	blink_enabled_ = false;
	ddram_address_ = 0;
	cgram_active_ = false;
	cgram_address_ = 0;
	memset(cgram_cache_, 0, sizeof(cgram_cache_));
	logical_row_ = 0;
	logical_column_ = 0;
}

void Hd44780CommandTranslator::handleCommand(uint8_t value) {
	if (value == 0x01) {
		handleClear();
		return;
	}
	if (value == 0x02) {
		handleHome();
		return;
	}
	if ((value & 0xFC) == 0x04) {
		handleEntryMode(value);
		return;
	}
	if ((value & 0xF8) == 0x08) {
		handleDisplayControl(value);
		return;
	}
	if ((value & 0xF0) == 0x10) {
		handleCursorShift(value);
		return;
	}
	if ((value & 0xE0) == 0x20) {
		handleFunctionSet(value);
		return;
	}
	if (value & 0x80) {
		handleSetDdramAddress(static_cast<uint8_t>(value & 0x7F));
		return;
	}
	if ((value & 0x40) == 0x40) {
		handleSetCgramAddress(static_cast<uint8_t>(value & 0x3F));
		return;
	}
	// Unsupported commands are ignored; LCDproc rarely emits the remaining opcodes.
}

bool Hd44780CommandTranslator::handleData(uint8_t value) {
	if (cgram_active_) {
		updateCgram(static_cast<uint8_t>(value & 0x1F));
		advanceCgramAddress();
		return true;
	}

	display_.setCursor(logical_column_, logical_row_);
	display_.write(value);
	advanceDdramAddress();
	return true;
}

void Hd44780CommandTranslator::handleClear() {
	exitCgramMode();
	display_.clear();
	display_.home();
	ddram_address_ = 0;
	logical_row_ = 0;
	logical_column_ = 0;
}

void Hd44780CommandTranslator::handleHome() {
	exitCgramMode();
	display_.home();
	ddram_address_ = 0;
	logical_row_ = 0;
	logical_column_ = 0;
}

void Hd44780CommandTranslator::handleEntryMode(uint8_t value) {
	increment_ = (value & 0x02) != 0;
	shift_on_write_ = (value & 0x01) != 0;
}

void Hd44780CommandTranslator::handleDisplayControl(uint8_t value) {
	const bool display_on = (value & 0x04) != 0;
	const bool cursor_on = (value & 0x02) != 0;
	const bool blink_on = (value & 0x01) != 0;
	display_enabled_ = display_on;
	cursor_enabled_ = cursor_on;
	blink_enabled_ = blink_on;
	if (display_enabled_) {
		display_.display();
	}
	// Cursor/blink toggles are currently no-ops; add support once IDisplay exposes them.
}

void Hd44780CommandTranslator::handleFunctionSet(uint8_t value) {
	(void)value;
	// LCDproc may tweak DL/N/F bits during init; nothing for us to do with those.
}

void Hd44780CommandTranslator::handleCursorShift(uint8_t value) {
	(void)value;
	// Cursor/display shift instructions would require manual text reflow.
	// For now we ignore them; LCDproc rarely issues these during normal dashboards.
}

void Hd44780CommandTranslator::handleSetCgramAddress(uint8_t value) {
	cgram_active_ = true;
	cgram_address_ = static_cast<uint8_t>(value & 0x3F);
}

void Hd44780CommandTranslator::handleSetDdramAddress(uint8_t value) {
	exitCgramMode();
	ddram_address_ = static_cast<uint8_t>(value & 0x7F);
	uint8_t row = 0;
	uint8_t column = 0;
	if (decodeDdramAddress(ddram_address_, row, column)) {
		logical_row_ = row;
		logical_column_ = column;
		display_.setCursor(column, row);
	}
}

void Hd44780CommandTranslator::exitCgramMode() {
	cgram_active_ = false;
}

bool Hd44780CommandTranslator::decodeDdramAddress(uint8_t address, uint8_t &row, uint8_t &column) const {
	static constexpr uint8_t offsets4[] = {0x00, 0x40, 0x14, 0x54};
	static constexpr uint8_t offsets2[] = {0x00, 0x40};
	static constexpr uint8_t offsets1[] = {0x00};

	const uint8_t *offsets = nullptr;
	uint8_t row_count = 0;

	if (LCDH >= 4) {
		offsets = offsets4;
		row_count = 4;
	} else if (LCDH == 3) {
		offsets = offsets4;
		row_count = 3;
	} else if (LCDH == 2) {
		offsets = offsets2;
		row_count = 2;
	} else {
		offsets = offsets1;
		row_count = 1;
	}

	for (uint8_t r = 0; r < row_count; ++r) {
		const uint8_t offset = offsets[r];
		const uint8_t end = static_cast<uint8_t>(offset + LCDW);
		if (address >= offset && address < end) {
			row = r;
			column = static_cast<uint8_t>(address - offset);
			return true;
		}
	}
	return false;
}

uint8_t Hd44780CommandTranslator::encodeDdramAddress(uint8_t row, uint8_t column) const {
	static constexpr uint8_t offsets4[] = {0x00, 0x40, 0x14, 0x54};
	static constexpr uint8_t offsets2[] = {0x00, 0x40};
	static constexpr uint8_t offsets1[] = {0x00};

	if (column >= LCDW) {
		return 0;
	}

	if (LCDH >= 4) {
		if (row >= 4) {
			row = static_cast<uint8_t>(row % 4);
		}
		return static_cast<uint8_t>(offsets4[row] + column);
	} else if (LCDH == 3) {
		if (row >= 3) {
			row = static_cast<uint8_t>(row % 3);
		}
		return static_cast<uint8_t>(offsets4[row] + column);
	} else if (LCDH == 2) {
		if (row >= 2) {
			row = static_cast<uint8_t>(row % 2);
		}
		return static_cast<uint8_t>(offsets2[row] + column);
	} else {
		return static_cast<uint8_t>(offsets1[0] + column);
	}
}

void Hd44780CommandTranslator::updateCgram(uint8_t value) {
	const uint8_t slot = static_cast<uint8_t>((cgram_address_ >> 3) & 0x07);
	const uint8_t row = static_cast<uint8_t>(cgram_address_ & 0x07);
	cgram_cache_[slot][row] = value;
	display_.createChar(slot, cgram_cache_[slot]);
}

void Hd44780CommandTranslator::advanceCgramAddress() {
	uint8_t next = cgram_address_;
	if (increment_) {
		next = static_cast<uint8_t>((next + 1) & 0x3F);
	} else {
	next = static_cast<uint8_t>((next - 1) & 0x3F);
	}
	cgram_address_ = next;
}

void Hd44780CommandTranslator::advanceDdramAddress() {
	if (!increment_) {
		if (logical_column_ == 0) {
			logical_column_ = static_cast<uint8_t>(LCDW - 1);
			if (logical_row_ == 0) {
				logical_row_ = static_cast<uint8_t>(LCDH - 1);
			} else {
				--logical_row_;
			}
		} else {
			--logical_column_;
		}
		ddram_address_ = encodeDdramAddress(logical_row_, logical_column_);
		return;
	}

	++logical_column_;
	if (logical_column_ >= LCDW) {
		logical_column_ = 0;
		logical_row_ = static_cast<uint8_t>((logical_row_ + 1) % LCDH);
	}
	ddram_address_ = encodeDdramAddress(logical_row_, logical_column_);
}
