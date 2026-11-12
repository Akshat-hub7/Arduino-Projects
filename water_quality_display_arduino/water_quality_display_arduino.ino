#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin Definitions
const int relayPin = 8;
const int trigPin = 9;
const int echoPin = 10;
const int turbidityPin = A0;
const int tdsPin = A1;

u
void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(relayPin, LOW); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;); 
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  // 1. Read Sensors
  int turbRaw = analogRead(turbidityPin);
  int tdsRaw = analogRead(tdsPin);
  
  // Basic TDS Conversion (Check your sensor's manual for specific voltage constants)
  float voltage = tdsRaw * 5.0 / 1024.0;
  float tdsPpm = (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage) * 0.5;

  // 2. Ultrasonic Distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  // 3. Logic: Motor ON if water is clear (low turb) AND object is near (< 20cm)
  // Adjust thresholds based on your testing
  if (turbRaw < 400 && distance < 20 && distance > 0) {
    digitalWrite(relayPin, HIGH);
    updateDisplay(turbRaw, tdsPpm, distance, "RUNNING");
  } else {
    digitalWrite(relayPin, LOW);
    updateDisplay(turbRaw, tdsPpm, distance, "STANDBY");
  }
  
  delay(500); 
}

void updateDisplay(int turb, float tds, int dist, String status) {
  display.clearDisplay();
  display.setCursor(0,0);
  
  display.println("--- SYSTEM STATUS ---");
  display.print("Turbidity: "); display.println(turb);
  display.print("TDS: "); display.print(tds, 0); display.println(" ppm");
  display.print("Dist: "); display.print(dist); display.println(" cm");
  display.println("---------------------");
  display.print("Motor: "); display.println(status);
  
  display.display();
}