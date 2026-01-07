#include "display/DualDisplay.h"

#include <Arduino.h>
#include <DisplayConfig.h>
#include <string.h>

#include "SerialDebug.h"

#if ENABLE_DUAL_DEBUG
#define DUAL_DEBUG(msg) SerialDebug::line(true, F(msg))
#else
#define DUAL_DEBUG(msg) do {} while (0)
#endif

DualDisplay::DualDisplay(IDisplay &primary, IDisplay &secondary)
#if ENABLE_DUAL_QUEUE
    : primary_(primary),
      secondary_(secondary),
      width_(0),
      height_(0),
      cursor_column_(0),
      cursor_row_(0),
      dirty_rows_mask_(0),
      shadow_{0},
      cgram_dirty_mask_(0),
      cgram_shadow_{{0}},
      queue_enabled_(false) {}
#else
    : primary_(primary), secondary_(secondary) {}
#endif

void DualDisplay::begin(uint8_t width, uint8_t height) {
	Serial.print(F("ENABLE_DUAL_DEBUG:"));
	Serial.println(ENABLE_DUAL_DEBUG);

	DUAL_DEBUG("DualDisplay: begin primary");
	primary_.begin(width, height);
	
	DUAL_DEBUG("DualDisplay: begin secondary");
	Serial.println(F("dual: begin secondary start"));

	secondary_.begin(width, height);

	Serial.println(F("dual: begin secondary done"));

#if ENABLE_DUAL_QUEUE
	width_ = width;
	height_ = height;
	cursor_column_ = 0;
	cursor_row_ = 0;
	last_write_micros_ = micros();
	memset(shadow_, ' ', sizeof(shadow_));
	dirty_rows_mask_ = 0;
	cgram_dirty_mask_ = 0;
	memset(cgram_shadow_, 0, sizeof(cgram_shadow_));
#endif

	DUAL_DEBUG("DualDisplay: begin complete");
}

void DualDisplay::clear() {
	DUAL_DEBUG("DualDisplay: clear");
#if ENABLE_DUAL_QUEUE
	cursor_column_ = 0;
	cursor_row_ = 0;
	memset(shadow_, ' ', sizeof(shadow_));
	dirty_rows_mask_ = 0x0F;

	if (!queue_enabled_) {
		primary_.clear();
		secondary_.clear();
	} else {
		// Defer display clears until idle so we don't block incoming serial bursts.
	}
#else
	primary_.clear();
	secondary_.clear();
#endif
#if ENABLE_SERIAL_DEBUG
	if (SerialDebug::isRuntimeEnabled()) {
		SerialDebug::printPrefix();
		Serial.println(F("dual: clear done"));
	}
#endif
}

void DualDisplay::home() {
#if ENABLE_DUAL_QUEUE
	cursor_column_ = 0;
	cursor_row_ = 0;
	if (!queue_enabled_) {
		primary_.home();
		secondary_.home();
	}
#else
	primary_.home();
	secondary_.home();
#endif
}

void DualDisplay::display() {
	primary_.display();
#if ENABLE_DUAL_QUEUE
	if (!queue_enabled_) {
		secondary_.display();
	}
#else
	secondary_.display();
#endif
}

void DualDisplay::setCursor(uint8_t column, uint8_t row) {
#if ENABLE_DUAL_QUEUE
	cursor_column_ = column;
	cursor_row_ = row;
	if (!queue_enabled_) {
		primary_.setCursor(column, row);
		secondary_.setCursor(column, row);
	}
#else
	primary_.setCursor(column, row);
	secondary_.setCursor(column, row);
#endif
}

size_t DualDisplay::write(uint8_t value) {
#if ENABLE_DUAL_DEBUG
#if ENABLE_DUAL_QUEUE
	if (queue_enabled_) {
		DUAL_DEBUG("DualDisplay: write");
	}
#endif
#endif
#if !ENABLE_DUAL_QUEUE
	const size_t written = primary_.write(value);
	secondary_.write(value);
	return written;
#else
	last_write_micros_ = micros();
	const size_t written = queue_enabled_ ? 1 : primary_.write(value);
	last_write_micros_ = micros();
	if (width_ == 0 || height_ == 0) {
		secondary_.write(value);
		return written;
	}

	if (cursor_row_ < height_ && cursor_column_ < width_) {
		const uint16_t index = static_cast<uint16_t>(cursor_row_) * width_ + cursor_column_;
		if (index < sizeof(shadow_)) {
			shadow_[index] = static_cast<char>(value);
		}
		if (cursor_row_ < 8) {
			dirty_rows_mask_ |= static_cast<uint8_t>(1U << cursor_row_);
		}
	}

	// Advance a simple cursor model for callers that write strings without
	// re-positioning each byte (e.g., the startup banner).
	++cursor_column_;
	if (cursor_column_ >= width_) {
		cursor_column_ = 0;
		cursor_row_ = static_cast<uint8_t>((cursor_row_ + 1) % (height_ ? height_ : 1));
	}

	if (!queue_enabled_) {
		secondary_.write(value);
	}
	return written;
#endif
}

