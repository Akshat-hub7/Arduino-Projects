#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- CONFIG ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
const int relayPin = 8;
const int buzzerPin = 6;
const int turbidityPin = A0;
const int tdsPin = A1;
const int irSensorPin = 7;

const int trigPinTank = 5;
const int echoPinTank = 4;

// Tank calibration
const int tankEmptyDist = 30;
const int tankFullDist = 5;

// Variables
int waterLevelPct = 0;

// Manual control
bool manualMode = false;
bool relayState = false;

// ---------- ULTRASONIC ----------
int readTankDistance() {
  long total = 0;
  int count = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPinTank, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPinTank, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinTank, LOW);

    long duration = pulseIn(echoPinTank, HIGH, 30000);

    if (duration > 0) {
      int d = duration * 0.034 / 2;
      if (d > 2 && d < 400) {
        total += d;
        count++;
      }
    }
    delay(10);
  }

  if (count == 0) return -1;
  return total / count;
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(irSensorPin, INPUT);
  pinMode(trigPinTank, OUTPUT);
  pinMode(echoPinTank, INPUT);

  digitalWrite(relayPin, LOW);
  digitalWrite(buzzerPin, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM STARTED");
  delay(1500);

  randomSeed(analogRead(0)); // seed randomness
}

// ---------- LOOP ----------
void loop() {

  // ===== RECEIVE FROM ESP =====
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "RELAY_ON") {
      manualMode = true;
      relayState = true;
    }
    else if (cmd == "RELAY_OFF") {
      manualMode = true;
      relayState = false;
    }
    else if (cmd == "RELAY_AUTO") {
      manualMode = false;
    }
  }

  // ===== RANDOM pH VALUE =====
  float phValue = random(65, 76) / 10.0;  // 6.5 to 7.5

  // ===== Sensors =====
  int turbRaw = analogRead(turbidityPin);
  int tdsRaw = analogRead(tdsPin);

  float voltage = tdsRaw * 5.0 / 1024.0;
  float tdsPpm = (133.42 * pow(voltage, 3)
                 - 255.86 * pow(voltage, 2)
                 + 857.39 * voltage) * 0.5;

  bool glassDetected = (digitalRead(irSensorPin) == LOW);

  int dist = readTankDistance();
  if (dist != -1) {
    int rawPct = map(dist, tankEmptyDist, tankFullDist, 0, 100);
    waterLevelPct = constrain(rawPct, 0, 100);
  }

  // ===== CONDITIONS =====
  bool tankOK = waterLevelPct > 5;
  bool phOK = (phValue >= 6.5 && phValue <= 7.5);
  bool tdsOK = (tdsPpm < 500);

  String status;

  if (manualMode) {
    digitalWrite(relayPin, relayState ? HIGH : LOW);
    status = relayState ? "MANUAL ON" : "MANUAL OFF";
  } else {
    if (glassDetected && tankOK && phOK && tdsOK) {
      digitalWrite(relayPin, HIGH);
      digitalWrite(buzzerPin, LOW);
      status = "RUN";
    } else {
      digitalWrite(relayPin, LOW);
      digitalWrite(buzzerPin, HIGH);

      if (!glassDetected) status = "NO GLASS";
      else status = "LOW WATER";
    }
  }

  // ===== LCD DISPLAY =====
  static int screen = 0;
  static unsigned long lastSwitch = 0;

  if (millis() - lastSwitch > 2000) {
    screen++;
    if (screen > 2) screen = 0;
    lastSwitch = millis();
  }

  lcd.clear();

  if (screen == 0) {
    lcd.setCursor(0, 0);
    lcd.print("pH:");
    lcd.print(phValue, 1);

    lcd.setCursor(0, 1);
    lcd.print("TDS:");
    lcd.print((int)tdsPpm);
  }

  else if (screen == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Turb:");
    lcd.print(turbRaw);

    lcd.setCursor(0, 1);
    lcd.print("TDS:");
    lcd.print((int)tdsPpm);
  }

  else {
    lcd.setCursor(0, 0);
    lcd.print("Water:");
    lcd.print(waterLevelPct);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("TDS:");
    lcd.print((int)tdsPpm);
  }

  // ===== SERIAL TO ESP =====
  Serial.print(phValue); Serial.print(",");
  Serial.print(tdsPpm); Serial.print(",");
  Serial.print(turbRaw); Serial.print(",");
  Serial.print(waterLevelPct); Serial.print(",");
  Serial.print(glassDetected); Serial.print(",");
  Serial.println(status);

  delay(500);
}