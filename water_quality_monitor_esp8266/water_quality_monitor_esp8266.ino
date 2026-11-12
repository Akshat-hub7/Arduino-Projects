#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

String dataString = "";

// ⏱ TIMER VARIABLES
bool motorRunning = false;
unsigned long motorStartTime = 0;

// 🔘 BUTTON CONTROL (V6)
BLYNK_WRITE(V6) {
  int val = param.asInt();

  if (val == 1) {
    Serial.println("RELAY_ON");   // start motor
    motorRunning = true;
    motorStartTime = millis();
  }
}

void setup() {
  Serial.begin(9600);
  Blynk.begin("", ssid, pass);
}

void loop() {
  Blynk.run();

  // ⏱ AUTO STOP AFTER 3 SECONDS
  if (motorRunning && millis() - motorStartTime >= 3000) {
    Serial.println("RELAY_AUTO");   // stop motor
    motorRunning = false;

    Blynk.virtualWrite(V6, 0);      // reset button
  }

  // 📩 RECEIVE DATA FROM ARDUINO
  if (Serial.available()) {
    dataString = Serial.readStringUntil('\n');
    dataString.trim();

    if (dataString == "RELAY_DONE") {
      Blynk.virtualWrite(V6, 0);
      return;
    }

    String values[6];
    int index = 0;

    for (int i = 0; i < dataString.length(); i++) {
      if (dataString[i] == ',') index++;
      else values[index] += dataString[i];
    }

    float ph = values[0].toFloat();
    float tds = values[1].toFloat();
    int turb = values[2].toInt();
    int level = values[3].toInt();
    int glass = values[4].toInt();
    String status = values[5];

    Blynk.virtualWrite(V0, ph);
    Blynk.virtualWrite(V1, tds);
    Blynk.virtualWrite(V2, turb);
    Blynk.virtualWrite(V3, level);
    Blynk.virtualWrite(V4, glass);
    Blynk.virtualWrite(V5, status);
  }
}