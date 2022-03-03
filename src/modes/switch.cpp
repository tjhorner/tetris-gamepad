#include "switch.h"

void SwitchMode::setup() {
  controller.begin();
}

void SwitchMode::teardown() { }

void SwitchMode::pressButton(ButtonType type) {
  controller.pressButton(this->buttonMap[type]);
}

void SwitchMode::releaseButton(ButtonType type) {
  controller.releaseButton(this->buttonMap[type]);
}