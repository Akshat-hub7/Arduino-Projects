#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
// --- Pin Definitions ---
#define DHTPIN 14         // D5
#define DHTTYPE DHT11
#define SOIL_PIN A0
#define RELAY_PIN 12      // D6

// --- DFPlayer Pins ---
// If D3/D4 (0,2) cause issues, try D1/D2 (5,4)
SoftwareSerial mySoftwareSerial(0, 2); 
DFRobotDFPlayerMini myDFPlayer;

// --- OLED Setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  mySoftwareSerial.begin(9600);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay OFF (assuming Active Low)

  // OLED Init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED failed"));
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("SYSTEM STARTING...");
  display.display();

  dht.begin();

  // DFPlayer Init
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("DFPlayer Error!"));
  } else {
    myDFPlayer.volume(30);
    myDFPlayer.playMp3Folder(1); 
  }

}

void loop() {
  // 1. Read Sensors
  int rawSoil = analogRead(SOIL_PIN);
  int moisture = map(rawSoil, 1023, 400, 0, 100);
  moisture = constrain(moisture, 0, 100);
  
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Check if DHT readings are valid
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }

  // 2. Update Display (All 3 stats)
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println("PLANT STATUS");
  display.drawFastHLine(0, 10, 128, WHITE);

  // Soil Moisture (Large)
  display.setCursor(0, 18);
  display.setTextSize(1);
  display.print("SOIL MOIST: ");
  display.setTextSize(2);
  display.print(moisture); display.println("%");

  // Temp & Humidity (Small)
  display.setTextSize(1);
  display.setCursor(0, 42);
  display.print("TEMP: "); display.print(t, 1); display.println(" C");
  display.setCursor(0, 54);
  display.print("HUMI: "); display.print(h, 1); display.println(" %");
  
  display.display();

  // 4. Watering Logic (3 Seconds) + Restart
  if (moisture < 30) {
    display.clearDisplay();
    display.setCursor(15, 25);
    display.setTextSize(2);
    display.println("WATERING!");
    display.display();

    // Sound + Motor On
    myDFPlayer.playMp3Folder(2); 
    digitalWrite(RELAY_PIN, LOW); 
    
    // 3 Second Watering Timer
    delay(2000); 
    
    // Motor Off
    digitalWrite(RELAY_PIN, HIGH);
    
    Serial.println("Watering done. Restarting...");
    delay(1000); 
    
    // REBOOT
    ESP.restart(); 
  }

  delay(3000); // 5-second interval between checks
}