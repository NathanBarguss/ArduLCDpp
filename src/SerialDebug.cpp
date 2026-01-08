#include "SerialDebug.h"

#if ENABLE_SERIAL_DEBUG
namespace SerialDebug {
static bool runtime_enabled = false;

void setRuntimeEnabled(bool enabled) {
	runtime_enabled = enabled;
}

bool isRuntimeEnabled() {
	return runtime_enabled;
}
} // namespace SerialDebug
#endif
