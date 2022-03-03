#include "ProController.h"
#include "esp_log.h"
#include "freertos/task.h"

esp_hidd_app_param_t app_param;
esp_hidd_qos_param_t both_qos;

const char *TAG = "ProController";
bool paired = false;
bool connected = false;
bool buttonStates[NUM_BUTTONS];
bool reportDirty = false;

SemaphoreHandle_t reportMutex = xSemaphoreCreateMutex();
TaskHandle_t sendButtonsHandle = NULL;

uint8_t hid_descriptor_pro_controller[] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05, // Usage (Game Pad)
    0xA1, 0x01, // Collection (Application)
    // Padding
    0x95, 0x03, //     REPORT_COUNT = 3
    0x75, 0x08, //     REPORT_SIZE = 8
    0x81, 0x03, //     INPUT = Cnst,Var,Abs
    // Sticks
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x09, 0x32,       //   Usage (Z)
    0x09, 0x35,       //   Usage (Rz)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x04,       //   Report Count (4)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    // DPAD
    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x07,       //   Logical Maximum (7)
    0x35, 0x00,       //   Physical Minimum (0)
    0x46, 0x3B, 0x01, //   Physical Maximum (315)
    0x65, 0x14,       //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,       //   Report Size (4)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x42,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    // Buttons
    0x65, 0x00, //   Unit (None)
    0x05, 0x09, //   Usage Page (Button)
    0x19, 0x01, //   Usage Minimum (0x01)
    0x29, 0x0E, //   Usage Maximum (0x0E)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x0E, //   Report Count (14)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    // Padding
    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,       //   Usage (0x20)
    0x75, 0x06,       //   Report Size (6)
    0x95, 0x01,       //   Report Count (1)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x7F,       //   Logical Maximum (127)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    // Triggers
    0x05, 0x01,       //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x33,       //   Usage (Rx)
    0x09, 0x34,       //   Usage (Ry)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,
    0xc0};

ProController::ProController()
{
  app_param.name = "Tetris Gamepad";
  app_param.description = "Tetris Gamepad";
  app_param.provider = "ESP32";
  app_param.subclass = 0x8;
  app_param.desc_list = hid_descriptor_pro_controller;
  app_param.desc_list_len = sizeof(hid_descriptor_pro_controller);

  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttonStates[i] = false;
  }
}

static uint8_t but1_send = 0;
static uint8_t but2_send = 0;
static uint8_t but3_send = 0;

void updateReport() {
  but1_send = 0;
  but2_send = 0;
  but3_send = 0;

  but1_send |= buttonStates[ButtonY] << 0;
  but1_send |= buttonStates[ButtonX] << 1;
  but1_send |= buttonStates[ButtonB] << 2;
  but1_send |= buttonStates[ButtonA] << 3;
  but1_send |= buttonStates[ButtonSR_JCL] << 4;
  but1_send |= buttonStates[ButtonSL_JCL] << 5;
  but1_send |= buttonStates[ButtonR] << 6;
  but1_send |= buttonStates[ButtonZR] << 7;

  but2_send |= buttonStates[ButtonMinus] << 0;
  but2_send |= buttonStates[ButtonPlus] << 1;
  but2_send |= buttonStates[ButtonRStick] << 2;
  but2_send |= buttonStates[ButtonLStick] << 3;
  but2_send |= buttonStates[ButtonHome] << 4;
  but2_send |= buttonStates[ButtonCapture] << 5;

  but3_send |= buttonStates[ButtonDPadDown] << 0;
  but3_send |= buttonStates[ButtonDPadUp] << 1;
  but3_send |= buttonStates[ButtonDPadRight] << 2;
  but3_send |= buttonStates[ButtonDPadLeft] << 3;
  but3_send |= buttonStates[ButtonSR_JCR] << 4;
  but3_send |= buttonStates[ButtonSL_JCR] << 5;
  but3_send |= buttonStates[ButtonL] << 6;
  but3_send |= buttonStates[ButtonZL] << 7;

  reportDirty = false;
}

