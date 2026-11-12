// ==========================================
// BLYNK CONFIGURATION
// ==========================================
#define BLYNK_PRINT Serial

// ==========================================
// LIBRARIES
// ==========================================
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================================
// WIFI
// ==========================================
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// ==========================================
// OLED CONFIG
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==========================================
// PINS
// ==========================================
const int fsrPins[] = {32, 33, 27, 35};

const int vibPin = 34;

const int ledPin = 2;
const int buzzerPin = 26;

// ==========================================
// THRESHOLDS
// ==========================================
const int pressureThreshold = 300;
const int vibrationThreshold = 1800;

// ==========================================
// FLAGS
// ==========================================
bool alarmActive = false;
bool eventSent = false;

// ==========================================
// TIMER
// ==========================================
BlynkTimer timer;

// ==========================================
// BLYNK BUTTON
// V3 = Buzzer Button
// ==========================================
BLYNK_WRITE(V3) {

  int buttonState = param.asInt();

  if (buttonState == 1) {

    digitalWrite(buzzerPin, HIGH);

  } else {

    digitalWrite(buzzerPin, LOW);
  }
}

// ==========================================
// SETUP
// ==========================================
void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  analogReadResolution(12);

  // ==========================================
  // OLED INIT
  // ==========================================
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED FAILED");

    while (1);
  }

  // ==========================================
  // BOOT ANIMATION
  // ==========================================
  display.clearDisplay();

  for (int i = 0; i <= 100; i += 4) {

    display.clearDisplay();

    display.drawRoundRect(0, 0, 128, 64, 6, WHITE);

    display.setTextSize(2);
    display.setTextColor(WHITE);

    display.setCursor(18, 8);
    display.print("ANTI");

    display.setCursor(18, 46);
    display.print("THEFT");

    // Loading Bar
    display.drawRect(14, 30, 100, 8, WHITE);

    display.fillRect(15, 31, i, 6, WHITE);

    display.display();

    delay(40);
  }

  // ==========================================
  // CONNECT TO BLYNK
  // ==========================================
  Blynk.begin("", ssid, pass);

  // ==========================================
  // READY SCREEN
  // ==========================================
  display.clearDisplay();

  display.drawRoundRect(0, 0, 128, 64, 6, WHITE);

  display.setTextSize(2);

  display.setCursor(18, 18);
  display.print("ONLINE");

  display.setTextSize(1);

  display.setCursor(28, 46);
  display.print("System Ready");

  display.display();

  delay(1500);

  // ==========================================
  // TIMER
  // ==========================================
  timer.setInterval(150L, processSensors);
}

// ==========================================
// LOOP
// ==========================================
void loop() {

  Blynk.run();
  timer.run();
}

// ==========================================
// SENSOR PROCESSING
// ==========================================
void processSensors() {

  bool weightDetected = false;

  int maxPressure = 0;

  // ==========================================
  // FSR READINGS
  // ==========================================
  for (int i = 0; i < 4; i++) {

    int val = analogRead(fsrPins[i]);

    if (val > pressureThreshold) {

      weightDetected = true;
    }

    if (val > maxPressure) {

      maxPressure = val;
    }
  }

  // ==========================================
  // VIBRATION SENSOR
  // ==========================================
  int vibrationValue = analogRead(vibPin);

  bool vibDetected = false;

  if (vibrationValue > vibrationThreshold) {

    vibDetected = true;
  }

  // ==========================================
  // SERIAL MONITOR
  // ==========================================
  Serial.print("Pressure: ");
  Serial.print(maxPressure);

  Serial.print(" | Vibration: ");
  Serial.println(vibrationValue);

  // ==========================================
  // ALARM CONDITION
  // ==========================================
  if (weightDetected || vibDetected) {

    alarmActive = true;

    // Send Notification Once
    if (!eventSent) {

      Blynk.logEvent(
        "intrusion_alert",
        "🚨 Theft detected!"
      );

      eventSent = true;
    }

    // LED ON
    digitalWrite(ledPin, HIGH);

    // Buzzer Pulse
    digitalWrite(buzzerPin, HIGH);

    delay(70);

    digitalWrite(buzzerPin, LOW);

  } else {

    alarmActive = false;

    digitalWrite(ledPin, LOW);

    eventSent = false;
  }

  // ==========================================
  // OLED UPDATE
  // ==========================================
  updateDisplay(maxPressure, vibrationValue);

  // ==========================================
  // BLYNK STATUS LABEL
  // V0 = LABEL
  // ==========================================
  if (alarmActive) {

    Blynk.virtualWrite(V0, "🚨 THEFT DETECTED");

  } else {

    Blynk.virtualWrite(V0, "✅ ALL SAFE");
  }

  // ==========================================
  // PRESSURE GAUGE
  // V1 = GAUGE
  // ==========================================
  Blynk.virtualWrite(V1, maxPressure);

  // ==========================================
  // VIBRATION GAUGE
  // V2 = GAUGE
  // ==========================================
  Blynk.virtualWrite(V2, vibrationValue);
}

