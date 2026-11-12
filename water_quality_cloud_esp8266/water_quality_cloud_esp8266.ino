#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// ---------- WIFI ----------
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// ---------- VARIABLES ----------
String rawPH;
float tds;
int turb;
int glassDetected;
String status;

// ---------- BLYNK BUTTON ----------
BLYNK_WRITE(V4) {
  int state = param.asInt();

  if (state == 1) {
    Serial.println("1");   // send ON to UNO
  } else {
    Serial.println("0");   // send OFF to UNO
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  Blynk.begin("", ssid, pass);
}

// ---------- LOOP ----------
void loop() {
  Blynk.run();

  // ===== RECEIVE FROM UNO =====
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');

    int p1 = data.indexOf(',');
    int p2 = data.indexOf(',', p1 + 1);
    int p3 = data.indexOf(',', p2 + 1);
    int p4 = data.indexOf(',', p3 + 1);

    if (p1 > 0 && p2 > 0 && p3 > 0 && p4 > 0) {
      rawPH = data.substring(0, p1);
      tds = data.substring(p1 + 1, p2).toFloat();
      turb = data.substring(p2 + 1, p3).toInt();
      glassDetected = data.substring(p3 + 1, p4).toInt();
      status = data.substring(p4 + 1);
    }
  }

  // ===== SEND TO BLYNK =====
  Blynk.virtualWrite(V0, rawPH);
  Blynk.virtualWrite(V1, tds);
  Blynk.virtualWrite(V2, turb);
  Blynk.virtualWrite(V3, status);

  if (glassDetected)
    Blynk.virtualWrite(V5, "Glass Detected");
  else
    Blynk.virtualWrite(V5, "No Glass");

  delay(500);
}