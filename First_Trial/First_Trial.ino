#define BUTTON_PIN 26     // GPIO0 for button (has pull-up)
#define LED_PIN 2        // GPIO2 is built-in LED on many ESP32 boards
int buttonState = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Internal pull-up resistor
  pinMode(LED_PIN, OUTPUT);          // Built-in LED
  digitalWrite(LED_PIN, HIGH);        // Start with LED off
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  Serial.println(buttonState);
  if (buttonState == LOW) {
    digitalWrite(LED_PIN, HIGH);     // Turn on LED when button is pressed
  } else {
    digitalWrite(LED_PIN, LOW);      // Turn off LED when button is released
  }
  delay(100);
}