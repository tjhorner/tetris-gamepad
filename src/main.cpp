#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard;
bool pressed = false;

struct KeyDefinition {
  uint8_t inputPin;
  uint8_t ledPin;
  uint8_t keyCode;
  bool beingPressed;
};

KeyDefinition keys[] = {
  // input pin, led pin, key
  { 23, 22, ' ' },
  // { 33, 32, KEY_LEFT_ARROW }
}; 

void setup() {
  for (KeyDefinition& key : keys) {
    key.beingPressed = false;
    pinMode(key.inputPin, INPUT_PULLUP);
    pinMode(key.ledPin, OUTPUT);
  }

  bleKeyboard.setName("da gamepad");
  bleKeyboard.begin();

  Serial.begin(9600);
}

void loop() {
  if (!bleKeyboard.isConnected()) {
    delay(1000);
    return;
  }

  for (KeyDefinition& key : keys) {
    bool pressed = digitalRead(key.inputPin) == LOW;
    if (pressed != key.beingPressed) {
      key.beingPressed = pressed;
      if (pressed) {
        digitalWrite(key.ledPin, HIGH);
        bleKeyboard.press(key.keyCode);
      } else {
        digitalWrite(key.ledPin, LOW);
        bleKeyboard.release(key.keyCode);
      }
    }
  }
}