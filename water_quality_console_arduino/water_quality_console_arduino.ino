#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- pH SENSOR ----------
SoftwareSerial sensorSerial(10, 11);
String rawPH = "0.00";
float pHValue = 7.0;

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- PINS ----------
const int relayPin = 8;
const int trigPin = 9;
const int echoPin = 7;
const int turbidityPin = A0;
const int tdsPin = A1;

// ---------- VARIABLES ----------
bool blynkRelayCmd = false;

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);        // ESP communication
  sensorSerial.begin(9600);

  pinMode(relayPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(relayPin, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  randomSeed(analogRead(A3));
}

// ---------- ULTRASONIC ----------
int getDistance() {
  long duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

// ---------- LOOP ----------
void loop() {

  // ===== RECEIVE RELAY COMMAND FROM ESP =====
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "1") blynkRelayCmd = true;
    if (cmd == "0") blynkRelayCmd = false;
  }

  // ===== RANDOM pH VALUE =====
  pHValue = random(65, 76) / 10.0;
  rawPH = String(pHValue, 1);

  // ===== OTHER SENSORS =====
  int turbRaw = analogRead(turbidityPin);
  int tdsRaw = analogRead(tdsPin);

  float voltage = tdsRaw * 5.0 / 1024.0;
  float tdsPpm = (133.42 * pow(voltage, 3)
                 - 255.86 * pow(voltage, 2)
                 + 857.39 * voltage) * 0.5;

  int distance = getDistance();
  bool glassDetected = distance < 10;

  bool safe = (turbRaw < 400 && tdsPpm < 500 &&
               pHValue >= 6.5 && pHValue <= 8.5 && glassDetected);

  // ===== RELAY CONTROL =====
  if (safe) {
    digitalWrite(relayPin, HIGH);
    delay(3000);
    digitalWrite(relayPin, LOW);
  }
  else if (blynkRelayCmd) {
    digitalWrite(relayPin, HIGH);
  } else {
    digitalWrite(relayPin, LOW);
  }

  String status = safe ? "SAFE" : "UNSAFE";

  // ===== SEND DATA TO ESP =====
  Serial.print(rawPH);
  Serial.print(",");
  Serial.print(tdsPpm);
  Serial.print(",");
  Serial.print(turbRaw);
  Serial.print(",");
  Serial.print(glassDetected);
  Serial.print(",");
  Serial.println(status);

  updateDisplay(turbRaw, tdsPpm, rawPH, distance, status);

  delay(500);
}

// ---------- OLED ----------
void updateDisplay(int turb, float tds, String rawPH, int dist, String status) {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("WATER QUALITY");
  display.print("pH: "); display.println(rawPH);
  display.print("TDS: "); display.println(tds, 0);
  display.print("Turb: "); display.println(turb);
  display.print("Dist: "); display.println(dist);

  display.println("----------------");

  display.setTextSize(2);
  display.println(status);
  display.setTextSize(1);

  display.display();
}