// ==========================================
// MODERN OLED UI
// ==========================================
void updateDisplay(int pressure, int vibration) {

  static int anim = 0;

  static bool pulse = false;

  anim++;

  if (anim > 120) {

    anim = 0;
  }

  pulse = !pulse;

  display.clearDisplay();

  // ==========================================
  // OUTER FRAME
  // ==========================================
  display.drawRoundRect(0, 0, 128, 64, 6, WHITE);

  // ==========================================
  // HEADER
  // ==========================================
  display.fillRoundRect(0, 0, 128, 12, 4, WHITE);

  display.setTextColor(BLACK);

  display.setTextSize(1);

  display.setCursor(18, 2);

  display.print("ANTI THEFT SYSTEM");

  display.setTextColor(WHITE);

  // ==========================================
  // SCAN LINE
  // ==========================================
  int scanX = anim % 128;

  display.drawLine(scanX, 13, scanX + 10, 13, WHITE);

  // ==========================================
  // STATUS DISPLAY
  // ==========================================
  if (alarmActive) {

    // Animated Alert Icon
    if (pulse) {

      display.fillCircle(18, 30, 12, WHITE);

      display.setTextColor(BLACK);

      display.setTextSize(2);

      display.setCursor(13, 20);

      display.print("!");

      display.setTextColor(WHITE);

    } else {

      display.drawCircle(18, 30, 12, WHITE);

      display.setTextSize(2);

      display.setCursor(13, 20);

      display.print("!");
    }

    // Alert Text
    display.setTextSize(2);

    display.setCursor(38, 18);

    display.print("ALERT");

    display.setTextSize(1);

    display.setCursor(42, 42);

    display.print("INTRUSION!");

    // Flash Border
    if (pulse) {

      display.drawRoundRect(2, 2, 124, 60, 6, WHITE);
    }

  } else {

    // Shield Icon
    display.drawRoundRect(8, 18, 20, 24, 4, WHITE);

    display.drawLine(12, 30, 16, 34, WHITE);

    display.drawLine(16, 34, 24, 24, WHITE);

    // Safe Text
    display.setTextSize(2);

    display.setCursor(40, 20);

    display.print("SAFE");

    display.setTextSize(1);

    display.setCursor(42, 42);

    display.print("SYSTEM NORMAL");
  }

  // ==========================================
  // PRESSURE BAR
  // ==========================================
  int pBar = map(pressure, 0, 4095, 0, 50);

  display.drawRect(6, 54, 52, 6, WHITE);

  display.fillRect(7, 55, pBar, 4, WHITE);

  // ==========================================
  // VIBRATION BAR
  // ==========================================
  int vBar = map(vibration, 0, 4095, 0, 50);

  display.drawRect(70, 54, 52, 6, WHITE);

  display.fillRect(71, 55, vBar, 4, WHITE);

  // ==========================================
  // VALUES
  // ==========================================
  display.setTextSize(1);

  display.setCursor(6, 46);

  display.print("P:");
  display.print(pressure);

  display.setCursor(70, 46);

  display.print("V:");
  display.print(vibration);

  // ==========================================
  // SIGNAL ICON
  // ==========================================
  display.drawLine(118, 20, 118, 24, WHITE);

  display.drawLine(121, 18, 121, 24, WHITE);

  display.drawLine(124, 15, 124, 24, WHITE);

  display.display();
}