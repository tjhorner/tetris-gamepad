#include "ProController.h"

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
  btSerial.begin();
  app_param.name = "Tetris Gamepad";
  app_param.description = "Tetris Gamepad";
  app_param.provider = "ESP32";
  app_param.subclass = 0x8;
  app_param.desc_list = hid_descriptor_pro_controller;
  app_param.desc_list_len = sizeof(hid_descriptor_pro_controller);
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

void ProController::begin()
{
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_bt_mem_release(ESP_BT_MODE_BLE);

  esp_err_t ret;
  if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
  {
    // ESP_LOGE(TAG, "initialize controller failed: %s\n",  esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
  {
    // ESP_LOGE(TAG, "enable controller failed: %s\n",  esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bluedroid_init()) != ESP_OK)
  {
    // ESP_LOGE(TAG, "initialize bluedroid failed: %s\n",  esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bluedroid_enable()) != ESP_OK)
  {
    // ESP_LOGE(TAG, "enable bluedroid failed: %s\n",  esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK)
  {
    // ESP_LOGE(TAG, "gap register failed: %s\n", esp_err_to_name(ret));
    return;
  }

  esp_bt_dev_set_device_name("Pro Controller");

  esp_bt_cod_t cod;
  cod.major = 0x05;
  cod.minor = 0b0000010;

  esp_bt_gap_set_cod(cod, ESP_BT_SET_COD_MAJOR_MINOR);
}