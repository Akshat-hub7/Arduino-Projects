#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

// 🔑 WiFi credentials
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

#define TRIG 14
#define ECHO 12
#define LED 13
#define BUZZER 15
#define RAIN A0

Servo servo1;
Servo servo2;

long duration;
int distance;

BlynkTimer timer;

// 🔄 Servo movement function
void moveServos() {
  for (int i = 0; i < 4; i++) {
    servo1.write(90);
    servo2.write(90);
    delay(400);

    servo1.write(0);
    servo2.write(180);
    delay(400);
  }
}

// 📱 Button control (V4)
BLYNK_WRITE(V4) {
  int state = param.asInt();
  if (state == 1) {
    Serial.println("Manual Button Pressed!");
    moveServos();
  }
}

void sendData() {

  // Ultrasonic
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH, 30000);
  distance = duration * 0.034 / 2;

  // Rain sensor
  int rainValue = analogRead(RAIN);
  int rainCM = map(rainValue, 900, 300, 0, 10);
  rainCM = constrain(rainCM, 0, 10);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Rain: ");
  Serial.println(rainCM);

  // Send numeric values
  Blynk.virtualWrite(V0, distance);
  Blynk.virtualWrite(V1, rainCM);

  // 🌧 Rain label
  if (rainCM > 0) {
    Blynk.virtualWrite(V3, "Rain Detected");
    Serial.println("Rain Detected!");
  } else {
    Blynk.virtualWrite(V3, "No Rain");
  }

  // 🔴 Object detection
  if (distance > 0 && distance < 30) {

    Blynk.virtualWrite(V2, "Object Detected");

    digitalWrite(LED, HIGH);

    // 🔊 Buzzer beeping
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
    }

    moveServos();

  } else {
    digitalWrite(LED, LOW);
    digitalWrite(BUZZER, LOW);
    Blynk.virtualWrite(V2, "No Object");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  servo1.attach(5);
  servo2.attach(4);

  Blynk.begin("", ssid, pass);

  timer.setInterval(1000L, sendData);
}

void loop() {
  Blynk.run();
  timer.run();
}