#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

String data = "";

void setup() {
  Serial.begin(9600);

  Blynk.begin("", ssid, pass);
}

void loop() {
  Blynk.run();

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      processData(data);
      data = "";
    } else {
      data += c;
    }
  }
}

// 🔥 PROCESS DATA FROM ARDUINO
void processData(String d) {
  int s1, s2, s3, freeSlots;

  sscanf(d.c_str(), "%d,%d,%d,%d", &s1, &s2, &s3, &freeSlots);

  Blynk.virtualWrite(V0, freeSlots); // Free slots
  Blynk.virtualWrite(V1, s1);        // Slot1
  Blynk.virtualWrite(V2, s2);        // Slot2
  Blynk.virtualWrite(V3, s3);        // Slot3
}

// 📱 BUTTON CONTROL FROM BLYNK
BLYNK_WRITE(V4) {
  int val = param.asInt();

  if (val == 1) {
    Serial.write('O');  // OPEN gate
  } else {
    Serial.write('C');  // CLOSE gate
  }
}