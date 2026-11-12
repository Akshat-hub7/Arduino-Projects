#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

// WiFi
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// GPIO pins
#define SERVO_PIN 5        // D1
#define ENTRY_SWITCH 14    // D5
#define EXIT_SWITCH 12     // D6

Servo gateServo;

int slotCount = 6;
int cars = 0;

bool entryDetected = false;
bool exitDetected = false;

BlynkTimer timer;

// -------- MANUAL BUTTON --------
BLYNK_WRITE(V6) {
  if (param.asInt() == 1) {
    openGate();
    Blynk.virtualWrite(V5, "Gate Opened Manually");
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  // Push buttons with internal pullup
  pinMode(ENTRY_SWITCH, INPUT_PULLUP);
  pinMode(EXIT_SWITCH, INPUT_PULLUP);

  gateServo.attach(SERVO_PIN);
  gateServo.write(0);

  Blynk.begin("", ssid, pass);

  timer.setInterval(1000L, sendToBlynk);

  Serial.println("Smart Parking Started");
}

// -------- LOOP --------
void loop() {
  Blynk.run();
  timer.run();

  int entryState = digitalRead(ENTRY_SWITCH);
  int exitState = digitalRead(EXIT_SWITCH);

  // -------- ENTRY --------
  // Button pressed = LOW
  if (entryState == LOW && !entryDetected) {
    entryDetected = true;

    if (cars < slotCount) {
      openGate();
      cars++;

      Serial.print("Car Entered | Cars: ");
      Serial.println(cars);

      Blynk.virtualWrite(V5, "Car Entered");
    } else {
      Serial.println("Parking FULL");
      Blynk.virtualWrite(V5, "Parking Full");
    }
  }

  // Button released
  if (entryState == HIGH) {
    entryDetected = false;
  }

  // -------- EXIT --------
  if (exitState == LOW && !exitDetected) {
    exitDetected = true;

    if (cars > 0) {
      cars--;

      Serial.print("Car Exited | Cars: ");
      Serial.println(cars);

      Blynk.virtualWrite(V5, "Car Exited");
    }
  }

  // Button released
  if (exitState == HIGH) {
    exitDetected = false;
  }
}

// -------- SEND TO BLYNK --------
void sendToBlynk() {
  int available = slotCount - cars;
  Blynk.virtualWrite(V4, available);
}

// -------- SERVO FUNCTION --------
void openGate() {
  Serial.println("Opening Gate...");

  gateServo.write(90);
  delay(3000);
  gateServo.write(0);

  Serial.println("Gate Closed");
}