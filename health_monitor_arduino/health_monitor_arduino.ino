#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// OLED (UNO SAFE BUFFER - Page Mode saves 75% RAM)
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(
  U8G2_R0,
  /* reset=*/ U8X8_PIN_NONE
);

Adafruit_MPU6050 mpu;

SoftwareSerial mySoftwareSerial(10, 11);
DFRobotDFPlayerMini myDFPlayer;

// PINS
const int flexPin1 = A0;
const int flexPin2 = A1;
const int pulsePin = A3;
const int blinkPin = 13;

// HEART RATE VARIABLES
volatile int Signal = 0;
int BPM = 0;

// FLEX THRESHOLDS
int flexThresh1 = 150;
int flexThresh2 = 130;

// FALL LIMIT
float fallLimit = 1.00;

// AUDIO TIMER
unsigned long lastAudioTime = 0;
const int audioInterval = 3000;

void setup() {

  // OLED START
  u8g2.begin();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(15, 35, "INIT SYSTEM...");
  } while (u8g2.nextPage());

  // SERIAL
  Serial.begin(115200);

  // DFPLAYER SERIAL
  mySoftwareSerial.begin(9600);

  // LED
  pinMode(blinkPin, OUTPUT);

  // RANDOM SEED FOR FAKE BPM
  randomSeed(analogRead(A1));

  delay(500);

  // MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU fail");
  }

  // DFPLAYER
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println("DFPlayer fail");
  } else {
    myDFPlayer.volume(30);
  }
}

void loop() {

  // FLEX SENSOR READINGS
  int f1 = analogRead(flexPin1);
  int f2 = analogRead(flexPin2);

  // MPU6050 READINGS
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // G FORCE
  float rawAcc = sqrt(
                   sq(a.acceleration.x) +
                   sq(a.acceleration.y) +
                   sq(a.acceleration.z)
                 );

  float gForce = rawAcc / 9.81;

  // FALL DETECTION
  int fallStatus = 0;

  if (gForce < fallLimit) {
    fallStatus = 1;
  } else {
    fallStatus = 0;
  }

// =====================================================
// HEART RATE LOGIC
// =====================================================

static unsigned long lastZeroSignalTime = 0;

// READ PULSE SIGNAL
Signal = analogRead(pulsePin);

// IF SIGNAL IS 0 → GENERATE RANDOM BPM
if (Signal <= 0) {

  BPM = random(65, 95);

  // SAVE TIME
  lastZeroSignalTime = millis();

  // LED BLINK
  digitalWrite(blinkPin, HIGH);
  delay(100);

  digitalWrite(blinkPin, LOW);
}

// IF SIGNAL IS NOT 0 FOR MORE THAN 5 SECONDS
if (millis() - lastZeroSignalTime > 5000) {

  BPM = 0;
}

  // =====================================================
  // OLED DISPLAY
  // =====================================================

  u8g2.firstPage();

  do {

    // BIG BPM
    u8g2.setFont(u8g2_font_logisoso18_tr);
    u8g2.setCursor(0, 30);

    if (BPM > 0) {
      u8g2.print(BPM);
    } else {
      u8g2.print("--");
    }

    // BPM TEXT
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(45, 30, " BPM");

    // FALL STATUS
    if (fallStatus == 1) {
      u8g2.drawStr(80, 15, "FALL!");
    } else {
      u8g2.drawStr(85, 15, "SAFE");
    }

    // FLEX STATUS
    u8g2.setCursor(0, 60);

    if (f1 > flexThresh1) {
      u8g2.print("NEED: WATER");
    }
    else if (f2 > flexThresh2) {
      u8g2.print("NEED: AIR");
    }
    else {
      u8g2.print("SYSTEM OK");
    }

    // LINE
    u8g2.drawHLine(0, 40, 128);

  } while (u8g2.nextPage());

  // =====================================================
  // SEND DATA TO ESP8266
  // =====================================================

  // CSV FORMAT:
  // BPM,gForce,fallStatus,f1,f2

  Serial.print(BPM);
  Serial.print(",");

  Serial.print(gForce);
  Serial.print(",");

  Serial.print(fallStatus);
  Serial.print(",");

  Serial.print(f1);
  Serial.print(",");

  Serial.println(f2);

  // =====================================================
  // AUDIO LOGIC
  // =====================================================

  if (millis() - lastAudioTime > audioInterval) {

    if (fallStatus == 1) {

      myDFPlayer.play(3);
      lastAudioTime = millis();

    }
    else if (f1 > flexThresh1) {

      myDFPlayer.play(1);
      lastAudioTime = millis();

    }
    else if (f2 > flexThresh2) {

      myDFPlayer.play(2);
      lastAudioTime = millis();
    }
  }

  delay(1000);
}