// Switch button report example //         batlvl       Buttons              Lstick           Rstick
// static uint8_t report30[] = {0x30, 0x00, 0x90,   0x00, 0x00, 0x00,   0x00, 0x00, 0x00,   0x00, 0x00, 0x00};
static uint8_t report30[] = {
    0x30,
    0x0,
    0x80,
    0, // but1
    0, // but2
    0, // but3
    0, // Ls
    0, // Ls
    0, // Ls
    0, // Rs
    0, // Rs
    0, // Rs
    0x08};
static uint8_t emptyReport[] = {
    0x0,
    0x0};

static uint8_t timer = 0;

//Calibration
// static int lxcalib = 0;
// static int lycalib = 0;
// static int cxcalib = 0;
// static int cycalib = 0;
// static int lcalib = 0;
// static int rcalib = 0;

//Buttons and sticks
// static uint8_t lx_send = 0;
// static uint8_t ly_send = 0;
// static uint8_t cx_send = 0;
// static uint8_t cy_send = 0;
// static uint8_t lt_send = 0;
// static uint8_t rt_send = 0;

void sendButtons() {
  xSemaphoreTake(reportMutex, portMAX_DELAY);
  if (reportDirty) updateReport();

  report30[1] = timer;

  // buttons
  report30[3] = but1_send;
  report30[4] = but2_send;
  report30[5] = but3_send;

  // encode left stick
  report30[6] = 0x6F; // (lx_send << 4) & 0xF0;
  report30[7] = 0xC8; // (lx_send & 0xF0) >> 4;
  report30[8] = 0x77; //ly_send;

  // encode right stick
  report30[9] = 0x16; //(cx_send << 4) & 0xF0;
  report30[10] = 0xD8;//(cx_send & 0xF0) >> 4;
  report30[11] = 0x7D;//cy_send;

  timer += 1;
  if (timer == 255)
    timer = 0;
  xSemaphoreGive(reportMutex);

  if (!paired)
  {
    emptyReport[1] = timer;
    esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(emptyReport), emptyReport);
    vTaskDelay(100);
  }
  else
  {
    esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(report30), report30);
    vTaskDelay(15);
  }
}

void sendButtonsTask(void* pvParameters)
{
  for (;;) {
    sendButtons();
  }
}

static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
  switch (event)
  {
  case ESP_BT_GAP_DISC_RES_EVT:
    // ESP_LOGI(SPP_TAG, "ESP_BT_GAP_DISC_RES_EVT");
    // esp_log_buffer_hex(SPP_TAG, param->disc_res.bda, ESP_BD_ADDR_LEN);
    break;
  case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
    // ESP_LOGI(SPP_TAG, "ESP_BT_GAP_DISC_STATE_CHANGED_EVT");
    break;
  case ESP_BT_GAP_RMT_SRVCS_EVT:
    // ESP_LOGI(SPP_TAG, "ESP_BT_GAP_RMT_SRVCS_EVT");
    // ESP_LOGI(SPP_TAG, "%d", param->rmt_srvcs.num_uuids);
    break;
  case ESP_BT_GAP_RMT_SRVC_REC_EVT:
    // ESP_LOGI(SPP_TAG, "ESP_BT_GAP_RMT_SRVC_REC_EVT");
    break;
  case ESP_BT_GAP_AUTH_CMPL_EVT:
  {
    if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
    {
      // ESP_LOGI(SPP_TAG, "authentication success: %s", param->auth_cmpl.device_name);
      // esp_log_buffer_hex(SPP_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
    }
    else
    {
      // ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
    }
    break;
  }

  default:
    break;
  }
}

