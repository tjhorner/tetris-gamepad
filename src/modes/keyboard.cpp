#include "keyboard.h"

void KeyboardMode::setup() {
  keyboard.setName("TGP (Keyboard)");
  keyboard.begin();
}

void KeyboardMode::teardown() {
  keyboard.end();
}

bool KeyboardMode::loop() {
  if (!keyboard.isConnected()) {
    delay(1000);
    return false;
  }

  return true;
}

void KeyboardMode::pressButton(ButtonType btn) {
  keyboard.press(this->buttonMap[btn]);
}

void KeyboardMode::releaseButton(ButtonType btn) {
  keyboard.release(buttonMap[btn]);
}