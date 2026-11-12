#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- pH SENSOR ----------
SoftwareSerial sensorSerial(10, 11);
String sensorData = "";
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
const int buzzerPin = 6;

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  sensorSerial.begin(9600);

  pinMode(relayPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(relayPin, LOW);
  digitalWrite(buzzerPin, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  Serial.println("System Initialized...");
}

// ---------- ULTRASONIC ----------
int getDistance() {
  long total = 0;
  int count = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);

    if (duration > 0) {
      int d = duration * 0.034 / 2;
      if (d > 2 && d < 400) {
        total += d;
        count++;
      }
    }
    delay(10);
  }

  if (count == 0) return 999;
  return total / count;
}

// ---------- LOOP ----------
void loop() {

  // ===== YOUR EXACT LOGIC (READ FULL DATA) =====
  if (sensorSerial.available()) {
    sensorData = sensorSerial.readStringUntil('\n');
    sensorData.trim();

    Serial.print("Sensor Output: ");
    Serial.println(sensorData);

    // ===== EXTRACT FIRST VALUE (pH) =====
    int commaIndex = sensorData.indexOf(',');

    if (commaIndex > 0) {
      String phString = sensorData.substring(0, commaIndex);
      float tempPH = phString.toFloat();

      if (tempPH > 0 && tempPH < 14) {
        pHValue = tempPH;
      }
    }
  }

  // -------- OTHER SENSORS --------
  int turbRaw = analogRead(turbidityPin);
  int tdsRaw = analogRead(tdsPin);

  float voltage = tdsRaw * 5.0 / 1024.0;
  float tdsPpm = (133.42 * pow(voltage, 3)
                 - 255.86 * pow(voltage, 2)
                 + 857.39 * voltage) * 0.5;

  int distance = getDistance();

  // -------- CONDITIONS --------
  bool goodTurbidity = turbRaw < 400;
  bool goodTDS = tdsPpm > 50 && tdsPpm < 300;
  bool goodPH = pHValue >= 6.5 && pHValue <= 8.5;
  bool glassDetected = distance < 20;

  // -------- LOGIC --------
  if (goodTurbidity && goodTDS && goodPH && glassDetected) {
    digitalWrite(relayPin, HIGH);
    digitalWrite(buzzerPin, LOW);
    updateDisplay(turbRaw, tdsPpm, pHValue, distance, "SAFE");
  } else {
    digitalWrite(relayPin, LOW);
    digitalWrite(buzzerPin, HIGH);
    updateDisplay(turbRaw, tdsPpm, pHValue, distance, "UNSAFE");
  }

  delay(500);
}

// ---------- OLED ----------
void updateDisplay(int turb, float tds, float ph, int dist, String status) {

  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("WATER MONITOR");
  display.print("Turb: "); display.println(turb);
  display.print("TDS : "); display.print(tds, 0); display.println("ppm");
  display.print("pH  : "); display.println(ph, 2);
  display.print("Dst : "); display.print(dist); display.println("cm");

  display.println("----------------");

  display.setTextSize(2);
  display.println(status);
  display.setTextSize(1);

  display.display();
}