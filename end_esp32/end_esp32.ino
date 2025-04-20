#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

const int LED_PIN = 0;
const int BUZZER_PIN = 1;
bool responded = false;

BLEAdvertising* pAdvertising;

void startBLEAdvertisement() {
  BLEDevice::init("Tool_03_hexscrewdriver");
  BLEServer* pServer = BLEDevice::createServer();
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // recommended for iOS
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
  Serial.println("📡 BLE Advertising started...");
}

void stopBLEAdvertisement() {
  if (pAdvertising) {
    pAdvertising->stop();
    BLEDevice::deinit();
    Serial.println("🛑 BLE Advertising stopped.");
  }
}

void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  if (responded) return;

  String msg = "";
  for (int i = 0; i < len; i++) msg += (char)data[i];

  if (msg == "ping") {
    const uint8_t* senderMac = recv_info->src_addr;
    int rssi = recv_info->rx_ctrl->rssi;

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             senderMac[0], senderMac[1], senderMac[2],
             senderMac[3], senderMac[4], senderMac[5]);
    Serial.printf("📡 Ping received from %s | RSSI = %d\n", macStr, rssi);

    startBLEAdvertisement();
    
    // Feedback (buzzer/light)
    digitalWrite(LED_PIN, HIGH);
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH); delay(600);
      digitalWrite(BUZZER_PIN, LOW);  delay(600);
    }
    digitalWrite(LED_PIN, LOW);
    stopBLEAdvertisement();
    responded = true;
    Serial.println("😴 Going to deep sleep...");
    esp_sleep_enable_timer_wakeup(30LL * 1000000);
    esp_deep_sleep_start();
  }
}

void setup() {
  delay(2000);  // Grace period on boot
  Serial.begin(115200);
  Serial.println("🔧 ESP32-C3 Tool booted");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 📡 Wi-Fi Setup (Channel 6)
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);                   // Needed to force channel
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);   // Match middleman channel
  esp_wifi_set_promiscuous(false);
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();
  

  // 🚀 Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  Serial.println("✅ Ready to receive ping...");
}

void loop() {
  // Nothing here; handled in callback
}