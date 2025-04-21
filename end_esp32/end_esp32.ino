#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "esp_sleep.h"

// BLE UUIDs
#define SERVICE_UUID        "19b10010-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10011-e8f2-537e-4f6c-d104768a1214"

// Hardware pins
const int LED_PIN = 0;
const int BUZZER_PIN = 1;

bool buzzNow = false, replyalr = false;

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
  digitalWrite(LED_PIN, LOW);

  // BLE init
  BLEDevice::init("Tool_05_digitalmultimeter");  // This must match the web callname
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
  pAdvertising->setScanResponse(false);
  pAdvertising->start();
  Serial.println("📡 BLE Advertising started");
}

void loop() {
  if (buzzNow && !replyalr) {
    // 💡 Feedback: Buzzer + LED
    replyalr = true;
    digitalWrite(LED_PIN, HIGH);
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH); delay(500);
      digitalWrite(BUZZER_PIN, LOW);  delay(500);
    }
    digitalWrite(LED_PIN, LOW);
    buzzNow = false;

    Serial.println("😴 Going to sleep for 10 seconds...");
    delay(200); // Let serial finish
    BLEDevice::deinit(); // Clean BLE stack before sleep
    esp_sleep_enable_timer_wakeup(10 * 1000000);  // 10 seconds
    esp_deep_sleep_start();
  }
}