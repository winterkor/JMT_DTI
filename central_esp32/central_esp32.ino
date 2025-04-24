#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include "esp_wifi.h"

const int LED_PIN = 2;
const char* ssid = "Luck";
const char* password = "winterwifinotfreebutfree";
const char* middlemanName = "Beacon2";

WebServer server(80);
int lastRSSI = -999;

// Tool definitions
uint8_t toolMACs[][6] = {
  {0x98, 0x88, 0xE0, 0xC9, 0x8C, 0x90},
  {0x98, 0x88, 0xE0, 0xC9, 0xA6, 0x68},
  {0x98, 0x88, 0xE0, 0xC9, 0x98, 0xF0},
  {0x98, 0x88, 0xE0, 0xC9, 0x7C, 0xD8},
  {0x98, 0x88, 0xE0, 0xCB, 0x28, 0x40}
};
String toolNames[] = {
  "Tool_01_clawhammer",
  "Tool_03_hexscrewdriver",
  "Tool_02_rubberhammer",
  "Tool_05_digitalmultimeter",
  "Tool_06_clamp"
};

void sendPingToTool(String toolCallname) {
  lastRSSI = -999;

  int index = -1;
  for (int i = 0; i < sizeof(toolNames) / sizeof(toolNames[0]); i++) {
    if (toolNames[i] == toolCallname) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    Serial.println("❌ Tool not found in tool list");
    return;
  }

  const uint8_t* toolMAC = toolMACs[index];

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, toolMAC, 6);
  peerInfo.channel = 6; // Must match the tool device
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(toolMAC)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("❌ Failed to add peer");
      return;
    }
    delay(100);
  }

  Serial.print("📤 Sending ping to: ");
  Serial.println(toolCallname);
  esp_now_send(toolMAC, (uint8_t*)"ping", 4);
  esp_now_del_peer(toolMAC);
}

void handlePingTool() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Only POST allowed");
    return;
  }

  String body = server.arg("plain");
  Serial.println("📥 Received POST: " + body);

  int start = body.indexOf(":\"") + 2;
  int end = body.indexOf("\"", start);
  String toolCallname = body.substring(start, end);
  Serial.println("🔎 Tool requested: " + toolCallname);

  sendPingToTool(toolCallname);

  String response = "{";
  response += "\"middleman\":\"" + String(middlemanName) + "\",";
  response += "\"rssi\":" + String(lastRSSI) + "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST");
  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Connected. IP: " + WiFi.localIP().toString());

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb([](const esp_now_recv_info_t*, const uint8_t*, int) {
    // No response handling
  });

  server.on("/ping_tool", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    server.send(204);
  });

  server.on("/ping_tool", handlePingTool);
  server.begin();
  Serial.println("🌐 HTTP server started.");
}

void loop() {
  server.handleClient();
}