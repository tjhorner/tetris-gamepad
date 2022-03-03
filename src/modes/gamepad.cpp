#include "gamepad.h"

void sendReportTask(void* pvParameters) {
  GamepadMode* mode = (GamepadMode*)pvParameters;
  while (true) {
#if EXTRA_LOGS
    Serial.printf("sending report %ld\n", millis());
#endif
    mode->sendReport();

    // reports do not need to be sent often,
    // and this task fires every time a button
    // is pressed or released anyway
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void GamepadMode::setup() {
  this->mutex = xSemaphoreCreateMutex();

  gamepad.deviceName = "TGP (Gamepad)";
  gamepad.deviceManufacturer = "TJ Horner";
  gamepad.begin(false);

  xTaskCreatePinnedToCore(sendReportTask, "sendReportTask", 2048, (void *)this, 10, &reportTaskHandle, 1);
}

void GamepadMode::teardown() {
  gamepad.end();
  if (reportTaskHandle != NULL)
    vTaskDelete(reportTaskHandle);
}

void GamepadMode::pressButton(ButtonType button) {
  gamepad.press(this->buttonMap[button]);
  if (reportTaskHandle != NULL)
    xTaskAbortDelay(reportTaskHandle);
}

void GamepadMode::releaseButton(ButtonType button) {
  gamepad.release(this->buttonMap[button]);
  if (reportTaskHandle != NULL)
    xTaskAbortDelay(reportTaskHandle);
}

void GamepadMode::sendReport() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  gamepad.sendReport();
  xSemaphoreGive(this->mutex);
}