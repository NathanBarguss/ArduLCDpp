#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include <DisplayConfig.h>

#include "SerialDebug.h"
#include "display/display_factory.h"

#if DISPLAY_BACKEND != HD44780
#include "display/Hd44780CommandTranslator.h"
#endif

#if DISPLAY_BACKEND != HD44780 && DISPLAY_BACKEND != OLED && DISPLAY_BACKEND != DUAL
#error "Unknown DISPLAY_BACKEND selected; update the firmware or config."
#endif

#if ENABLE_SERIAL_DEBUG && ENABLE_VERBOSE_DEBUG_LOGS
#define DEBUG_LOG(msg) SerialDebug::line(true, F(msg))
#else
#define DEBUG_LOG(msg) do {} while (0)
#endif

byte cmd; //will hold our sent command

static IDisplay &display = getDisplay();
static bool host_active = false;
static bool startup_screen_visible = false;
static bool pending_host_active_report = false;
#if ENABLE_SERIAL_DEBUG
static uint32_t last_rx_micros = 0;
static constexpr uint32_t HOST_IDLE_BEFORE_LOG_US = 20000; // 20ms of quiet = burst finished at 57,600 bps
static uint16_t rx_bytes_total = 0;
static uint16_t rx_bytes_since_boot = 0;
#endif

#if ENABLE_SERIAL_DEBUG
static void emit_boot_diagnostics();
static void maybe_emit_host_active_report() {
	// Once the host stream has gone idle, emit the "host active" banner and
	// boot diagnostics (and re-enable verbose logging) without risking RX loss.
	if (!pending_host_active_report) {
		return;
	}
	if (Serial.available() != 0) {
		return;
	}
	if ((micros() - last_rx_micros) <= HOST_IDLE_BEFORE_LOG_US) {
		return;
	}

	SerialDebug::setRuntimeEnabled(true);
	Serial.println(F("raw: host active"));
	SerialDebug::kv(true, F("rx.bytes_total"), rx_bytes_total);
	SerialDebug::kv(true, F("rx.bytes_since_boot"), rx_bytes_since_boot);
	emit_boot_diagnostics();
	SerialDebug::line(true, F("serial_debug: host active"));
	pending_host_active_report = false;
}
#endif

#if DISPLAY_BACKEND != HD44780
static Hd44780CommandTranslator command_translator(display);
#endif

#if ENABLE_SERIAL_DEBUG
static constexpr uint16_t SERIAL_WAIT_LOG_THRESHOLD_US = 500;
static constexpr uint8_t SERIAL_BACKLOG_LOW_WATER = 8;

static int16_t free_sram() {
	extern char __heap_start;
	extern char *__brkval;
	char stack_top;
	char *heap_end = __brkval ? __brkval : &__heap_start;
	return static_cast<int16_t>(&stack_top - heap_end);
}

static uint32_t compute_i2c_clock_hz() {
	const uint8_t twps = static_cast<uint8_t>(TWSR & ((1 << TWPS0) | (1 << TWPS1)));
	uint16_t prescaler = 1;
	if (twps == 1) {
		prescaler = 4;
	} else if (twps == 2) {
		prescaler = 16;
	} else if (twps == 3) {
		prescaler = 64;
	}
	const uint32_t denominator = static_cast<uint32_t>(16UL + (2UL * TWBR * prescaler));
	return denominator == 0 ? 0 : static_cast<uint32_t>(F_CPU / denominator);
}

struct BootDiagnostics {
	uint32_t display_begin_us;
	int16_t free_sram_bytes;
	uint32_t i2c_clock_hz;
	bool captured;
};

static BootDiagnostics boot_diagnostics = {0, 0, 0, false};

static void capture_boot_diagnostics(uint32_t display_begin_us) {
	boot_diagnostics.display_begin_us = display_begin_us;
	boot_diagnostics.free_sram_bytes = free_sram();
	boot_diagnostics.i2c_clock_hz = compute_i2c_clock_hz();
	boot_diagnostics.captured = true;
}

