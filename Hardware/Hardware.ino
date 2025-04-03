#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp_sleep.h"
#include <WiFi.h>

// UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "68342c53-ac8f-48af-bb4d-7a55375c98a5"

const int LED_PIN = 2;       // Built-in LED (change if using external)
const int BUZZER_PIN = 4;    // Connect buzzer to GPIO 4

unsigned long startTime;     
bool buzzNow = false;        // For signal sending
BLECharacteristic *commandCharacteristic;


// Listen for Command
class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) {
    String value = characteristic->getValue();
    if (value == "buzz") {
      buzzNow = true;
    }
  }
};

void setup() {
  Serial.begin(115200); //Can remove to save energy
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  // WiFi.mode(WIFI_OFF); // Turn off Wi-Fi to save battery
  // btStop(); // disables Bluetooth classic (BLE still works)

  BLEDevice::init("Tool_01"); //Set this ESP32 as Tool_01
  BLEServer *pServer = BLEDevice::createServer(); //Creates a BLE server
  BLEService *pService = pServer->createService(SERVICE_UUID); //Creates a BLE service

  commandCharacteristic = pService->createCharacteristic(
                           CHARACTERISTIC_UUID,
                           BLECharacteristic::PROPERTY_WRITE
                         );

  commandCharacteristic->setCallbacks(new CommandCallback());
  commandCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("🔋 Advertising for 5 seconds...");
  startTime = millis();
}

void loop() {
  if (buzzNow) {
    Serial.println("🔔 Buzzing!");
    digitalWrite(LED_PIN, HIGH); // Flash out LED
    for(int i=0;i<3;i++){ // Beep 3 times
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
      delay(1000);
    }
    digitalWrite(LED_PIN, LOW); // Stop LED
    buzzNow = false;
  }

  // After 5 seconds, go to deep sleep
  if (millis() - startTime > 5000) {
    Serial.println("💤 Going to deep sleep for 10 seconds...");
    esp_sleep_enable_timer_wakeup(1 * 1000000); // Set up the time after it go to sleep, wake it up after 10 seconds
    esp_deep_sleep_start(); // Deep sleep mode stop loop
  }
  //After that, it automatically restarts the program completely
}