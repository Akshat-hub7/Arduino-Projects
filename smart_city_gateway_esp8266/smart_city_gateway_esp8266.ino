#define BLYNK_TEMPLATE_ID "TMPL3BIGB7M3p"
#define BLYNK_TEMPLATE_NAME "Smart City"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi Credentials
char ssid = "YOUR_WIFI_SSID";
char pass = "YOUR_WIFI_PASSWORD";

void setup() {
  // Serial baud rate MUST match the Arduino Uno (9600)
  Serial.begin(9600);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  Serial.println("NodeMCU Online");
}

void loop() {
  Blynk.run();

  // Check for incoming data from Arduino Uno
  if (Serial.available() > 0) {
    // Expecting: "air,soil,fire,traffic" (e.g., "450,65,0,1")
    String data = Serial.readStringUntil('\n');
    data.trim();

    if (data.length() > 0) {
      int firstComma = data.indexOf(',');
      int secondComma = data.indexOf(',', firstComma + 1);
      int thirdComma = data.indexOf(',', secondComma + 1);

      if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
        String air = data.substring(0, firstComma);
        String soil = data.substring(firstComma + 1, secondComma);
        String fire = data.substring(secondComma + 1, thirdComma);
        String traffic = data.substring(thirdComma + 1);

        // Push to Blynk Virtual Pins
        Blynk.virtualWrite(V0, air.toInt());      // Gauge for Air
        Blynk.virtualWrite(V1, soil.toInt());     // Level for Soil %
        
        // Fire Status
        if (fire == "1") {
          Blynk.virtualWrite(V2, "!! FIRE ALERT !!");
          Blynk.logEvent("fire_detected", "Emergency! Fire detected in the city.");
        } else {
          Blynk.virtualWrite(V2, "City Safe");
        }

        // Traffic Status
        if (traffic == "1") {
          Blynk.virtualWrite(V3, "RED LIGHT VIOLATION");
        } else {
          Blynk.virtualWrite(V3, "Traffic Normal");
        }
      }
    }
  }
}