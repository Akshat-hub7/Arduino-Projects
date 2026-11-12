#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Servo.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic parking slots
#define TRIG1 2
#define ECHO1 3

#define TRIG2 4
#define ECHO2 5

#define TRIG3 6
#define ECHO3 7

// 🔥 NEW: Gate Ultrasonic
#define TRIG4 8
#define ECHO4 10

// Servo
#define SERVO_PIN 9
Servo gateServo;

// Thresholds
#define THRESHOLD 10        // parking slots
#define GATE_THRESHOLD 15   // gate detection distance (cm)

// Slot states
bool slot1 = true;
bool slot2 = true;
bool slot3 = true;

// Animation
int carX = -20;
bool blink = false;

// Blynk override
bool remoteOpen = false;

// Distance function
long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(5);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);

  if (duration == 0) return 999;

  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(9600);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  // 🔥 Gate ultrasonic
  pinMode(TRIG4, OUTPUT);
  pinMode(ECHO4, INPUT);

  gateServo.attach(SERVO_PIN);
  gateServo.write(0);

  if (!display.begin(0x3C, true)) {
    Serial.println("OLED failed");
    while (1);
  }

  display.clearDisplay();
}

void loop() {

  // 🔥 Blynk command
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'O') remoteOpen = true;
    if (cmd == 'C') remoteOpen = false;
  }

  // 🔥 Gate detection using ultrasonic
  long gateDistance = getDistance(TRIG4, ECHO4);
  bool carAtGate = (gateDistance < GATE_THRESHOLD);

  // 🔥 Slot sensors
  long d1 = getDistance(TRIG1, ECHO1);
  delay(60);

  long d2 = getDistance(TRIG2, ECHO2);
  delay(60);

  long d3 = getDistance(TRIG3, ECHO3);
  delay(60);

  // Slot logic
  slot1 = (d1 > THRESHOLD);
  slot2 = (d2 > THRESHOLD);
  slot3 = (d3 > THRESHOLD);

  int available = slot1 + slot2 + slot3;

  // 🔥 Servo logic
  if ((carAtGate && available > 0) || remoteOpen) {
    gateServo.write(90);
  } else {
    gateServo.write(0);
  }

  delay(200);

  // ===== SEND DATA TO ESP =====
  Serial.print(slot1); Serial.print(",");
  Serial.print(slot2); Serial.print(",");
  Serial.print(slot3); Serial.print(",");
  Serial.println(available);

  // ===== OLED UI =====
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  blink = !blink;

  // 🚫 FULL
  if (available == 0) {

    display.setTextSize(2);
    display.setCursor(10, 5);
    display.println("PARKING");

    if (blink) {
      display.setCursor(20, 35);
      display.println("FULL!");
    }

    display.fillRect(0, 60, carX + 30, 4, SH110X_WHITE);

    carX += 5;
    if (carX > 128) carX = 0;
  }

  // 🟢 NORMAL
  else {

    display.setTextSize(1);
    display.setCursor(15, 0);
    display.println("SMART PARKING");

    // Car animation
    if (carAtGate) {
      display.fillRect(carX, 10, 10, 5, SH110X_WHITE);
      carX += 5;
      if (carX > 120) carX = -20;
    } else {
      carX = -20;
    }

    // Slot 1
    display.drawRect(0, 20, 40, 20, SH110X_WHITE);
    if (!slot1) {
      display.fillRect(0, 20, 40, 20, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else display.setTextColor(SH110X_WHITE);
    display.setCursor(5, 25);
    display.print(slot1 ? "E" : "F");

    // Slot 2
    display.drawRect(44, 20, 40, 20, SH110X_WHITE);
    if (!slot2) {
      display.fillRect(44, 20, 40, 20, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else display.setTextColor(SH110X_WHITE);
    display.setCursor(49, 25);
    display.print(slot2 ? "E" : "F");

    // Slot 3
    display.drawRect(88, 20, 40, 20, SH110X_WHITE);
    if (!slot3) {
      display.fillRect(88, 20, 40, 20, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else display.setTextColor(SH110X_WHITE);
    display.setCursor(93, 25);
    display.print(slot3 ? "E" : "F");

    display.setTextColor(SH110X_WHITE);

    display.setCursor(5, 42); display.print("S1");
    display.setCursor(49, 42); display.print("S2");
    display.setCursor(93, 42); display.print("S3");

    display.setCursor(0, 55);
    display.print("Free:");
    display.print(available);

    display.setCursor(70, 55);

    if ((carAtGate && available > 0) || remoteOpen) {
      if (blink)
        display.print("OPEN");
      else
        display.print("    ");
    } else {
      display.print("CLOSED");
    }
  }

  display.display();

  delay(150);
}