// OPTIMIZED MIDDLEMAN CODE WITH BUZZ RETRY OVER 30 SECONDS
#include <WiFi.h>
#include "esp_wifi.h"
#include <esp_now.h>
#include <WebServer.h>

const char* ssid = "Luck";
const char* password = "winterwifinotfreebutfree";
bool sendSuccess = false;

WebServer server(80);

// Tool MAC addresses
uint8_t toolMACs[][6] = {
  {0x98, 0x88, 0xE0, 0xC9, 0x8C, 0x90},
  {0x98, 0x88, 0xE0, 0xC9, 0xA6, 0x68}, // Tool_05
  {0x98, 0x88, 0xE0, 0xC9, 0x98, 0xF0}  // Tool_02
};
String toolNames[] = {"Tool_01_clawhammer","Tool_05_digitalmultimeter", "Tool_02_rubberhammer"};

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  if (status == ESP_NOW_SEND_SUCCESS) {
    sendSuccess = true;  // ✅ set success flag
  }
}

void sendBuzzToTool(String toolCallname) {
  int index = -1;
  for (int i = 0; i < sizeof(toolNames) / sizeof(toolNames[0]); i++) {
    if (toolNames[i] == toolCallname) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    Serial.println("Tool not found in MAC list.");
    return;
  }

  Serial.println("Sending buzz command repeatedly for up to 30 seconds...");
  sendSuccess = false;  // Reset before sending

  unsigned long startTime = millis();
  while (millis() - startTime < 30000) {
    esp_err_t result = esp_now_send(toolMACs[index], (uint8_t *)"buzz", 4);
    if (result == ESP_OK) {
      Serial.println("Buzz command queued.");
    } else {
      Serial.println("Failed to queue buzz command.");
    }

    // Wait for onDataSent callback to update sendSuccess
    unsigned long waitStart = millis();
    while (!sendSuccess && millis() - waitStart < 300) {
      delay(10);  // Give time for callback to fire
    }

    if (sendSuccess) {
      Serial.println("Confirmed send success. Exiting retry loop.");
      break;  // ✅ Exit only when confirmed SUCCESS
    }

    delay(700);  // Rest of the 1-second interval
  }
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

  sendBuzzToTool(toolCallname);

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

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  for (int i = 0; i < sizeof(toolMACs) / sizeof(toolMACs[0]); i++) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, toolMACs[i], 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("Peer added");
    } else {
      Serial.println("Failed to add peer or already added");
    }
  }

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