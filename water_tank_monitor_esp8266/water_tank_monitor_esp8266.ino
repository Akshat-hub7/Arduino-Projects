#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// Pins
#define TRIG 14   // D5
#define ECHO 12   // D6

#define RAIN1 5   // D1
#define RAIN2 4   // D2
#define RAIN3 0   // D3

#define RELAY 13  // D7
#define BUZZER 15 // D8

long duration;
float distance;
float tankHeight = 15.0;

// Modes
bool autoMode = true;
bool manualRelayState = false;

BlynkTimer timer;

// -------- WATER LEVEL --------
float getWaterLevelPercent() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH);
  distance = duration * 0.034 / 2;

  float level = ((tankHeight - distance) / tankHeight) * 100;

  if (level > 100) level = 100;
  if (level < 0) level = 0;

  return level;
}

// -------- TANK FULL --------
bool isTankFull() {
  if (digitalRead(RAIN1) == LOW ||
      digitalRead(RAIN2) == LOW ||
      digitalRead(RAIN3) == LOW) {
    return true;
  }
  return false;
}

// -------- BLYNK RELAY CONTROL --------
BLYNK_WRITE(V2) {  // Manual Relay Button
  manualRelayState = param.asInt();
}

BLYNK_WRITE(V3) {  // Auto / Manual Switch
  autoMode = param.asInt();
}

// -------- SEND DATA TO BLYNK --------
void sendData() {
  float waterLevel = getWaterLevelPercent();
  bool full = isTankFull();

  Blynk.virtualWrite(V0, waterLevel);
  Blynk.virtualWrite(V1, full);

  Serial.print("Water Level: ");
  Serial.print(waterLevel);
  Serial.print("% | Full: ");
  Serial.println(full);

  // -------- CONTROL LOGIC --------
  if (autoMode) {
    if (waterLevel < 30 && !full) {
      digitalWrite(RELAY, LOW);   // ON
      digitalWrite(BUZZER, LOW);
      Serial.println("AUTO: Motor ON");
    } 
    else if (full) {
      digitalWrite(RELAY, HIGH);  // OFF
      Serial.println("AUTO: Overflow");

      digitalWrite(BUZZER, HIGH);
      delay(300);
      digitalWrite(BUZZER, LOW);
    } 
    else {
      digitalWrite(RELAY, HIGH); // OFF
    }
  } 
  else {
    // Manual Mode
    digitalWrite(RELAY, manualRelayState ? LOW : HIGH);
    Serial.println("MANUAL MODE");
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(RAIN1, INPUT);
  pinMode(RAIN2, INPUT);
  pinMode(RAIN3, INPUT);

  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RELAY, HIGH);
  digitalWrite(BUZZER, LOW);

  Blynk.begin("", ssid, pass);

  timer.setInterval(1000L, sendData);
}

void loop() {
  Blynk.run();
  timer.run();
}