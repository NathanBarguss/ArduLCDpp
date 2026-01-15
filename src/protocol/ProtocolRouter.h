#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "protocol/ActiveProtocol.h"

#ifndef PROTO_DETECT_BUFFER_BYTES
#define PROTO_DETECT_BUFFER_BYTES 16
#endif

#ifndef PROTO_DETECT_TIMEOUT_MS
#define PROTO_DETECT_TIMEOUT_MS 250
#endif

#ifndef PROTO_SESSION_IDLE_RESET_MS
#define PROTO_SESSION_IDLE_RESET_MS 10000
#endif

class ProtocolRouter {
public:
	ProtocolRouter() { reset(); }

	ActiveProtocol protocol() const { return protocol_; }

	void reset() {
		protocol_ = ActiveProtocol::Unknown;
		buffer_len_ = 0;
		replay_index_ = 0;
		replaying_ = false;
		first_byte_seen_ = false;
		first_byte_ms_ = 0;
	}

	bool maybeResetAfterIdleUs(uint32_t idle_us) {
		if (protocol_ == ActiveProtocol::Unknown) {
			return false;
		}
		const uint32_t threshold_us = static_cast<uint32_t>(PROTO_SESSION_IDLE_RESET_MS) * 1000UL;
		if (idle_us < threshold_us) {
			return false;
		}
		reset();
		return true;
	}

	uint8_t read(uint8_t (*read_byte_blocking)()) {
		if (replaying_) {
			if (replay_index_ < buffer_len_) {
				return buffer_[replay_index_++];
			}
			replaying_ = false;
		}

		if (protocol_ != ActiveProtocol::Unknown) {
			return read_byte_blocking();
		}

		// Detection phase: buffer bytes until we can safely pick a protocol.
		while (protocol_ == ActiveProtocol::Unknown) {
			const uint8_t b = read_byte_blocking();
			if (!first_byte_seen_) {
				first_byte_seen_ = true;
				first_byte_ms_ = millis();
			}
			if (buffer_len_ < PROTO_DETECT_BUFFER_BYTES) {
				buffer_[buffer_len_++] = b;
			}

			const ActiveProtocol decided = detectFromBuffer();
			const bool timed_out = first_byte_seen_ && (millis() - first_byte_ms_) >= PROTO_DETECT_TIMEOUT_MS;
			const bool buffer_full = buffer_len_ >= PROTO_DETECT_BUFFER_BYTES;

			if (decided != ActiveProtocol::Unknown) {
				commit(decided);
				break;
			}

			if (timed_out || buffer_full) {
				// Ambiguity defaults to LOS-PANEL to preserve backward compatibility.
				commit(ActiveProtocol::LosPanel);
				break;
			}
		}

		// Now that we have committed, start replaying from byte 0.
		return buffer_[replay_index_++];
	}

private:
	void commit(ActiveProtocol protocol) {
		protocol_ = protocol;
		replaying_ = true;
		replay_index_ = 0;
	}

	ActiveProtocol detectFromBuffer() const {
		if (buffer_len_ == 0) {
			return ActiveProtocol::Unknown;
		}

		uint16_t los_score = 0;
		uint16_t mo_score = 0;

		for (uint8_t i = 0; i < buffer_len_; ++i) {
			const uint8_t b = buffer_[i];
			if (b == 0xFD) {
				// Strong LOS-PANEL indicator: brightness byte.
				los_score = static_cast<uint16_t>(los_score + 200);
				continue;
			}
			if (b != 0xFE) {
				continue;
			}

			if (i + 1 >= buffer_len_) {
				continue;
			}

			const uint8_t cmd = buffer_[static_cast<uint8_t>(i + 1)];

			// Strong LOS-PANEL indicator: set-DDRAM-address opcodes.
			if (cmd >= 0x80) {
				los_score = static_cast<uint16_t>(los_score + 40);
				continue;
			}

			// Strong Matrix Orbital indicators (LCD Smartie 5.x + matrix.dll init).
			if (cmd == 0x98) { // brightness level
				mo_score = static_cast<uint16_t>(mo_score + 80);
				continue;
			}
			if (cmd == 'G') { // set cursor x,y
				if (i + 3 < buffer_len_) {
					const uint8_t x = buffer_[static_cast<uint8_t>(i + 2)];
					const uint8_t y = buffer_[static_cast<uint8_t>(i + 3)];
					if (x >= 1 && x <= 20 && y >= 1 && y <= 4) {
						mo_score = static_cast<uint16_t>(mo_score + 120);
						continue;
					}
				}
			}

			switch (cmd) {
				// LCD Smartie (matrix.dll) init commands.
				case 'T': // cursor blink off
				case 'X': // clear screen
				case 'K': // cursor off
				case 'R': // auto scroll off
				case 'D': // auto line wrap off
				case 'A': // auto transmit keys
				case '`': // auto repeat off
				case 'B': // backlight on (param follows)
				case 'F': // backlight off
				case 'P': // contrast
					mo_score = static_cast<uint16_t>(mo_score + 25);
					break;
				default:
					// Weak LOS indicator: common HD44780 init commands.
					if (cmd == 0x01 || cmd == 0x02 || (cmd >= 0x08 && cmd <= 0x0F)) {
						los_score = static_cast<uint16_t>(los_score + 10);
					}
					break;
			}
		}

		if (los_score >= 100) {
			return ActiveProtocol::LosPanel;
		}
		if (mo_score >= 120 && mo_score > los_score) {
			return ActiveProtocol::MatrixOrbital;
		}
		return ActiveProtocol::Unknown;
	}

	ActiveProtocol protocol_;
	uint8_t buffer_[PROTO_DETECT_BUFFER_BYTES];
	uint8_t buffer_len_;
	uint8_t replay_index_;
	bool replaying_;
	bool first_byte_seen_;
	uint32_t first_byte_ms_;
};
