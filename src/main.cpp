#include "main.h"

Mode* mode;

struct ButtonDefinition {
  uint8_t inputPin;
  uint8_t ledPin;
  ButtonType type;
  bool beingPressed;
};

ButtonDefinition buttonMap[] = {
  // Row 1
  { BUTTON_HOLD_INPUT, BUTTON_HOLD_LED, ButtonType::Hold }, // Hold
  { BUTTON_CCW_INPUT, BUTTON_CCW_LED, ButtonType::CCW }, // CCW
  { BUTTON_CW_INPUT, BUTTON_CW_LED, ButtonType::CW }, // CW
  { BUTTON_LEFT_INPUT, BUTTON_LEFT_LED, ButtonType::Left }, // Left
  { BUTTON_RIGHT_INPUT, BUTTON_RIGHT_LED, ButtonType::Right }, // Right

  // Row 2
  { BUTTON_HARD_DROP_INPUT, BUTTON_HARD_DROP_LED, ButtonType::HardDrop }, // Hard Drop
  { BUTTON_SOFT_DROP_INPUT, BUTTON_SOFT_DROP_LED, ButtonType::SoftDrop }, // Soft Drop

  // Row 3
  { BUTTON_ZONE_INPUT, BUTTON_ZONE_LED, ButtonType::Zone }, // Zone
};

enum SelectedMode {
  Gamepad, // default, since it's first
  Keyboard,
  Switch
};

SelectedMode getSavedMode() {
  return (SelectedMode) EEPROM.read(ADDR_SAVED_MODE);
}

void setSavedMode(SelectedMode mode) {
  EEPROM.write(ADDR_SAVED_MODE, (uint8_t) mode);
  EEPROM.commit();
}

void startupAnimation(int repeatCount) {
  for (int i = 0; i < repeatCount; i++) {
    for (ButtonDefinition& btn : buttonMap) {
      digitalWrite(btn.ledPin, HIGH);
      delay(50);
    }

    delay(100);

    for (ButtonDefinition& btn : buttonMap) {
      digitalWrite(btn.ledPin, LOW);
      delay(50);
    }

    delay(100);
  }
}

void IRAM_ATTR isr() {
  for (ButtonDefinition& btn : buttonMap) {
    bool pressed = digitalRead(btn.inputPin) == LOW;
    if (pressed != btn.beingPressed) {
      btn.beingPressed = pressed;

      if (pressed) {
        assert(mode != NULL);
        mode->pressButton(btn.type);
        // digitalWrite(btn.ledPin, HIGH);
      } else {
        assert(mode != NULL);
        mode->releaseButton(btn.type);
        // digitalWrite(btn.ledPin, LOW);
      }
    }
  }
}

void attachInterrupts() {
  for (ButtonDefinition& btn : buttonMap) {
    attachInterrupt(digitalPinToInterrupt(btn.inputPin), isr, CHANGE);
  }
}

extern "C" void app_main() {
  initArduino();

  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  for (ButtonDefinition& btn : buttonMap) {
    btn.beingPressed = false;
    pinMode(btn.inputPin, INPUT_PULLUP);
    pinMode(btn.ledPin, OUTPUT);
  }

  // Set mode based on button held at startup (See config.h to customize)
  if (digitalRead(MODE_SELECT_GAMEPAD) == LOW) {
    Serial.println("Switching to gamepad mode");
    setSavedMode(Gamepad);
  } else if (digitalRead(MODE_SELECT_KEYBOARD) == LOW) {
    Serial.println("Switching to keyboard mode");
    setSavedMode(Keyboard);
  } else if (digitalRead(MODE_SELECT_SWITCH) == LOW) {
    Serial.println("Switching to Switch Pro Controller mode");
    setSavedMode(Switch);
  }

  switch (getSavedMode()) {
    default:
      // somehow got in an invalid state
      setSavedMode(Gamepad);
    case Gamepad:
      mode = new GamepadMode();
      startupAnimation(1);
      break;
    case Keyboard:
      mode = new KeyboardMode();
      startupAnimation(2);
      break;
    case Switch:
      mode = new SwitchMode();
      startupAnimation(3);
      break;
  }

  bool forceResetMac = digitalRead(RESET_MAC_INPUT) == LOW;
  setMacAddress((int) getSavedMode(), forceResetMac);

  assert(mode != NULL);
  mode->setup();

  attachInterrupts();

  Serial.println("booted!");
}
