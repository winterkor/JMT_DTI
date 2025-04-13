#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEClient.h>

// ==== Wi-Fi credentials ====
const char* ssid = "Luck";
const char* password = "winterwifinotfreebutfree";

// ==== BLE UUIDs ====
#define SERVICE_UUID        "19b10010-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10011-e8f2-537e-4f6c-d104768a1214"

// ==== Globals ====
WebServer server(80);
BLEScan* bleScan;

// ==== Handle BLE communication ====
void sendBuzzToTool(String toolCallname) {
  Serial.println("🔍 Scanning for BLE device: " + toolCallname);

  BLEScanResults* results = bleScan->start(5); // 5 seconds

  for (int i = 0; i < results->getCount(); i++) {
    BLEAdvertisedDevice device = results->getDevice(i);
    String name = device.getName().c_str();

    Serial.println("🔎 Found: " + name);

    if (name == toolCallname) {
      Serial.println("✅ Match found! Connecting...");

      BLEClient* client = BLEDevice::createClient();
      if (client->connect(&device)) {
        BLERemoteService* service = client->getService(SERVICE_UUID);
        if (!service) {
          Serial.println("❌ Service not found!");
          client->disconnect();
          return;
        }

        BLERemoteCharacteristic* characteristic = service->getCharacteristic(CHARACTERISTIC_UUID);
        if (!characteristic || !characteristic->canWrite()) {
          Serial.println("❌ Characteristic error!");
          client->disconnect();
          return;
        }

        characteristic->writeValue("buzz");
        Serial.println("📤 'buzz' command sent.");
        client->disconnect();
        return;
      } else {
        Serial.println("❌ Failed to connect.");
        return;
      }
    }
  }

  Serial.println("❌ Target tool not found.");
  bleScan->clearResults();
}

// ==== Handle HTTP POST request ====
void handleFindTool() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Only POST allowed");
    return;
  }

  String body = server.arg("plain");
  Serial.println("📨 Received POST body: " + body);

  int start = body.indexOf(":\"") + 2;
  int end = body.indexOf("\"", start);
  String toolCallname = body.substring(start, end);

  Serial.println("🔧 Parsed tool callname: " + toolCallname);
  Serial.println("🛠️  Starting BLE process...");

  sendBuzzToTool(toolCallname);

  // 🔐 Add these for CORS
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST");

  server.send(200, "application/json", "{\"status\":\"buzz_sent\"}");
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);

  // Wi-Fi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("🔌 Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to Wi-Fi. IP: " + WiFi.localIP().toString());

  // BLE scan init
  BLEDevice::init("MiddlemanESP32");
  bleScan = BLEDevice::getScan();
  bleScan->setActiveScan(true);

  // Web server route
  server.on("/find_tool", handleFindTool);
  server.begin();
  Serial.println("🌐 HTTP server started.");
}

void loop() {
  server.handleClient();  // Handle HTTP requests
}