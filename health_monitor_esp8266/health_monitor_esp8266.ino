#define BLYNK_TEMPLATE_ID "TMPL3koNUpscI"
#define BLYNK_TEMPLATE_NAME "Health Monitoring"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid = "YOUR_WIFI_SSID";
char pass = "YOUR_WIFI_PASSWORD";

// Timing variable to prevent flooding Blynk
unsigned long lastUpdate = 0;

void setup() {
  // Must match the ATmega328P Baud Rate
  Serial.begin(115200); 
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();

  if (Serial.available() > 0) {
    // Read the incoming line from ATmega328P
    String data = Serial.readStringUntil('\n');
    data.trim(); // Remove any stray whitespace

    // Parsing the CSV format: BPM,gForce,fallStatus,f1,f2
    int c1 = data.indexOf(',');
    int c2 = data.indexOf(',', c1 + 1);
    int c3 = data.indexOf(',', c2 + 1);
    int c4 = data.indexOf(',', c3 + 1);

    // Validation: Ensure all 4 commas exist
    if (c1 != -1 && c2 != -1 && c3 != -1 && c4 != -1) {
      String sBpm  = data.substring(0, c1);
      String sG    = data.substring(c1 + 1, c2);
      String sFall = data.substring(c2 + 1, c3);
      String sF1   = data.substring(c3 + 1, c4);
      String sF2   = data.substring(c4 + 1);

      int bpmVal   = sBpm.toInt();
      float gVal   = sG.toFloat();
      int fallVal  = sFall.toInt();
      int f1Val    = sF1.toInt();
      int f2Val    = sF2.toInt();

      // Update Blynk (V1: BPM, V2: G-Force)
      Blynk.virtualWrite(V1, bpmVal);
      Blynk.virtualWrite(V2, gVal);
      
      // V3: Flex Sensor 1 (Need Water)
      // Matching threshold 150 from your UNO code
      if(f1Val < 150) {
        Blynk.virtualWrite(V3, "Status: OK");
      } else {
        Blynk.virtualWrite(V3, "I Need Water!!");
      }

      // V4: Flex Sensor 2 (Need Air)
      // Matching threshold 130 from your UNO code
      if(f2Val < 130) {
        Blynk.virtualWrite(V4, "Status: OK");
      } else {
        Blynk.virtualWrite(V4, "I Need Fresh Air!!");
      }

      // V5: Fall Detection
      if(fallVal == 1) {
        Blynk.virtualWrite(V5, "!!! FALL DETECTED !!!");
        // Only log event if using Blynk IoT Notifications
        Blynk.logEvent("fall_alert", "Emergency! A fall has been detected!");
      } else {
        Blynk.virtualWrite(V5, "User Safe");
      }
    }
  }
}