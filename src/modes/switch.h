#include <Arduino.h>
// #include <ProController.h>
#include <map>
#include "mode.h"
#include "pins.h"
#include "config.h"

#ifndef SWITCH_H
#define SWITCH_H

class SwitchMode : public Mode {
  // private:
  //   ProController controller;

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