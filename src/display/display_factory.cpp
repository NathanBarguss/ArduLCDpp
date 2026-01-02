#include "display/display_factory.h"

#include <DisplayConfig.h>

#include "display/HD44780Display.h"

IDisplay &getDisplay() {
#if DISPLAY_BACKEND == HD44780
	static HD44780Display display(
	    12, // RS
	    2,  // Enable
	    3,  // D0
	    4,  // D1
	    5,  // D2
	    6,  // D3
	    7,  // D4
	    8,  // D5
	    9,  // D6
	    10, // D7
	    LED_PIN);
	return display;
#else
#error "Selected DISPLAY_BACKEND is not implemented."
#endif
}