static void emit_boot_diagnostics() {
	if (!boot_diagnostics.captured) {
		return;
	}
	SerialDebug::kv(true, F("display.begin.us"), boot_diagnostics.display_begin_us);
	SerialDebug::kv(true, F("free_sram.bytes"), boot_diagnostics.free_sram_bytes);
	SerialDebug::kv(true, F("i2c.clock.hz"), boot_diagnostics.i2c_clock_hz);
	boot_diagnostics.captured = false;
}
#endif

static void write_centered_line(const char *text, uint8_t row) {
	if (!text || row >= LCDH) {
		return;
	}

	const size_t len = strlen(text);
	const size_t slice = len > LCDW ? LCDW : len;
	char buffer[LCDW + 1];
	memcpy(buffer, text, slice);
	buffer[slice] = '\0';

	uint8_t column = 0;
	if (slice < LCDW) {
		column = static_cast<uint8_t>((LCDW - slice) / 2);
	}
	display.setCursor(column, row);
	display.write(buffer);
}

static void display_startup_screen() {
	startup_screen_visible = true;
	display.clear();
	display.home();
	write_centered_line("ArduLCDpp Ready", 0);
	if (LCDH > 1) {
		write_centered_line("Waiting for host...", 1);
	}
	if (LCDH > 2) {
		write_centered_line("bit.ly/4plthUv", 2);
	}
}

static void dismiss_startup_screen() {
	if (!startup_screen_visible) {
		return;
	}
	startup_screen_visible = false;
	display.clear();
	display.home();
}

void setup() {
	Serial.begin(BAUDRATE);

	Serial.print(F("ENABLE_SERIAL_DEBUG:"));
	Serial.println(ENABLE_SERIAL_DEBUG);

	Serial.print(F("ENABLE_VERBOSE_DEBUG_LOGS:"));
	Serial.println(ENABLE_VERBOSE_DEBUG_LOGS);	

#if ENABLE_SERIAL_DEBUG
	SerialDebug::setRuntimeEnabled(true);
	Serial.println(F("debug: instrumentation armed"));
#endif
	setDualQueueingEnabled(false);
	DEBUG_LOG("setup: serial online");
	// set up the LCD's number of columns and rows:
	DEBUG_LOG("setup: display.begin");
#if ENABLE_SERIAL_DEBUG
	const uint32_t display_begin_start = micros();
#endif
	display.begin(LCDW, LCDH);
#if ENABLE_SERIAL_DEBUG
	const uint32_t display_begin_duration = micros() - display_begin_start;
#endif
	DEBUG_LOG("setup: display.begin complete");
#if ENABLE_SERIAL_DEBUG
	capture_boot_diagnostics(display_begin_duration);
#endif
	display.setBacklight(STARTUP_BRIGHTNESS);
	DEBUG_LOG("setup: backlight set");
	display.display();
	DEBUG_LOG("setup: display() called");
#if DISPLAY_BACKEND != HD44780
	command_translator.reset();
#endif
	display_startup_screen();
	DEBUG_LOG("setup: startup banner drawn");
#if ENABLE_SERIAL_DEBUG
	Serial.print(F("debug: free_sram.after_banner="));
	Serial.println(free_sram());
#endif
	// Ensure any deferred OLED bytes drain before we start servicing the host.
	serviceDisplayIdleWork();
}

