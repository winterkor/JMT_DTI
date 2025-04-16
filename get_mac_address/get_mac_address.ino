#include <WiFi.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"      // ✅ This is what you were missing

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);  // Ensure we use the STA MAC (used by ESP-NOW)
  String macStr = WiFi.macAddress();
  
  Serial.println("=====================================");
  Serial.println("ESP32 Tool MAC Address (for ESP-NOW):");
  Serial.println(macStr);
  Serial.println("=====================================");

  // Also print in byte-array format
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Serial.print("Byte array: { ");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    if (mac[i] < 16) Serial.print("0");  // Add leading zero
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(", ");
  }
  Serial.println(" }");
}

void loop() {
  // Nothing here — just prints MAC once
}