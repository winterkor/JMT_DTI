#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"

const int LED_PIN = 8;
const int BUZZER_PIN = 4;

bool buzzNow = false;

// Callback when ESP-NOW data is received
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)incomingData[i];
  }

  Serial.print("From MAC: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
           recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);
  Serial.println(macStr);

  Serial.println("ESP-NOW Message: " + message);

  if (message == "buzz") {
    buzzNow = true;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);  // Set to match middleman channel
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  Serial.println("Ready to receive ESP-NOW messages...");
}

void loop() {
  if (buzzNow) {
    digitalWrite(LED_PIN, HIGH);
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
      delay(1000);
    }
    digitalWrite(LED_PIN, LOW);
    buzzNow = false;

    Serial.println("Going to deep sleep for 30 seconds...");
    esp_sleep_enable_timer_wakeup(30LL * 1000000);  // 30 seconds
    esp_deep_sleep_start();
  }
}
