#include "keyboard.h"

void KeyboardMode::setup() {
  keyboard.setName("TGP (Keyboard)");
  keyboard.begin();
}

void KeyboardMode::teardown() {
  keyboard.end();
}

void KeyboardMode::pressButton(ButtonType btn) {
  keyboard.press(this->buttonMap[btn]);
}

void KeyboardMode::releaseButton(ButtonType btn) {
  keyboard.release(this->buttonMap[btn]);
}