/// Switch Replies
static uint8_t reply02[] = {0x21, 0x01, 0x40, 0x00, 0x00, 0x00, 0xe6, 0x27, 0x78, 0xab, 0xd7, 0x76, 0x00, 0x82, 0x02, 0x03, 0x48, 0x03, 0x02, 0xD8, 0xA0, 0x1D, 0x40, 0x15, 0x66, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply08[] = {0x21, 0x02, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply03[] = {0x21, 0x05, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply04[] = {0x21, 0x06, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x83, 0x04, 0x00, 0x6a, 0x01, 0xbb, 0x01, 0x93, 0x01, 0x95, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply1060[] = {0x21, 0x03, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x00, 0x60, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply1050[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x50, 0x60, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply1080[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x80, 0x60, 0x00, 0x00, 0x18, 0x5e, 0x01, 0x00, 0x00, 0xf1, 0x0f,
                              0x19, 0xd0, 0x4c, 0xae, 0x40, 0xe1,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0x00, 0x00};
static uint8_t reply1098[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x98, 0x60, 0x00, 0x00, 0x12, 0x19, 0xd0, 0x4c, 0xae, 0x40, 0xe1,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0x00, 0x00};
// User analog stick calib
static uint8_t reply1010[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x10, 0x80, 0x00, 0x00, 0x18, 0x00, 0x00};
static uint8_t reply103D[] = {0x21, 0x05, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x3D, 0x60, 0x00, 0x00, 0x19, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply1020[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x20, 0x60, 0x00, 0x00, 0x18, 0x00, 0x00};
static uint8_t reply4001[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply4801[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply3001[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply3333[] = {0x21, 0x03, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void esp_bt_hidd_cb(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
{
  switch (event)
  {
  case ESP_HIDD_INIT_EVT:
    if (param->init.status == ESP_HIDD_SUCCESS)
    {
      ESP_LOGI(TAG, "setting hid parameters");
      esp_bt_hid_device_register_app(&app_param, &both_qos, &both_qos);
    }
    else
    {
      ESP_LOGE(TAG, "init hidd failed!");
    }
    break;
  case ESP_HIDD_DEINIT_EVT:
    break;
  case ESP_HIDD_REGISTER_APP_EVT:
    if (param->register_app.status == ESP_HIDD_SUCCESS)
    {
      ESP_LOGI(TAG, "setting hid parameters success!");
      ESP_LOGI(TAG, "setting to connectable, discoverable");
      esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
      if (param->register_app.in_use && param->register_app.bd_addr != NULL)
      {
        ESP_LOGI(TAG, "start virtual cable plug!");
        esp_bt_hid_device_connect(param->register_app.bd_addr);
      }
    }
    else
    {
      ESP_LOGE(TAG, "setting hid parameters failed!");
    }
    break;
  case ESP_HIDD_UNREGISTER_APP_EVT:
    if (param->unregister_app.status == ESP_HIDD_SUCCESS)
    {
      ESP_LOGI(TAG, "unregister app success!");
    }
    else
    {
      ESP_LOGE(TAG, "unregister app failed!");
    }
    break;
  case ESP_HIDD_OPEN_EVT:
    if (param->open.status == ESP_HIDD_SUCCESS)
    {
      if (param->open.conn_status == ESP_HIDD_CONN_STATE_CONNECTING)
      {
        ESP_LOGI(TAG, "connecting...");
      }
      else if (param->open.conn_status == ESP_HIDD_CONN_STATE_CONNECTED)
      {
        connected = true;
        ESP_LOGI(TAG, "connected to %02x:%02x:%02x:%02x:%02x:%02x", param->open.bd_addr[0],
                 param->open.bd_addr[1], param->open.bd_addr[2], param->open.bd_addr[3], param->open.bd_addr[4],
                 param->open.bd_addr[5]);

        ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
        esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

        if(sendButtonsHandle != NULL)
        {
          vTaskDelete(sendButtonsHandle);
          sendButtonsHandle = NULL;
        }

        xTaskCreatePinnedToCore(sendButtonsTask, "sendButtons", 2048, NULL, 2, &sendButtonsHandle, 1);

        // clear blinking LED - solid
        // vTaskDelete(BlinkHandle);
        // BlinkHandle = NULL;
        // gpio_set_level(LED_GPIO, 1);
        // start solid
        // xSemaphoreTake(xSemaphore, portMAX_DELAY);
        // connected = true;
        // xSemaphoreGive(xSemaphore);
        // restart send_task
        // if (SendingHandle != NULL)
        // {
        //   vTaskDelete(SendingHandle);
        //   SendingHandle = NULL;
        // }
        // xTaskCreatePinnedToCore(send_task, "send_task", 2048, NULL, 2, &SendingHandle, 0);
      }
      else
      {
        ESP_LOGE(TAG, "unknown connection status");
      }
    }
    else
    {
      ESP_LOGE(TAG, "open failed!");
    }
    break;
  case ESP_HIDD_CLOSE_EVT:
    ESP_LOGI(TAG, "ESP_HIDD_CLOSE_EVT");
    if (param->close.status == ESP_HIDD_SUCCESS)
    {
      if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTING)
      {
        ESP_LOGI(TAG, "disconnecting...");
      }
      else if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTED)
      {
        connected = false;
        ESP_LOGI(TAG, "disconnected!");

        if (sendButtonsHandle != NULL)
        {
          vTaskDelete(sendButtonsHandle);
          sendButtonsHandle = NULL;
        }

        ESP_LOGI(TAG, "making self discoverable and connectable again.");
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

        // xTaskCreate(startBlink, "blink_task", 1024, NULL, 1, &BlinkHandle);
        // start blink
        // xSemaphoreTake(xSemaphore, portMAX_DELAY);
        // connected = false;
        // xSemaphoreGive(xSemaphore);
      }
      else
      {
        ESP_LOGE(TAG, "unknown connection status");
      }
    }
    else
    {
      ESP_LOGE(TAG, "close failed!");
    }
    break;
  case ESP_HIDD_SEND_REPORT_EVT:
    // ESP_LOGI(TAG, "ESP_HIDD_SEND_REPORT_EVT id:0x%02x, type:%d", param->send_report.report_id,
    //          param->send_report.report_type);
    break;
  case ESP_HIDD_REPORT_ERR_EVT:
    ESP_LOGI(TAG, "ESP_HIDD_REPORT_ERR_EVT");
    break;
  case ESP_HIDD_GET_REPORT_EVT:
    ESP_LOGI(TAG, "ESP_HIDD_GET_REPORT_EVT id:0x%02x, type:%d, size:%d", param->get_report.report_id,
             param->get_report.report_type, param->get_report.buffer_size);
    break;
  case ESP_HIDD_SET_REPORT_EVT:
    ESP_LOGI(TAG, "ESP_HIDD_SET_REPORT_EVT");
    break;
  case ESP_HIDD_SET_PROTOCOL_EVT:
    ESP_LOGI(TAG, "ESP_HIDD_SET_PROTOCOL_EVT");
    if (param->set_protocol.protocol_mode == ESP_HIDD_BOOT_MODE)
    {
      ESP_LOGI(TAG, "  - boot protocol");
    }
    else if (param->set_protocol.protocol_mode == ESP_HIDD_REPORT_MODE)
    {
      ESP_LOGI(TAG, "  - report protocol");
    }
    break;
  case ESP_HIDD_INTR_DATA_EVT:
    ESP_LOGI(TAG, "ESP_HIDD_INTR_DATA_EVT");

    // switch pairing sequence
    if (param->intr_data.len == 49)
    {
      if (param->intr_data.data[10] == 2)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply02), reply02);
      }
      if (param->intr_data.data[10] == 8)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply08), reply08);
      }
      if (param->intr_data.data[10] == 16 && param->intr_data.data[11] == 0 && param->intr_data.data[12] == 96)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply1060), reply1060);
      }
      if (param->intr_data.data[10] == 16 && param->intr_data.data[11] == 80 && param->intr_data.data[12] == 96)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply1050), reply1050);
      }
      if (param->intr_data.data[10] == 3)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply03), reply03);
      }
      if (param->intr_data.data[10] == 4)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply04), reply04);
      }
      if (param->intr_data.data[10] == 16 && param->intr_data.data[11] == 128 && param->intr_data.data[12] == 96)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply1080), reply1080);
      }
      if (param->intr_data.data[10] == 16 && param->intr_data.data[11] == 152 && param->intr_data.data[12] == 96)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply1098), reply1098);
      }
      if (param->intr_data.data[10] == 16 && param->intr_data.data[11] == 16 && param->intr_data.data[12] == 128)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply1010), reply1010);
      }
      if (param->intr_data.data[10] == 16 && param->intr_data.data[11] == 61 && param->intr_data.data[12] == 96)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply103D), reply103D);
      }
      if (param->intr_data.data[10] == 16 && param->intr_data.data[11] == 32 && param->intr_data.data[12] == 96)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply1020), reply1020);
      }
      if (param->intr_data.data[10] == 64 && param->intr_data.data[11] == 1)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply4001), reply4001);
      }
      if (param->intr_data.data[10] == 72 && param->intr_data.data[11] == 1)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply4801), reply4801);
      }
      if (param->intr_data.data[10] == 48 && param->intr_data.data[11] == 1)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply3001), reply3001);
      }

      if (param->intr_data.data[10] == 33 && param->intr_data.data[11] == 33)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply3333), reply3333);
        paired = true;
      }
      if (param->intr_data.data[10] == 64 && param->intr_data.data[11] == 2)
      {
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xa1, sizeof(reply4001), reply4001);
      }
      ESP_LOGI(TAG, "got an interrupt report from host, subcommand: %d  %d  %d Length: %d", param->intr_data.data[10], param->intr_data.data[11], param->intr_data.data[12], param->intr_data.len);
    }
    else
    {
      ESP_LOGI("heap size:", "%d", xPortGetFreeHeapSize());
      ESP_LOGI(TAG, "pairing packet size != 49, subcommand: %d  %d  %d  Length: %d", param->intr_data.data[10], param->intr_data.data[11], param->intr_data.data[12], param->intr_data.len);
    }
    break;
  case ESP_HIDD_VC_UNPLUG_EVT:
    ESP_LOGI(TAG, "ESP_HIDD_VC_UNPLUG_EVT");
    if (param->vc_unplug.status == ESP_HIDD_SUCCESS)
    {
      if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTED)
      {
        ESP_LOGI(TAG, "disconnected!");

        ESP_LOGI(TAG, "making self discoverable and connectable again.");
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
      }
      else
      {
        ESP_LOGE(TAG, "unknown connection status");
      }
    }
    else
    {
      ESP_LOGE(TAG, "close failed!");
    }
    break;
  default:
    break;
  }
}

