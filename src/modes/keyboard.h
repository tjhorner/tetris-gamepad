#include <Arduino.h>
#include <BleKeyboard.h>
#include <map>
#include "mode.h"
#include "pins.h"
#include "config.h"

#ifndef KEYBOARD_H
#define KEYBOARD_H

class KeyboardMode : public Mode {
  private:
    BleKeyboard keyboard;
    std::map<ButtonType, uint8_t> buttonMap = {
      { ButtonType::Hold, KEY_LEFT_SHIFT },
      { ButtonType::CCW, 'z' },
      { ButtonType::CW, 'x' },
      { ButtonType::Left, KEY_LEFT_ARROW },
      { ButtonType::Right, KEY_RIGHT_ARROW },
      { ButtonType::HardDrop, ' ' },
      { ButtonType::SoftDrop, KEY_DOWN_ARROW },
      { ButtonType::Zone, 'a' },
    };

  public:
    KeyboardMode() { };
    ~KeyboardMode() { };

    void setup();
    void teardown();
    bool loop();
    void pressButton(ButtonType type);
    void releaseButton(ButtonType type);
};

#endif