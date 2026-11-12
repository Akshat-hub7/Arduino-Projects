#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "YOUR_WIFI_SSID"; // Ensure this matches your router/hotspot
char pass[] = "YOUR_WIFI_PASSWORD";

void setup() {
  // Must match the Arduino's Serial.begin(9600)
  Serial.begin(9600);
  Blynk.begin("", ssid, pass);
}

// Receive Time from Blynk App and forward to Arduino ATmega328P
// We only keep V1, V2, and V3
BLYNK_WRITE(V1) { String t = param.asStr(); Serial.println("T1:" + t.substring(0,5)); }
BLYNK_WRITE(V2) { String t = param.asStr(); Serial.println("T2:" + t.substring(0,5)); }
BLYNK_WRITE(V3) { String t = param.asStr(); Serial.println("T3:" + t.substring(0,5)); }

void loop() {
  Blynk.run();

  // Handle incoming data from the Arduino (Time and Status updates)
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('>'); 
    int startIdx = data.indexOf('<');
    
    if (startIdx != -1) {
      String msg = data.substring(startIdx + 1);

      // Updates Clock display in Blynk
      if (msg.startsWith("TIME:")) {
        Blynk.virtualWrite(V6, msg.substring(5));
      } 
      // Updates Status Terminal/Label in Blynk
      else if (msg.startsWith("STATUS:")) {
        Blynk.virtualWrite(V5, msg.substring(7));
      }
      // Sends Alert for missed pills
      else if (msg.startsWith("MISSED:")) {
        String alertMsg = msg.substring(7);
        Blynk.virtualWrite(V5, alertMsg);
        Blynk.logEvent("pill_missed", alertMsg); 
      }
    }
  }
}