void ProController::begin()
{
  esp_bt_controller_disable();

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  bt_cfg.mode = ESP_BT_MODE_CLASSIC_BT;

  esp_bt_mem_release(ESP_BT_MODE_BLE);

  esp_err_t ret;
  if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
  {
    ESP_LOGE(TAG, "initialize controller failed: %s\n", esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
  {
    ESP_LOGE(TAG, "enable controller failed: %s\n", esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bluedroid_init()) != ESP_OK)
  {
    ESP_LOGE(TAG, "initialize bluedroid failed: %s\n", esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bluedroid_enable()) != ESP_OK)
  {
    ESP_LOGE(TAG, "enable bluedroid failed: %s\n", esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK)
  {
    ESP_LOGE(TAG, "gap register failed: %s\n", esp_err_to_name(ret));
    return;
  }

  esp_bt_dev_set_device_name("Pro Controller");
  ESP_LOGD(TAG, "set device name");

  esp_bt_cod_t cod;
  cod.major = 0x05;
  cod.minor = 0b0000010;

  esp_bt_gap_set_cod(cod, ESP_BT_SET_COD_MAJOR_MINOR);
  ESP_LOGD(TAG, "set cod");

  esp_bt_hid_device_register_callback(esp_bt_hidd_cb);
  ESP_LOGD(TAG, "set callbacks");

  esp_bt_hid_device_init();
  ESP_LOGD(TAG, "device init");

  auto addr = esp_bt_dev_get_address();
  for (int z = 0; z < 6; z++)
    reply02[z + 19] = addr[z];

  ESP_LOGD(TAG, "exiting begin");
}

void ProController::loop() {
  if (connected) vTaskDelay(1);
  if (!connected) vTaskDelay(10);
}

void immediatelySendButtons() {
  if (sendButtonsHandle != NULL)
    xTaskAbortDelay(sendButtonsHandle);
}

void ProController::pressButton(ProControllerButton btn) {
  buttonStates[btn] = true;
  reportDirty = true;
  immediatelySendButtons();
}

void ProController::releaseButton(ProControllerButton btn) {
  buttonStates[btn] = false;
  reportDirty = true;
  immediatelySendButtons();
}
