#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"

const int LED_PIN = 8;
const int BUZZER_PIN = 4;

bool responded = false;

void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  if (responded) return;

  String msg = "";
  for (int i = 0; i < len; i++) msg += (char)data[i];

  if (msg == "ping") {
    const uint8_t* senderMac = recv_info->src_addr;
    int rssi = recv_info->rx_ctrl->rssi;

    // 📡 Print who sent the ping
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             senderMac[0], senderMac[1], senderMac[2],
             senderMac[3], senderMac[4], senderMac[5]);
    Serial.print("📡 Ping received from: ");
    Serial.println(macStr);
    Serial.printf("📶 RSSI = %d\n", rssi);

    // ✅ Add peer dynamically (safe even if already added)
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, senderMac, 6);
    peerInfo.channel = 6;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
    delay(300);

    // ✉️ Reply with pong and RSSI
    String reply = "pong:" + String(rssi);
    esp_err_t res = esp_now_send(senderMac, (uint8_t*)reply.c_str(), reply.length());
    Serial.println(res == ESP_OK ? "✅ Sent pong!" : "❌ Failed to send pong!");
    Serial.println(reply);
    Serial.println(WiFi.channel());
    esp_now_del_peer(senderMac);  // Clean up!

    // Optional: Visual and audible feedback
    digitalWrite(LED_PIN, LOW);
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH); delay(500);
      digitalWrite(BUZZER_PIN, LOW); delay(500);
    }
    digitalWrite(LED_PIN, HIGH);

    responded = true;
    delay(100);
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
  digitalWrite(LED_PIN, HIGH);

  // 📡 Wi-Fi Setup (Channel 6)
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);                   // Needed to force channel
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);   // Match middleman channel
  esp_wifi_set_promiscuous(false);
  WiFi.disconnect();

  // 🚀 Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  Serial.println("✅ Ready to receive ping...");
  Serial.println(WiFi.channel());
}

void loop() {
  // Nothing here; everything runs via interrupt callback
}