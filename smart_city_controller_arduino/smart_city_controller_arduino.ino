#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- PIN ASSIGNMENTS ---
const int BUZZER_PIN = A2;     
const int FLAME_PIN = 2;      
const int BIN_SERVO_PIN = 3;  
const int TRIG_PIN = 6;       
const int ECHO_PIN = 7;       
const int RELAY_PUMP_PIN = 8; 
const int TRAFFIC_RED = 9;    
const int TRAFFIC_YELLOW = 10;
const int TRAFFIC_GREEN = 11;
const int STREET_LIGHT_LED = 12; 
const int IR_TRAFFIC_PIN = 13; 
const int MQ135_PIN = A0;     
const int SOIL_PIN = A1;      
const int LDR_PIN = A3;       

Servo binServo;

// Timing Variables
unsigned long lightTimer = 0;
unsigned long sensorTimer = 0;

// Dustbin delay variables
unsigned long binOpenTime = 0;
bool binOpen = false;

// 🔥 Fire stability variables (FIX ADDED)
unsigned long fireDetectTime = 0;
bool fireStable = false;

int lightState = 0; 
int airQuality = 0;
int soilPercent = 0;
int ldrValue = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(IR_TRAFFIC_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PUMP_PIN, OUTPUT);
  pinMode(TRAFFIC_RED, OUTPUT);
  pinMode(TRAFFIC_YELLOW, OUTPUT);
  pinMode(TRAFFIC_GREEN, OUTPUT);
  pinMode(STREET_LIGHT_LED, OUTPUT); 
  pinMode(LDR_PIN, INPUT);

  binServo.attach(BIN_SERVO_PIN);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. SENSOR UPDATES ---
  if (currentMillis - sensorTimer >= 2000) {
    airQuality = analogRead(MQ135_PIN);
    soilPercent = constrain(map(analogRead(SOIL_PIN), 1023, 200, 0, 100), 0, 100);
    ldrValue = analogRead(LDR_PIN);

    sensorTimer = currentMillis;
    
    Serial.print(airQuality); Serial.print(",");
    Serial.print(soilPercent); Serial.print(",");
    Serial.print(digitalRead(FLAME_PIN) == LOW ? 1 : 0); Serial.print(",");
    Serial.println(lightState == 2 && digitalRead(IR_TRAFFIC_PIN) == LOW ? 1 : 0);
  }

  // --- 2. DUSTBIN LOGIC (5 sec delay) ---
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  int distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;

  if (distance > 0 && distance < 15 && !binOpen) {
    binServo.write(90);
    binOpen = true;
    binOpenTime = millis();
  }

  if (binOpen && millis() - binOpenTime >= 5000) {
    binServo.write(0);
    binOpen = false;
  }

  // --- 3. STREET LIGHT USING LDR ---
  if (ldrValue < 500) {
    digitalWrite(STREET_LIGHT_LED, HIGH);
  } else {
    digitalWrite(STREET_LIGHT_LED, LOW);
  }

  // --- 4. TRAFFIC SYSTEM ---
  if (lightState == 0 && currentMillis - lightTimer > 5000) { lightState = 1; lightTimer = currentMillis; }
  else if (lightState == 1 && currentMillis - lightTimer > 2000) { lightState = 2; lightTimer = currentMillis; }
  else if (lightState == 2 && currentMillis - lightTimer > 5000) { lightState = 0; lightTimer = currentMillis; }

  digitalWrite(TRAFFIC_GREEN, (lightState == 0));
  digitalWrite(TRAFFIC_YELLOW, (lightState == 1));
  digitalWrite(TRAFFIC_RED, (lightState == 2));

  bool redCrossed = (lightState == 2 && digitalRead(IR_TRAFFIC_PIN) == LOW);

  // 🔥 STABLE FIRE DETECTION (FIX)
  bool fireRaw = (digitalRead(FLAME_PIN) == LOW);

  if (fireRaw) {
    if (fireDetectTime == 0) {
      fireDetectTime = millis();
    }
    if (millis() - fireDetectTime > 1000) {
      fireStable = true;
    }
  } else {
    fireDetectTime = 0;
    fireStable = false;
  }

  // --- CONTROL ---
  if (fireStable) {
    digitalWrite(RELAY_PUMP_PIN, HIGH);
    tone(BUZZER_PIN, 2500); 
  } 
  else if (redCrossed) {
    tone(BUZZER_PIN, 400); 
    digitalWrite(RELAY_PUMP_PIN, LOW);
  } 
  else {
    noTone(BUZZER_PIN);
    digitalWrite(RELAY_PUMP_PIN, LOW);
  }

  // --- 5. OLED DISPLAY ---
  display.clearDisplay();

  display.fillRect(0, 0, 128, 16, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(5, 4);
  display.print("AIR:"); display.print(airQuality);
  display.setCursor(70, 4);
  display.print("SOIL:"); display.print(soilPercent); display.print("%");

  display.setTextColor(WHITE);

  if (fireStable) {
    display.setTextSize(2);
    display.setCursor(15, 25); display.println("!!FIRE!!");
    display.setTextSize(1);
    display.setCursor(25, 45); display.print("PUMP ACTIVE");
  } 
  else if (redCrossed) {
    display.setCursor(10, 25); display.println("TRAFFIC VIOLATION");
    display.drawRect(5, 40, 118, 18, WHITE);
    display.setCursor(12, 45); display.print("RED LIGHT CROSSED");
  } 
  else {
    display.setCursor(30, 30);
    display.print("CITY SECURE");

    display.setCursor(20, 50);
    if (ldrValue < 500) {
      display.print("STREET LIGHT ON");
    } else {
      display.print("STREET LIGHT OFF");
    }
  }

  display.display();
}