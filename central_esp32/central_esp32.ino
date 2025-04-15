// REVISED MIDDLEMAN CODE WITH ESP-NOW INSTEAD OF BLE
#include <WiFi.h>
#include "esp_wifi.h"
#include <esp_now.h>
#include <WebServer.h>


const char* ssid = "Luck";
const char* password = "winterwifinotfreebutfree";

WebServer server(80);

// Tool MAC addresses
// Replace with your real tool MAC addresses
uint8_t toolMACs[][6] = {
  {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC}, // Tool_01
  {0x24, 0x6F, 0x28, 0xDD, 0xEE, 0xFF}  // Tool_02
};
String toolNames[] = {"Tool_01_clawhammer", "Tool_02_rubberhammer"};

// ESP-NOW send callback
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void sendBuzzToTool(String toolCallname) {
  for (int i = 0; i < sizeof(toolNames) / sizeof(toolNames[0]); i++) {
    if (toolNames[i] == toolCallname) {
      esp_err_t result = esp_now_send(toolMACs[i], (uint8_t *)"buzz", 4);
      if (result == ESP_OK) {
        Serial.println("Buzz command sent to: " + toolCallname);
      } else {
        Serial.println("Failed to send buzz command.");
      }
      return;
    }
  }
  Serial.println("Tool not found in MAC list.");
}

void handleFindTool() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Only POST allowed");
    return;
  }

  String body = server.arg("plain");
  Serial.println("Received POST body: " + body);

  int start = body.indexOf(":\"") + 2;
  int end = body.indexOf("\"", start);
  String toolCallname = body.substring(start, end);

  Serial.println("Tool: " + toolCallname);

  WiFi.disconnect(true);
  delay(100);
  esp_wifi_stop();
  esp_now_init();
  esp_now_register_send_cb(onDataSent);

  for (int i = 0; i < sizeof(toolMACs) / sizeof(toolMACs[0]); i++) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, toolMACs[i], 6);
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("Peer added");
    } else {
      Serial.println("Failed to add peer");
    }
  }

  sendBuzzToTool(toolCallname);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST");
  server.send(200, "application/json", "{\"status\":\"buzz_sent\"}");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi. IP: " + WiFi.localIP().toString());

  server.on("/find_tool", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    server.send(204);
  });

  server.on("/find_tool", handleFindTool);
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}