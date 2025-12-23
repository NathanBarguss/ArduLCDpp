#pragma once

#include <stddef.h>
#include <stdint.h>

class IDisplay {
public:
	virtual ~IDisplay() = default;

	virtual void begin(uint8_t width, uint8_t height) = 0;
	virtual void clear() = 0;
	virtual void home() = 0;
	virtual void display() = 0;
	virtual void setCursor(uint8_t column, uint8_t row) = 0;
	virtual size_t write(uint8_t value) = 0;
	virtual void createChar(uint8_t slot, const uint8_t bitmap[8]) = 0;
	virtual void command(uint8_t value) = 0;
	virtual void setBacklight(uint8_t level) { (void)level; }

	// Helper for writing null-terminated strings without duplicating code in callers.
	virtual size_t write(const char *str) {
		if (!str) {
			return 0;
		}

		size_t written = 0;
		while (*str) {
			written += write(static_cast<uint8_t>(*str++));
		}
		return written;
	}
};
