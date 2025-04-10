#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "esp_sleep.h"

// BLE UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "6acbd969-2f0d-4e1d-92c2-c99c698aed83"

// Hardware pins
const int LED_PIN = 8;
const int BUZZER_PIN = 4;

bool buzzNow = false;
unsigned long startTime;

// BLE characteristic pointer
BLECharacteristic* commandCharacteristic;

// BLE callback
class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    String value = characteristic->getValue();
    if (!value.isEmpty() && value[0] == 'j') { // 'j' for "jiggle"
      buzzNow = true;
    }
  }
};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // BLE setup
  BLEDevice::init("Tool_01");
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

  // Go to deep sleep after 5 seconds
  if (millis() - startTime > 5000) {
    esp_sleep_enable_timer_wakeup(1000000); // 1 second
    esp_deep_sleep_start(); // Will reset and run setup again
  }
}