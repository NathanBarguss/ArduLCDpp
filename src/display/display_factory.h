#pragma once

#include "display/IDisplay.h"

IDisplay &getDisplay();
void serviceDisplayIdleWork();
void setDualQueueingEnabled(bool enabled);
