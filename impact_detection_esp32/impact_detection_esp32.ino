#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// --- Configuration ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

#define BOTtoken "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID "YOUR_TELEGRAM_CHAT_ID"

// --- Pins ---
#define FSR_PIN 34       // Analog pin for FSR
#define BUTTON_PIN 4     // Manual SOS button
#define GPS_RX 16
#define GPS_TX 17

// --- FSR Threshold ---
const int FSR_THRESHOLD = 2000;  
// Adjust according to your sensor values

// --- Telegram Timing ---
unsigned long botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// --- Objects ---
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

// =====================================================
// FUNCTION TO SEND LOCATION
// =====================================================
void sendLocationToTelegram(bool isImpact = false) {

  // Read GPS data
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  if (gps.location.isValid()) {

    String message;

    if (isImpact) {
      message = "⚠️ IMPACT DETECTED!\n";
    } else {
      message = "📍 Manual Location Update\n";
    }

    message += "Location:\n";
    message += "http://www.google.com/maps/place/";
    message += String(gps.location.lat(), 6);
    message += ",";
    message += String(gps.location.lng(), 6);

    bot.sendMessage(CHAT_ID, message, "");

    Serial.println("Location sent to Telegram");

  } else {

    bot.sendMessage(CHAT_ID,
                    "⚠️ Impact detected but GPS signal not available!",
                    "");

    Serial.println("GPS not fixed");
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  // GPS Serial
  GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // Pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // WiFi Connection
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println("System Ready");
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  // ---------------------------------
  // Read GPS Continuously
  // ---------------------------------
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  // ---------------------------------
  // Read FSR Sensor
  // ---------------------------------
  int fsrValue = analogRead(FSR_PIN);

  Serial.print("FSR Value: ");
  Serial.println(fsrValue);

  // Detect Strong Pressure/Impact
  if (fsrValue > FSR_THRESHOLD) {

    Serial.println("IMPACT DETECTED!");

    sendLocationToTelegram(true);

    delay(5000); // Prevent multiple alerts
  }

  // ---------------------------------
  // Manual SOS Button
  // ---------------------------------
  if (digitalRead(BUTTON_PIN) == LOW) {

    Serial.println("Manual SOS Triggered");

    sendLocationToTelegram(false);

    delay(1000);
  }

  // ---------------------------------
  // Telegram Commands
  // ---------------------------------
  if (millis() > lastTimeBotRan + botRequestDelay) {

    int numNewMessages =
      bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      for (int i = 0; i < numNewMessages; i++) {

        String text = bot.messages[i].text;

        if (text == "/location") {
          sendLocationToTelegram(false);
        }
      }

      numNewMessages =
        bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }

  delay(100);
}