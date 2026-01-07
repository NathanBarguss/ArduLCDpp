#pragma once

#include <DisplayConfig.h>

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
	void pumpSecondary(uint8_t maxOps = 1);
	size_t pendingSecondaryWrites() const;
	void setQueueingEnabled(bool enabled);

private:
	IDisplay &primary_;
	IDisplay &secondary_;
#if ENABLE_DUAL_QUEUE
	uint8_t width_ = 0;
	uint8_t height_ = 0;
	uint8_t cursor_column_ = 0;
	uint8_t cursor_row_ = 0;
	uint32_t last_write_micros_ = 0;

	// OLED updates are deferred while the host is active to avoid I2C writes
	// blocking the UART receiver during bursts. We maintain a tiny shadow of the
	// HD44780-visible text and refresh dirty rows during idle time.
	uint8_t dirty_rows_mask_ = 0;
	char shadow_[LCDW * LCDH];

	bool queue_enabled_;
#endif
};