int serial_read() {
	int result = -1;
#if ENABLE_SERIAL_DEBUG
	const uint32_t wait_start = micros();
	uint16_t spins = 0;
#endif
	while(result == -1) {
		if(Serial.available() > 0) {
			result = Serial.read();
#if ENABLE_SERIAL_DEBUG
			++rx_bytes_total;
			++rx_bytes_since_boot;
			last_rx_micros = micros();
#endif
		}
#if ENABLE_SERIAL_DEBUG
		else {
			maybe_emit_host_active_report();
			serviceDisplayIdleWork();
			++spins;
		}
#else
		else {
			serviceDisplayIdleWork();
		}
#endif
	}
#if ENABLE_SERIAL_DEBUG
	const uint32_t wait_us = micros() - wait_start;
	const int backlog = Serial.available();
	if (SerialDebug::isRuntimeEnabled() &&
	    (wait_us > SERIAL_WAIT_LOG_THRESHOLD_US || backlog < SERIAL_BACKLOG_LOW_WATER)) {
		SerialDebug::printPrefix();
		Serial.print(F("serial.byte wait_us="));
		Serial.print(wait_us);
		Serial.print(F(" spins="));
		Serial.print(spins);
		Serial.print(F(" backlog="));
		Serial.println(backlog);
	}
#endif
	return result;
}

/**
 * From : https://github.com/lcdproc/lcdproc/blob/master/server/drivers.c
 * All the serial connections supported by driver
 *

static const struct hd44780_SerialInterface serial_interfaces[] = {
//        type                  instr ms  data     v     ^   pre bitrate bits  K   esc   cmd  B  Besc  Boff   Bon Multi  End
	{ HD44780_CT_PICANLCD,      0x11,  0, 0x12, 0x00, 0x1F,    0,   9600,   8, 0, 0x00,    0, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LCDSERIALIZER, 0xFE,  0,    0, 0x00, 0x00,    0,   9600,   8, 0, 0x00,    0, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LOS_PANEL,     0xFE,  0,    0, 0x00, 0x00,    0,   9600,   4, 1, 0xFE,    0, 1, 0xFD,    0, 0xFF,   0,    0 },
	{ HD44780_CT_VDR_LCD,       0xFE,  0,    0, 0x00, 0x00,    0,   9600,   4, 0, 0x00,    0, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_VDR_WAKEUP,    0xC0,  0, 0xC4, 0xC0, 0xCF,    0,   9600,   4, 0, 0x00,    0, 1,    0, 0xC9, 0xC8,   1, 0xCF },
	{ HD44780_CT_PERTELIAN,     0xFE,  0,    0, 0x00, 0x00,    0,   9600,   8, 0, 0x00,    0, 1, 0xFE, 0x02, 0x03,   0,    0 },
	{ HD44780_CT_EZIO,          0xFE, 40,    0, 0x00, 0x00, 0x28,   2400,   4, 1, 0xFD, 0x06, 0,    0,    0,    0,   0,    0 }
};

Additional resources:

https://github.com/lcdproc/lcdproc/blob/master/server/drivers/hd44780-serial.c
https://lcdproc.sourceforge.net/docs/lcdproc-0-5-6-user.html#los-panel
*/

void loop() {
	cmd = serial_read();
	if (!host_active) {
		host_active = true;
#if ENABLE_SERIAL_DEBUG
		// Important: do not emit verbose serial logs while the host may still be
		// streaming a burst, otherwise we can block long enough to overrun the
		// UART RX buffer and lose the tail bytes we are trying to process.
		SerialDebug::setRuntimeEnabled(false);
		rx_bytes_since_boot = 0;
		pending_host_active_report = true;
#else
		SerialDebug::setRuntimeEnabled(true);
#endif
		setDualQueueingEnabled(true);
		dismiss_startup_screen();
	}
	// Note: host-active reporting is emitted from the `serial_read()` idle loop
	// so we still report even when the host stops sending bytes.
	switch(cmd) {
			case 0xFE:
#if DISPLAY_BACKEND == HD44780
				display.command(serial_read());
#else
				command_translator.handleCommand(static_cast<uint8_t>(serial_read()));
#endif
				break;
			case 0xFD:
				// backlight control
				display.setBacklight(serial_read());
				break;
			default:
				// By default we write to the LCD
#if DISPLAY_BACKEND == HD44780
				display.write(cmd);
#else
				if (!command_translator.handleData(cmd)) {
					display.write(cmd);
				}
#endif
				break;
	}
	serviceDisplayIdleWork();
}
