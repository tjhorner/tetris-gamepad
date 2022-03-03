#include <Arduino.h>
#include <ProController.h>
#include <map>
#include "mode.h"
#include "pins.h"
#include "config.h"

#ifndef SWITCH_H
#define SWITCH_H

class SwitchMode : public Mode {
  private:
    ProController controller;
    std::map<ButtonType, ProControllerButton> buttonMap = {
      { ButtonType::Hold, ButtonL },
      { ButtonType::CCW, ButtonB },
      { ButtonType::CW, ButtonA },
      { ButtonType::Left, ButtonDPadLeft },
      { ButtonType::Right, ButtonDPadRight },
      { ButtonType::HardDrop, ButtonDPadUp },
      { ButtonType::SoftDrop, ButtonDPadDown },
      { ButtonType::Zone, ButtonZL },
    };

  public:
    SwitchMode() { };
    ~SwitchMode() { };

    void setup();
    void teardown();
    bool loop();
    void pressButton(ButtonType type);
    void releaseButton(ButtonType type);
};

#endif