void DualDisplay::createChar(uint8_t slot, const uint8_t bitmap[8]) {
	primary_.createChar(slot, bitmap);
#if ENABLE_DUAL_QUEUE
	last_write_micros_ = micros();
	if (slot < 8 && bitmap) {
		memcpy(cgram_shadow_[slot], bitmap, 8);
		cgram_dirty_mask_ |= static_cast<uint8_t>(1U << slot);
	}

	if (!queue_enabled_) {
		secondary_.createChar(slot, bitmap);
		// In non-queue mode the OLED is already up to date; clear any queued slot.
		if (slot < 8) {
			cgram_dirty_mask_ &= static_cast<uint8_t>(~(1U << slot));
		}
	}
#else
	secondary_.createChar(slot, bitmap);
#endif
}

void DualDisplay::command(uint8_t value) {
	primary_.command(value);
#if ENABLE_DUAL_QUEUE
	if (!queue_enabled_) {
		secondary_.command(value);
	}
#else
	secondary_.command(value);
#endif
}

void DualDisplay::setBacklight(uint8_t level) {
	DUAL_DEBUG("DualDisplay: setBacklight");
	primary_.setBacklight(level);
	secondary_.setBacklight(level);
}

void DualDisplay::pumpSecondary(uint8_t maxOps) {
#if ENABLE_DUAL_QUEUE
	if (!queue_enabled_ || maxOps == 0) {
		return;
	}
	if (Serial.available() > 0) {
		return;
	}
	static constexpr uint32_t kIdleBeforeRefreshUs = 20000; // avoid refreshing during ongoing UART bursts
	if ((micros() - last_write_micros_) < kIdleBeforeRefreshUs) {
		return;
	}

	uint8_t remaining = maxOps;

	// Flush any pending custom glyph slots first; rendering bytes 0..7 depends
	// on these being in sync before we repaint rows from the shadow buffer.
	while (cgram_dirty_mask_ != 0 && remaining > 0) {
		uint8_t slot = 0;
		while (slot < 8 && ((cgram_dirty_mask_ & (1U << slot)) == 0)) {
			++slot;
		}
		if (slot >= 8) {
			cgram_dirty_mask_ = 0;
			break;
		}

		secondary_.createChar(slot, cgram_shadow_[slot]);
		cgram_dirty_mask_ &= static_cast<uint8_t>(~(1U << slot));
		--remaining;
	}

	uint8_t refreshed = 0;
	while (dirty_rows_mask_ != 0 && remaining > 0) {
		uint8_t row = 0;
		while (row < height_ && ((dirty_rows_mask_ & (1U << row)) == 0)) {
			++row;
		}
		if (row >= height_) {
			dirty_rows_mask_ = 0;
			break;
		}

#if ENABLE_SERIAL_DEBUG
		const uint32_t start = SerialDebug::isRuntimeEnabled() ? micros() : 0;
#endif

		primary_.setCursor(0, row);
		secondary_.setCursor(0, row);
		const uint16_t base = static_cast<uint16_t>(row) * width_;
		for (uint8_t col = 0; col < width_; ++col) {
			const uint16_t index = base + col;
			if (index < sizeof(shadow_)) {
				const uint8_t ch = static_cast<uint8_t>(shadow_[index]);
				primary_.write(ch);
				secondary_.write(ch);
			}
		}
		dirty_rows_mask_ &= static_cast<uint8_t>(~(1U << row));

#if ENABLE_SERIAL_DEBUG
		if (SerialDebug::isRuntimeEnabled()) {
			const uint32_t duration = micros() - start;
			SerialDebug::printPrefix();
			Serial.print(F("dual.refresh.row_us="));
			Serial.print(duration);
			Serial.print(F(" row="));
			Serial.println(row);
		}
#endif
		++refreshed;
		--remaining;
	}
#else
	(void)maxOps;
#endif
}

size_t DualDisplay::pendingSecondaryWrites() const {
#if ENABLE_DUAL_QUEUE
	uint8_t count = 0;
	uint8_t mask = dirty_rows_mask_;
	while (mask) {
		count += static_cast<uint8_t>(mask & 1U);
		mask >>= 1U;
	}
	uint8_t glyphs = 0;
	uint8_t slots = cgram_dirty_mask_;
	while (slots) {
		glyphs += static_cast<uint8_t>(slots & 1U);
		slots >>= 1U;
	}
	return static_cast<size_t>(count + glyphs);
#else
	return 0;
#endif
}

void DualDisplay::setQueueingEnabled(bool enabled) {
#if ENABLE_DUAL_QUEUE
#if ENABLE_SERIAL_DEBUG
	if (SerialDebug::isRuntimeEnabled()) {
		SerialDebug::printPrefix();
		Serial.print(F("dual.queue.enabled="));
		Serial.println(enabled ? 1 : 0);
	}
#endif
	if (queue_enabled_ == enabled) {
		return;
	}
	queue_enabled_ = enabled;
#else
	(void)enabled;
#endif
}
