#include "gamepad.h"

void GamepadMode::setup() {
  gamepad.deviceName = "TGP (Gamepad)";
  gamepad.deviceManufacturer = "TJ Horner";
  // gamepad.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  gamepad.begin();
}

void GamepadMode::teardown() {
  gamepad.end();
}

bool GamepadMode::loop() {
  return gamepad.isConnected();
}

void GamepadMode::pressButton(ButtonType button) {
  gamepad.press(this->buttonMap[button]);
}

void GamepadMode::releaseButton(ButtonType button) {
  gamepad.release(this->buttonMap[button]);
}