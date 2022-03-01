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

void startupAnimation(int blinks) {
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    for (ButtonDefinition& btn : buttonMap) {
      digitalWrite(btn.ledPin, HIGH);
      delay(100);
    }

    delay(500);

    digitalWrite(LED_BUILTIN, LOW);
    for (ButtonDefinition& btn : buttonMap) {
      digitalWrite(btn.ledPin, LOW);
      delay(100);
    }

    delay(500);
  }
}

void setup() {
  Serial.begin(9600);

  for (ButtonDefinition& btn : buttonMap) {
    btn.beingPressed = false;
    pinMode(btn.inputPin, INPUT_PULLUP);
    pinMode(btn.ledPin, OUTPUT);
  }

  // Set mode based on button held at startup (See config.h to customize)
  if (digitalRead(MODE_SELECT_GAMEPAD) == LOW) {
    Serial.println("Booting into gamepad mode");
    mode = new GamepadMode();
    startupAnimation(2);
  } else {
    Serial.println("Booting into keyboard mode");
    mode = new KeyboardMode();
    startupAnimation(1);
  }

  assert(mode != NULL);
  mode->setup();
}

void loop() {
  if (!mode->loop()) {
    return;
  }

  // TODO: maybe use interrupts here? this is a bit slow
  for (ButtonDefinition& btn : buttonMap) {
    bool pressed = digitalRead(btn.inputPin) == LOW;
    if (pressed != btn.beingPressed) {
      btn.beingPressed = pressed;

      if (pressed) {
        digitalWrite(btn.ledPin, HIGH);
        mode->pressButton(btn.type);
      } else {
        digitalWrite(btn.ledPin, LOW);
        mode->releaseButton(btn.type);
      }
    }
  }
}
