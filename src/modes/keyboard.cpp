#include "keyboard.h"

void KeyboardMode::setup() {
  Serial.println("Setting up keyboard mode");

  keyboard.setName("Tetris Gamepad");
  keyboard.begin();
}

void KeyboardMode::teardown() {
  keyboard.end();
}

bool KeyboardMode::loop() {
  if (!keyboard.isConnected()) {
    Serial.println("Waiting for connection");
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