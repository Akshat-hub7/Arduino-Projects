#define BLYNK_TEMPLATE_ID "TMPL3Tznaap96"
#define BLYNK_TEMPLATE_NAME "Health Monitoring"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Ticker.h>

// -------- WiFi --------
char ssid = "YOUR_WIFI_SSID";
char pass = "YOUR_WIFI_PASSWORD";

// -------- Pins --------
#define BUZZER_PIN 12
#define DHTPIN 14
#define DHTTYPE DHT11
#define HEART_RATE_PIN A0  

// -------- OLED --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- Sensors --------
DHT dht(DHTPIN, DHTTYPE);
Adafruit_MPU6050 mpu;
Ticker timerInterrupt;

// -------- Pulse Variables --------
volatile int BPM;
volatile int Signal;
volatile int IBI = 600;
volatile boolean Pulse = false;
volatile boolean QS = false;
volatile int rate[10];
volatile unsigned long sampleCounter = 0;
volatile unsigned long lastBeatTime = 0;
volatile int P = 512;
volatile int T = 512;
volatile int thresh = 525;
volatile int amp = 100;
volatile boolean firstBeat = true;
volatile boolean secondBeat = false;

// -------- ISR --------
void ICACHE_RAM_ATTR onTimerISR() {
  Signal = analogRead(HEART_RATE_PIN);
  sampleCounter += 2;
  int N = sampleCounter - lastBeatTime;

  if (Signal < thresh && N > (IBI / 5) * 3) {
    if (Signal < T) T = Signal;
  }
  if (Signal > thresh && Signal > P) {
    P = Signal;
  }

  if (N > 250) {
    if ((Signal > thresh) && !Pulse && (N > (IBI / 5) * 3)) {
      Pulse = true;
      IBI = sampleCounter - lastBeatTime;
      lastBeatTime = sampleCounter;

      if (secondBeat) {
        secondBeat = false;
        for (int i = 0; i <= 9; i++) rate[i] = IBI;
      }
      if (firstBeat) {
        firstBeat = false;
        secondBeat = true;
        return;
      }

      word runningTotal = 0;
      for (int i = 0; i <= 8; i++) {
        rate[i] = rate[i + 1];
        runningTotal += rate[i];
      }
      rate[9] = IBI;
      runningTotal += rate[9];
      runningTotal /= 10;
      BPM = 60000 / runningTotal;
      QS = true;
    }
  }

  if (Signal < thresh && Pulse) {
    Pulse = false;
    amp = P - T;
    thresh = amp / 2 + T;
    P = thresh;
    T = thresh;
  }

  if (N > 2500) {
    thresh = 512; P = 512; T = 512;
    lastBeatTime = sampleCounter;
    firstBeat = true; secondBeat = false;
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  mpu.begin();
  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timerInterrupt.attach_ms(2, onTimerISR);

  Serial.println("System Ready");
}

// -------- LOOP --------
void loop() {
  Blynk.run();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelMag = sqrt(sq(a.acceleration.x) + sq(a.acceleration.y) + sq(a.acceleration.z));
  bool isFalling = (accelMag > 10.0);

  float tempC = dht.readTemperature();
  float tempF = tempC * 9.0 / 5.0 + 32.0;

  int displayBPM = BPM;
  if (displayBPM > 120) displayBPM = 0;

  // -------- SERIAL --------
  Serial.print("BPM: "); Serial.print(displayBPM);
  Serial.print(" | Temp: "); Serial.print(tempC);
  Serial.print("C / "); Serial.print(tempF);
  Serial.print("F | Accel: "); Serial.print(accelMag);
  if (isFalling) Serial.print(" | FALL DETECTED");
  Serial.println();

  // -------- OLED DESIGN --------
  display.clearDisplay();

  // Top Box (Temperature)
  display.drawRect(0, 0, 128, 16, WHITE);
  display.setCursor(5, 4);
  display.setTextSize(1);
  display.print("Temp: ");
  display.print(tempC, 1);
  display.print("C ");
  display.print(tempF, 0);
  display.print("F");

  // Middle Box (Heart Rate)
  display.drawRect(0, 18, 128, 30, WHITE);
  display.setCursor(30, 20);
  display.setTextSize(1);
  display.print("HEART RATE");

  display.setCursor(35, 30);
  display.setTextSize(2);
  display.print(displayBPM);
  display.print(" BPM");

  // Bottom Status Box
  display.drawRect(0, 50, 128, 14, WHITE);
  display.setCursor(5, 53);
  display.setTextSize(1);

  String fallStatus = "OK";

  if (isFalling) {
    display.print("FALL DETECTED!");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    fallStatus = "FALL DETECTED";
  } else {
    display.print("Status: Normal");
  }

  display.display();

  // -------- BLYNK --------
  Blynk.virtualWrite(V0, displayBPM);
  Blynk.virtualWrite(V1, tempC);
  Blynk.virtualWrite(V2, tempF);
  Blynk.virtualWrite(V3, fallStatus);

  delay(500);
}