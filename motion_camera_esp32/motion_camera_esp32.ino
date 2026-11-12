#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// --- CONFIGURATION ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Telegram bot
const char* botToken = "YOUR_TELEGRAM_BOT_TOKEN";
const char* chatID = "YOUR_TELEGRAM_CHAT_ID";

#define IR_PIN 13           // IR Sensor connected to GPIO 13
unsigned long lastTriggerTime = 0;
const unsigned long triggerCooldown = 15000; // 15 seconds

// --- CAMERA PIN DEFINITIONS (AI-THINKER) ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiClientSecure clientTCP;

void sendPhotoTelegram(camera_fb_t * fb) {
  clientTCP.setInsecure(); // Skip SSL certificate validation
  
  if (!clientTCP.connect("api.telegram.org", 443)) {
    Serial.println("Connection to Telegram failed");
    return;
  }

  String boundary = "ESP32CAMBOUNDARY";
  String startRequest = "--" + boundary + "\r\n";
  startRequest += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + String(chatID) + "\r\n";
  startRequest += "--" + boundary + "\r\n";
  startRequest += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n🎯 IR Sensor Triggered!\r\n";
  startRequest += "--" + boundary + "\r\n";
  startRequest += "Content-Disposition: form-data; name=\"photo\"; filename=\"image.jpg\"\r\n";
  startRequest += "Content-Type: image/jpeg\r\n\r\n";
  String endRequest = "\r\n--" + boundary + "--\r\n";

  size_t totalLen = startRequest.length() + fb->len + endRequest.length();

  clientTCP.print("POST /bot" + String(botToken) + "/sendPhoto HTTP/1.1\r\n");
  clientTCP.print("Host: api.telegram.org\r\n");
  clientTCP.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
  clientTCP.print("Content-Length: " + String(totalLen) + "\r\n\r\n");

  clientTCP.print(startRequest);
  clientTCP.write(fb->buf, fb->len);
  clientTCP.print(endRequest);

  Serial.println("📸 Photo sent!");
  clientTCP.stop();
}

void setup() {
  Serial.begin(115200);
  
  // IR sensors are usually ACTIVE LOW (output 0 when they see something)
  pinMode(IR_PIN, INPUT_PULLUP);

  // Camera Setup
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Optimized for stability (Low memory usage)
  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    return;
  }

  // WiFi Setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void loop() {
  // Check if IR sensor detects an object (LOW signal)
  bool isDetected = (digitalRead(IR_PIN) == LOW);

  if (isDetected && (millis() - lastTriggerTime > triggerCooldown)) {
    lastTriggerTime = millis();
    Serial.println("🚨 Alert! IR Sensor Triggered.");

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Failed to capture photo");
      return;
    }

    sendPhotoTelegram(fb);
    esp_camera_fb_return(fb);
  }
}