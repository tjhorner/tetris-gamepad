#ifndef USE_NIMBLE
#define USE_NIMBLE
#endif

#include <Arduino.h>
#include <EEPROM.h>
#include "button.h"
#include "pins.h"
#include "config.h"
#include "randmac.h"
#include "modes/mode.h"
#include "modes/gamepad.h"
#include "modes/keyboard.h"
#include "modes/switch.h"