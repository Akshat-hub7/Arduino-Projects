#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// =========================
// DFPLAYER
// =========================
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// =========================
// ULTRASONIC SENSOR PINS
// =========================
const int trigPin = 2;

const int echoPins[] = {
  3, 4, 5, 6, 7
};

const int numSensors = 5;

// =========================
// DRUM BUTTON
// =========================
const int drumButton = 8;

// =========================
// SETTINGS
// =========================
const int distanceThreshold = 25;
const int minDistance = 3;

bool isPlaying[] = {
  false, false, false, false, false
};

bool lastButtonState = HIGH;

void setup() {

  Serial.begin(115200);

  mySoftwareSerial.begin(9600);

  pinMode(13, OUTPUT);

  // Drum button
  pinMode(drumButton, INPUT_PULLUP);

  // Ultrasonic pins
  pinMode(trigPin, OUTPUT);

  for (int i = 0; i < numSensors; i++) {
    pinMode(echoPins[i], INPUT);
  }

  // DFPlayer init
  if (!myDFPlayer.begin(mySoftwareSerial)) {

    while (true) {
      digitalWrite(13, HIGH);
      delay(200);
      digitalWrite(13, LOW);
      delay(200);
    }
  }

  myDFPlayer.volume(25);
}

void loop() {

  // =========================
  // ARDUINO DRUM BUTTON
  // =========================
  bool buttonState = digitalRead(drumButton);

  // Button pressed
  if (lastButtonState == HIGH && buttonState == LOW) {

    myDFPlayer.volume(30);
    myDFPlayer.play(6);

    Serial.println("NOTE:6");

    blinkLed();
  }

  lastButtonState = buttonState;

  // =========================
  // RECEIVE ESP8266 COMMANDS
  // =========================
  if (Serial.available() > 0) {

    String command = Serial.readStringUntil('\n');

    command.trim();

    if (command.startsWith("PLAY")) {

      int note = command.substring(4).toInt();

      if (note >= 1 && note <= 6) {

        myDFPlayer.volume(25);
        myDFPlayer.play(note);

        Serial.print("NOTE:");
        Serial.println(note);

        blinkLed();
      }
    }
  }

  // =========================
  // ULTRASONIC AIR PIANO
  // =========================
  for (int i = 0; i < numSensors; i++) {

    long distance = getDistance(echoPins[i]);

    if (distance > 0 && distance < distanceThreshold) {

      int dynamicVol = map(
        distance,
        minDistance,
        distanceThreshold,
        30,
        5
      );

      dynamicVol = constrain(dynamicVol, 5, 30);

      if (!isPlaying[i]) {

        myDFPlayer.volume(dynamicVol);

        // Notes 1-5
        myDFPlayer.play(i + 1);

        Serial.print("NOTE:");
        Serial.println(i + 1);

        isPlaying[i] = true;

        blinkLed();

        delay(80);
      }

    } else {

      isPlaying[i] = false;
    }
  }

  delay(20);
}

// =========================
// DISTANCE FUNCTION
// =========================
long getDistance(int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);

  if (duration == 0)
    return 999;

  return duration * 0.034 / 2;
}

// =========================
// LED BLINK
// =========================
void blinkLed() {

  digitalWrite(13, HIGH);
  delay(60);

  digitalWrite(13, LOW);
}