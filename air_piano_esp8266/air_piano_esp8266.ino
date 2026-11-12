#define BLYNK_TEMPLATE_ID "TMPL3qIWBsD4P"
#define BLYNK_TEMPLATE_NAME "Air Piano"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid = "YOUR_WIFI_SSID";
char pass = "YOUR_WIFI_PASSWORD";

// Drum button pin
#define DRUM_BUTTON D3

bool lastButtonState = HIGH;

void setup() {

  // Must match ATmega328P baud rate
  Serial.begin(115200);

  // Button setup
  pinMode(DRUM_BUTTON, INPUT_PULLUP);

  Blynk.begin(auth, ssid, pass);
}

// =========================
// BLYNK BUTTONS
// =========================
BLYNK_WRITE(V1) {
  if (param.asInt()) Serial.println("PLAY1");
}

BLYNK_WRITE(V2) {
  if (param.asInt()) Serial.println("PLAY2");
}

BLYNK_WRITE(V3) {
  if (param.asInt()) Serial.println("PLAY3");
}

BLYNK_WRITE(V4) {
  if (param.asInt()) Serial.println("PLAY4");
}

BLYNK_WRITE(V5) {
  if (param.asInt()) Serial.println("PLAY5");
}

// Optional Blynk Drum Button (V7)
BLYNK_WRITE(V7) {
  if (param.asInt()) Serial.println("PLAY6");
}

void loop() {

  Blynk.run();

  // =========================
  // PHYSICAL DRUM BUTTON
  // =========================
  bool buttonState = digitalRead(DRUM_BUTTON);

  // Detect button press
  if (lastButtonState == HIGH && buttonState == LOW) {

    Serial.println("PLAY6");

    delay(200);
  }

  lastButtonState = buttonState;

  // =========================
  // RECEIVE NOTE STATUS
  // =========================
  if (Serial.available() > 0) {

    String msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg.startsWith("NOTE:")) {

      Blynk.virtualWrite(V6, "Playing: Note " + msg.substring(5));
    }
  }
}