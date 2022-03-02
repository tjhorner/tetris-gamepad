#include "gamepad.h"

void GamepadMode::setup() {
  gamepad.deviceName = "TGP (Gamepad)";
  gamepad.deviceManufacturer = "TJ Horner";
  gamepad.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  gamepad.begin();
}

void GamepadMode::teardown() {
  gamepad.end();
}

bool GamepadMode::loop() {
  if (!gamepad.isConnected()) {
    delay(1000);
    return false;
  }

  return true;
}

void GamepadMode::pressButton(ButtonType button) {
  gamepad.press(buttonMap[button]);
}

void GamepadMode::releaseButton(ButtonType button) {
  gamepad.release(buttonMap[button]);
}