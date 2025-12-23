#include "display/display_factory.h"

#include <DisplayConfig.h>

#include "display/HD44780Display.h"

IDisplay &getDisplay() {
#if DISPLAY_BACKEND == HD44780
	static HD44780Display display(
	    12, // RS
	    11, // Enable
	    2,  // D0
	    3,  // D1
	    4,  // D2
	    5,  // D3
	    6,  // D4
	    7,  // D5
	    8,  // D6
	    9,  // D7
	    LED_PIN);
	return display;
#else
#error "Selected DISPLAY_BACKEND is not implemented."
#endif
}
