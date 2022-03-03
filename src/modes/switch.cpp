#include "switch.h"

void SwitchMode::setup() {
  controller.begin();
}

void SwitchMode::teardown() { }

bool SwitchMode::loop() {
  controller.loop();
  return true;
}

void SwitchMode::pressButton(ButtonType type) {
  controller.pressButton(this->buttonMap[type]);
}

void SwitchMode::releaseButton(ButtonType type) {
  controller.releaseButton(this->buttonMap[type]);
}