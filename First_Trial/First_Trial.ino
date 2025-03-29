#define BUTTON_PIN 26     // GPIO0 for button (has pull-up)
#define LED_PIN 2        // GPIO2 is built-in LED on many ESP32 boards
#define BUZZER_PIN 12
int buttonState = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Internal pull-up resistor
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);        // Start with LED off
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  Serial.println(buttonState);
  if (digitalRead(BUTTON_PIN) == LOW) { // Button is pressed
    digitalWrite(LED_PIN, HIGH);        // Turn on LED
    tone(BUZZER_PIN, TONE_FREQ);     // Turn on buzzer
  } else {
    digitalWrite(LED_PIN, LOW);         // Turn off LED
    noTone(BUZZER_PIN);       // Turn off buzzer
  }
  delay(100);
}