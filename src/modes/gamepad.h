#include <BleGamepad.h>
#include <map>
#include "esp_log.h"
#include "mode.h"
#include "pins.h"
#include "config.h"

#ifndef GAMEPAD_H
#define GAMEPAD_H

class GamepadMode : public Mode {
  private:
    bool reportDirty = false;
    TaskHandle_t reportTaskHandle = NULL;
    SemaphoreHandle_t mutex = NULL;
    BleGamepad gamepad;
    std::map<ButtonType, uint8_t> buttonMap = {
      { ButtonType::Hold, BUTTON_1 },
      { ButtonType::CCW, BUTTON_2 },
      { ButtonType::CW, BUTTON_3 },
      { ButtonType::Left, BUTTON_4 },
      { ButtonType::Right, BUTTON_5 },
      { ButtonType::HardDrop, BUTTON_6 },
      { ButtonType::SoftDrop, BUTTON_7 },
      { ButtonType::Zone, BUTTON_8 },
    };

  public:
    GamepadMode() { };
    ~GamepadMode() { };

    void setup();
    void teardown();
    void pressButton(ButtonType button);
    void releaseButton(ButtonType button);
    
    void sendReport();
};

#endif