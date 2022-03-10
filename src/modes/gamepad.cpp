#include "gamepad.h"

void sendReportTask(void* pvParameters) {
  GamepadMode* mode = (GamepadMode*)pvParameters;
  while (true) {
    ESP_LOGV("sendReportTask", "sending report");
    mode->sendReport();
    vTaskDelay(15 / portTICK_PERIOD_MS);
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
  this->reportDirty = true;

  if (reportTaskHandle != NULL)
    xTaskAbortDelay(reportTaskHandle);
}

void GamepadMode::releaseButton(ButtonType button) {
  gamepad.release(this->buttonMap[button]);
  this->reportDirty = true;

  if (reportTaskHandle != NULL)
    xTaskAbortDelay(reportTaskHandle);
}

void GamepadMode::sendReport() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
  if (this->reportDirty)
    gamepad.sendReport();
  xSemaphoreGive(this->mutex);
}