# Arduino Projects

A collection of Arduino and ESP-based prototypes. Each project is kept in its own folder, and each `.ino` file uses the same name as its folder so it opens directly in the Arduino IDE.

## Project index

| Project | Platform | Overview |
|---|---|---|
| [air_piano_arduino](air_piano_arduino/) | Arduino | Gesture-controlled musical notes using distance sensors and a DFPlayer Mini. |
| [air_piano_esp8266](air_piano_esp8266/) | ESP8266 | Wi-Fi/Blynk version of the air-piano controller. |
| [water_quality_monitor_arduino](water_quality_monitor_arduino/) | Arduino | Water-quality readings with an I2C LCD and relay control. |
| [water_quality_monitor_esp8266](water_quality_monitor_esp8266/) | ESP8266 | Wi-Fi/Blynk water-monitoring sketch. |
| [anti_theft_floor_esp32](anti_theft_floor_esp32/) | ESP32 | Sensor-based floor intrusion alert with Blynk reporting. |
| [water_quality_console_arduino](water_quality_console_arduino/) | Arduino | Water-quality sensor readings, OLED display, relay, and serial link. |
| [water_quality_cloud_esp8266](water_quality_cloud_esp8266/) | ESP8266 | Blynk companion for water-monitoring data. |
| [impact_location_alert_esp32](impact_location_alert_esp32/) | ESP32 | FSR-triggered GPS location alerts through Telegram. |
| [gps_geolinker_tracker](gps_geolinker_tracker/) | Arduino-compatible board | GPS tracking using the GeoLinker library. |
| [impact_detection_esp32](impact_detection_esp32/) | ESP32 | MPU6050/FSR impact detection, GPS location, buzzer, and Telegram alerts. |
| [health_monitor_arduino](health_monitor_arduino/) | Arduino | Sensor-based health and fall-monitoring prototype with display/audio output. |
| [health_monitor_blynk_esp8266](health_monitor_blynk_esp8266/) | ESP8266 | Health dashboard using heart-rate, temperature, and motion data. |
| [health_monitor_esp8266](health_monitor_esp8266/) | ESP8266 | Blynk receiver for serial health and fall-monitoring data. |
| [medicine_reminder_arduino](medicine_reminder_arduino/) | Arduino | RTC-based medicine reminder with indicators and alarms. |
| [medicine_reminder_esp8266](medicine_reminder_esp8266/) | ESP8266 | Wi-Fi/Blynk medicine-reminder sketch. |
| [motion_camera_esp32](motion_camera_esp32/) | ESP32-CAM | Motion-triggered camera and notification prototype. |
| [parking_status_esp8266](parking_status_esp8266/) | ESP8266 | Blynk bridge for parking-slot status and gate commands. |
| [smart_city_gate_esp8266](smart_city_gate_esp8266/) | ESP8266 | Blynk-connected parking gate with entry/exit counting. |
| [parking_display_arduino](parking_display_arduino/) | Arduino | Local parking status display and servo control. |
| [ph_monitor_arduino](ph_monitor_arduino/) | Arduino | pH input, water-quality sensors, OLED display, and alarm/relay logic. |
| [scarecrow_controller_esp8266](scarecrow_controller_esp8266/) | ESP8266 | Wi-Fi/Blynk scarecrow controller with servo output. |
| [smart_city_controller_arduino](smart_city_controller_arduino/) | Arduino | Local smart-city sensor/control prototype. |
| [smart_city_gateway_esp8266](smart_city_gateway_esp8266/) | ESP8266 | Blynk gateway for serial smart-city sensor data. |
| [smart_plant_monitor_esp8266](smart_plant_monitor_esp8266/) | ESP8266 | Plant/environment monitoring with display, sensors, and audio output. |
| [water_quality_display_arduino](water_quality_display_arduino/) | Arduino | Turbidity/TDS monitoring with OLED status display and relay output. |
| [water_tank_monitor_esp8266](water_tank_monitor_esp8266/) | ESP8266 | Blynk-enabled water-tank monitoring prototype. |

Related sketches are retained when their board, sensor set, or behavior differs. A redundant crash-alert sample was excluded from the published collection.

## Getting started

1. Install the Arduino IDE and the board support package for the board named in the project index.
2. Open the corresponding project folder (or its `.ino` file) in the IDE.
3. Install the libraries required by that sketch. Check its `#include` lines; commonly used libraries here include Blynk, Adafruit sensor/display libraries, TinyGPS++, UniversalTelegramBot, DFRobotDFPlayerMini, RTClib, GeoLinker, and ArduinoJson.
4. Review the pin definitions and wire the hardware for that specific sketch.
5. Add your own credentials and service configuration as described below, select the board and port, then compile and upload.

These are independent prototypes, not one combined firmware. Pin assignments, sensor thresholds, serial formats, and Blynk virtual pins are sketch-specific.

## Credentials and configuration

No Blynk template identifiers, Blynk authentication tokens, API keys, Wi-Fi passwords, or Telegram bot credentials are stored in the published sketches. Network-enabled examples use placeholder Wi-Fi settings. Blynk sketches pass an empty token to `Blynk.begin`; add your token locally to that call if you want to connect them. The GPS tracker omits GeoLinker API-key configuration, and the plant monitor runs locally with its ThingSpeak upload removed. Do not commit real credentials.

If any previously embedded token or password was active, revoke or rotate it before reuse.

## Screenshots

No application or hardware screenshots were supplied with the source files. These projects are Arduino firmware rather than a standalone application, so screenshots have not been fabricated. Add genuine Blynk dashboard or device screenshots under `docs/screenshots/` and link them here when available.

## Safety

These sketches are educational prototypes, not certified medical, vehicle-safety, security, or emergency-response systems. Verify sensor behavior and wiring on the bench; do not rely on them for real-world safety decisions.
