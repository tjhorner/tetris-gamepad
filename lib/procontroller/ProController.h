#ifndef PRO_CONTROLLER_H
#define PRO_CONTROLLER_H

#include <esp_bt.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <freertos/semphr.h>
#include "esp_hidd_api.h"

// in the order they appear in the HID report
enum ProControllerButton {
  ButtonY,
  ButtonX,
  ButtonB,
  ButtonA,
  ButtonSR_JCL,
  ButtonSL_JCL,
  ButtonR,
  ButtonZR,

  ButtonMinus,
  ButtonPlus,
  ButtonRStick,
  ButtonLStick,
  ButtonHome,
  ButtonCapture,

  ButtonDPadDown,
  ButtonDPadUp,
  ButtonDPadRight,
  ButtonDPadLeft,
  ButtonSR_JCR,
  ButtonSL_JCR,
  ButtonL,
  ButtonZL,

  NUM_BUTTONS
};

class ProController {
  public:
    ProController();

    void begin();
    void loop();
    void pressButton(ProControllerButton button);
    void releaseButton(ProControllerButton button);
};

#endif