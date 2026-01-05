#pragma once

#include <Arduino.h>
#include <DisplayConfig.h>

namespace SerialDebug {
inline void printPrefix() {
	Serial.print(F("debug:"));
	Serial.print(micros());
	Serial.print(F("us "));
}

inline void line(bool enabled, const __FlashStringHelper *message) {
	if (!enabled || message == nullptr) {
		return;
	}
	printPrefix();
	Serial.println(message);
}

inline void line(bool enabled, const char *message) {
	if (!enabled || message == nullptr) {
		return;
	}
	printPrefix();
	Serial.println(message);
}

template <typename TValue>
inline void kv(bool enabled, const __FlashStringHelper *key, const TValue &value) {
	if (!enabled || key == nullptr) {
		return;
	}
	printPrefix();
	Serial.print(key);
	Serial.print(F("="));
	Serial.println(value);
}

template <typename TValue>
inline void kv(bool enabled, const char *key, const TValue &value) {
	if (!enabled || key == nullptr) {
		return;
	}
	printPrefix();
	Serial.print(key);
	Serial.print(F("="));
	Serial.println(value);
}
} // namespace SerialDebug
