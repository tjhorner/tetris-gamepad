#include "randmac.h"

void getBaseAddress(uint8_t* out, bool forceReset) {
  EEPROM.readBytes(ADDR_BASE_ADDRESS, out, 6);

  if (out[0] == 0xFF || forceReset) {
    for (int i = 0; i < 6; i++)
      out[i] = esp_random() % 255;

    EEPROM.writeBytes(ADDR_BASE_ADDRESS, out, 6);
    EEPROM.commit();
  }
}

void setMacAddress(int index, bool forceReset) {
  randomSeed(index);

  uint8_t mac_addr[6];
  getBaseAddress(mac_addr, forceReset);

  mac_addr[5] = mac_addr[5] + index;

  Serial.printf("New base MAC: %x:%x:%x:%x:%x:%x\n",
    mac_addr[0], mac_addr[1], mac_addr[2],
    mac_addr[3], mac_addr[4], mac_addr[5]);

  esp_err_t result = esp_base_mac_addr_set(mac_addr);

  if (result != ESP_OK)
    Serial.printf("Failed to set MAC address (error %d / %x)\n", result, result);
}