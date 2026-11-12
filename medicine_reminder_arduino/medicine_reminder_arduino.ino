#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

// Updated to 3 pins each
const int irPins[] = {2, 3, 4};   
const int ledPins[] = {8, 9, 10};
const int buzzer = 12;

// Updated to 3 slots
String pillTimes[3] = {"08:00", "12:00", "18:00"};
bool triggered[3] = {false, false, false};

void setup() {
  Serial.begin(9600); 
  Wire.begin();
  rtc.begin();

  // Loop runs 3 times now
  for (int i = 0; i < 3; i++) {
    pinMode(irPins[i], INPUT);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  pinMode(buzzer, OUTPUT);

  // --- HARDWARE SELF-TEST ---
  for(int i=0; i<3; i++) digitalWrite(ledPins[i], HIGH);
  tone(buzzer, 2000);
  delay(2000);
  for(int i=0; i<3; i++) digitalWrite(ledPins[i], LOW);
  noTone(buzzer);
}

void loop() {
  DateTime now = rtc.now();
  
  // 1. Send Time to Serial/Blynk
  char fullTime[12]; 
  sprintf(fullTime, "<TIME:%02d:%02d:%02d>", now.hour(), now.minute(), now.second());
  Serial.println(fullTime);

  // 2. Format Time for Comparison (HH:mm)
  char checkTime[6]; 
  sprintf(checkTime, "%02d:%02d", now.hour(), now.minute());
  String currentMatch = String(checkTime);

  // 3. Check Alarms for 3 slots
  for (int i = 0; i < 3; i++) {
    if (currentMatch == pillTimes[i] && !triggered[i]) {
      runAlarm(i);
      triggered[i] = true;
    }
    if (currentMatch != pillTimes[i]) {
      triggered[i] = false;
    }
  }

  // 4. Listen for Time Updates (Removed T4)
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("T1:")) pillTimes[0] = input.substring(3, 8);
    if (input.startsWith("T2:")) pillTimes[1] = input.substring(3, 8);
    if (input.startsWith("T3:")) pillTimes[2] = input.substring(3, 8);
  }
  delay(1000); 
}

void runAlarm(int slot) {
  unsigned long startTime = millis();
  bool taken = false;
  
  Serial.println("<STATUS:Medicine " + String(slot + 1) + " ALERT!>");

  while (!taken && (millis() - startTime < 60000)) {
    digitalWrite(ledPins[slot], HIGH);
    tone(buzzer, 3000, 200);
    
    if (digitalRead(irPins[slot]) == LOW) {
      digitalWrite(ledPins[slot], LOW);
      noTone(buzzer);
      Serial.println("<STATUS:Medicine " + String(slot + 1) + " TAKEN>");
      taken = true;
      delay(3000); 
    }
    delay(400); 
  }

  if (!taken) {
    digitalWrite(ledPins[slot], LOW);
    noTone(buzzer);
    Serial.println("<MISSED:Medicine " + String(slot + 1) + " NOT taken!>");
  }
}