#pragma once

#include <Arduino.h>
#include <DisplayConfig.h>

namespace SerialDebug {
#if ENABLE_SERIAL_DEBUG
void setRuntimeEnabled(bool enabled);
bool isRuntimeEnabled();

inline void printPrefix() {
	Serial.print(F("debug:"));
	Serial.print(micros());
	Serial.print(F("us "));
}

inline void line(bool enabled, const __FlashStringHelper *message) {
	if (!enabled || message == nullptr || !isRuntimeEnabled()) {
		return;
	}
	printPrefix();
	Serial.println(message);
}

inline void line(bool enabled, const char *message) {
	if (!enabled || message == nullptr || !isRuntimeEnabled()) {
		return;
	}
	printPrefix();
	Serial.println(message);
}

template <typename TValue>
inline void kv(bool enabled, const __FlashStringHelper *key, const TValue &value) {
	if (!enabled || key == nullptr || !isRuntimeEnabled()) {
		return;
	}
	printPrefix();
	Serial.print(key);
	Serial.print(F("="));
	Serial.println(value);
}

template <typename TValue>
inline void kv(bool enabled, const char *key, const TValue &value) {
	if (!enabled || key == nullptr || !isRuntimeEnabled()) {
		return;
	}
	printPrefix();
	Serial.print(key);
	Serial.print(F("="));
	Serial.println(value);
}
#else
inline void setRuntimeEnabled(bool) {}
inline bool isRuntimeEnabled() { return false; }
inline void printPrefix() {}
inline void line(bool, const __FlashStringHelper *) {}
inline void line(bool, const char *) {}
template <typename TValue>
inline void kv(bool, const __FlashStringHelper *, const TValue &) {}
template <typename TValue>
inline void kv(bool, const char *, const TValue &) {}
#endif
} // namespace SerialDebug
