#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "esp_sleep.h"

// BLE UUIDs – must match middleman
#define SERVICE_UUID        "19b10010-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10011-e8f2-537e-4f6c-d104768a1214"

// Hardware pins
const int LED_PIN = 8;
const int BUZZER_PIN = 4;

bool buzzNow = false;
unsigned long startTime;

// BLE characteristic pointer
BLECharacteristic* commandCharacteristic;

// BLE callback class
class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    String value = characteristic->getValue();
    if (value == "buzz") {
      Serial.println("✅ Received 'buzz' command!");
      buzzNow = true;
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // BLE init
  BLEDevice::init("Tool_01_clawhammer");  // This must match the web callname
  BLEServer* pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService(SERVICE_UUID);

  commandCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  commandCharacteristic->setCallbacks(new CommandCallback());
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("📡 BLE Advertising: Tool_01_clawhammer");
  startTime = millis();
}

void loop() {
  if (buzzNow) {
    digitalWrite(LED_PIN, LOW);
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
      delay(1000);
    }
    digitalWrite(LED_PIN, HIGH);
    buzzNow = false;
  }

  // Deep sleep after 5 seconds idle
  if (millis() - startTime > 5000) {
    Serial.println("😴 Going to deep sleep...");
    esp_sleep_enable_timer_wakeup(1000000); // 1 second wake-up
    esp_deep_sleep_start(); // Will reset and restart setup
  }
}