#include "display/DualDisplay.h"

#include <Arduino.h>
#include <DisplayConfig.h>

#include "SerialDebug.h"

#if ENABLE_DUAL_DEBUG
#define DUAL_DEBUG(msg) SerialDebug::line(true, F(msg))
#else
#define DUAL_DEBUG(msg) do {} while (0)
#endif

DualDisplay::DualDisplay(IDisplay &primary, IDisplay &secondary)
    : primary_(primary), secondary_(secondary) {}

void DualDisplay::begin(uint8_t width, uint8_t height) {
	DUAL_DEBUG("DualDisplay: begin primary");
	primary_.begin(width, height);
	DUAL_DEBUG("DualDisplay: begin secondary");
	secondary_.begin(width, height);
	DUAL_DEBUG("DualDisplay: begin complete");
}

void DualDisplay::clear() {
	DUAL_DEBUG("DualDisplay: clear");
	primary_.clear();
	secondary_.clear();
}

void DualDisplay::home() {
	primary_.home();
	secondary_.home();
}

void DualDisplay::display() {
	primary_.display();
	secondary_.display();
}

void DualDisplay::setCursor(uint8_t column, uint8_t row) {
	primary_.setCursor(column, row);
	secondary_.setCursor(column, row);
}

size_t DualDisplay::write(uint8_t value) {
	DUAL_DEBUG("DualDisplay: write");
	const size_t written = primary_.write(value);
	secondary_.write(value);
	return written;
}

void DualDisplay::createChar(uint8_t slot, const uint8_t bitmap[8]) {
	primary_.createChar(slot, bitmap);
	secondary_.createChar(slot, bitmap);
}

void DualDisplay::command(uint8_t value) {
	primary_.command(value);
	secondary_.command(value);
}

void DualDisplay::setBacklight(uint8_t level) {
	DUAL_DEBUG("DualDisplay: setBacklight");
	primary_.setBacklight(level);
	secondary_.setBacklight(level);
}
