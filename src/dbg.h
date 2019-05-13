#pragma once

#include <HardwareSerial.h>
#include "hardware.h"

#ifndef LOGGING
#define LOGGING false
#endif

#if LOGGING
#define DBG(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif
