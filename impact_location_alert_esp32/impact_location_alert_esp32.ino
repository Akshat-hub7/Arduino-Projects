#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ⚠️ IMPORTANT: You should change this token as it was shared publicly
#define BOTtoken "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID "YOUR_TELEGRAM_CHAT_ID"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000; // Increased slightly for stability
unsigned long lastTimeBotRan;

HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

// --- FSR CONFIGURATION ---
#define FSR_PIN 34          // GPIO 34 is an Analog Input (ADC1)
int fsrThreshold = 1500;    // Adjust this (0-4095) based on required pressure
bool isPressed = false;
unsigned long lastTriggerTime = 0;
unsigned long triggerDelay = 2000; // Prevent spamming messages (2 seconds)

void sendLocationToTelegram() {
  if (gps.location.isValid()) {
    float lat = gps.location.lat();
    float lng = gps.location.lng();

    String lokasi = "📍 Live Location (FSR Triggered):\nhttps://www.google.com/maps/@";
    lokasi += String(lat, 6);
    lokasi += ",";
    lokasi += String(lng, 6);
    lokasi += ",21z";

    bot.sendMessage(CHAT_ID, lokasi, "");
    Serial.println("Location sent via FSR");
  } else {
    bot.sendMessage(CHAT_ID, "GPS not fixed yet!", "");
    Serial.println("GPS Fix missing");
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) continue;

    String text = bot.messages[i].text;
    if (text == "/start") {
      bot.sendMessage(chat_id, "FSR GPS Tracker Online.\n/location - Get current location", "");
    }
    if (text == "/location") {
      sendLocationToTelegram();
    }
  }
}

void setup() {
  Serial.begin(115200);
  GPSSerial.begin(9600, SERIAL_8N1, 16, 17);

  // FSR Pin doesn't strictly need pinMode for analogRead, 
  // but it's good practice. Note: No PULLUP needed due to external resistor.
  pinMode(FSR_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

void loop() {
  // 📡 Continuously read GPS
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  // 🔘 FSR Pressure Reading
  int fsrReading = analogRead(FSR_PIN);

  // If pressure is above threshold and cooldown has passed
  if (fsrReading > fsrThreshold && !isPressed && (millis() - lastTriggerTime > triggerDelay)) {
    isPressed = true;
    lastTriggerTime = millis();
    Serial.print("FSR Triggered! Reading: ");
    Serial.println(fsrReading);
    sendLocationToTelegram();
  } 
  
  // Reset the pressed state when pressure is released
  if (fsrReading < (fsrThreshold - 200)) { 
    isPressed = false;
  }

  // 🤖 Telegram polling
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}