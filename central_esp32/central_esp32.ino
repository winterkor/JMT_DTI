#include <WiFi.h>


#include "esp_wifi.h"
#include <esp_now.h>
#include <WebServer.h>

const char* ssid = "Luck";
const char* password = "winterwifinotfreebutfree";
const char* middlemanName = "M2";

WebServer server(80);
int lastRSSI = -999;
bool gotReply = false;

// Tool MAC addresses
uint8_t toolMACs[][6] = {
  {0x98, 0x88, 0xE0, 0xC9, 0x8C, 0x90},
  {0x98, 0x88, 0xE0, 0xC9, 0xA6, 0x68},
  {0x98, 0x88, 0xE0, 0xC9, 0x98, 0xF0}
};
String toolNames[] = {"Tool_01_clawhammer", "Tool_05_digitalmultimeter", "Tool_02_rubberhammer"};

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  String msg = "";
  for (int i = 0; i < len; i++) msg += (char)incomingData[i];
  Serial.println(msg);

  if (msg.startsWith("pong:")) {
    lastRSSI = msg.substring(5).toInt();
    gotReply = true;
    Serial.printf("✅ Got pong! RSSI = %d\n", lastRSSI);
  }
}

void sendPingToTool(String toolCallname) {
  int index = -1;
  for (int i = 0; i < sizeof(toolNames) / sizeof(toolNames[0]); i++) {
    if (toolNames[i] == toolCallname) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    Serial.println("❌ Tool not found in MAC list.");
    return;
  }

  const uint8_t* toolMAC = toolMACs[index];

  gotReply = false;
  lastRSSI = -999;

  // Add peer if needed
  if (!esp_now_is_peer_exist(toolMAC)) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, toolMAC, 6);
    peerInfo.channel = 6;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("✅ Tool peer added before sending.");
      delay(200);  // ⏳ Let peer sync
    } else {
      Serial.println("❌ Failed to add tool peer.");
      return;
    }
  }

  Serial.println("📡 Sending ping...");
  esp_err_t result = esp_now_send(toolMAC, (uint8_t*)"ping", 4);
  if (result == ESP_OK) {
    Serial.println("📤 Ping sent!");
  } else {
    Serial.println("❌ Failed to send ping.");
    return;
  }
  Serial.println(WiFi.channel());

  // Wait for pong response
  unsigned long startTime = millis();
  while (!gotReply && millis() - startTime < 10000) {
    delay(10);
  }

  if (!gotReply) {
    Serial.println("⚠️ Timeout: No pong received.");
  }

  esp_now_del_peer(toolMAC);  // 🧹 Clean up
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
  Serial.println("🔍 Tool requested: " + toolCallname);

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
  Serial.println(WiFi.macAddress());

  // Set channel and init ESP-NOW
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);

  // Web server routes
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