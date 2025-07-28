# Installation und Setup

## Voraussetzungen

### Hardware
- ESP32-C3 Development Board
- PCF8574 I2C GPIO Expander Modul
- DS18B20 wasserdichter Temperatursensor
- 4.7kΩ Widerstand (Pull-up für DS18B20)
- 100kΩ und 10kΩ Widerstände (Voltage Divider)
- 4x AA Lithium Batterien
- Rain Bird ESP-BAT-BT-6 Controller

### Software
- Arduino IDE 2.0 oder höher
- ESP32 Board Package
- Benötigte Libraries (siehe unten)

## Arduino IDE Setup

### 1. ESP32 Board Package installieren
```
File → Preferences → Additional Board Manager URLs:
https://espressif.github.io/arduino-esp32/package_esp32_index.json

Tools → Board → Boards Manager → Suche "ESP32" → Install
```

### 2. Board Konfiguration
```
Board: "ESP32C3 Dev Module"
CPU Frequency: "160MHz"
Flash Size: "4MB"
Partition Scheme: "Default 4MB with spiffs"
Upload Speed: "921600"
```

### 3. Libraries installieren
Über Library Manager (Tools → Manage Libraries):

```
- PCF8574 library by Renzo Mischianti
- OneWire by Paul Stoffregen
- DallasTemperature by Miles Burton
- ArduinoJson by Benoit Blanchon
```

## Hardware-Aufbau

### Schritt 1: I2C Verbindungen
```
ESP32-C3    →    PCF8574
GPIO2 (SDA) →    SDA
GPIO4 (SCL) →    SCL
3.3V        →    VCC
GND         →    GND
```

### Schritt 2: Temperatursensor
```
ESP32-C3    →    DS18B20
GPIO3       →    Data (+ 4.7kΩ Pull-up zu 3.3V)
3.3V        →    VCC
GND         →    GND
```

### Schritt 3: Battery Monitor
```
Batterie+ → 100kΩ → GPIO6 (ESP32)
                 ↓
               10kΩ → GND
```

### Schritt 4: Rain Bird Interface
```
PCF8574 Pin 0 → Rain Bird Sensor Input+
ESP32 GND     → Rain Bird Sensor Input-
```

## Software Installation

### 1. Repository klonen
```bash
git clone https://github.com/raifdmueller/ESP32-Smart-Rain-Sensor.git
cd ESP32-Smart-Rain-Sensor
```

### 2. Konfiguration erstellen
```bash
cd src
cp config.h.example config.h
```

### 3. config.h bearbeiten
```cpp
#define WIFI_SSID "IhrWiFiName"
#define WIFI_PASSWORD "IhrWiFiPasswort"
#define OPENWEATHER_API_KEY "IhrAPIKey"
#define LATITUDE "50.1109"    // Ihre Koordinaten
#define LONGITUDE "8.6821"
```

### 4. OpenWeatherMap API Key besorgen
1. Account bei https://openweathermap.org erstellen
2. API Key generieren (kostenlos für 1000 Calls/Tag)
3. Koordinaten Ihres Standorts ermitteln

### 5. Code hochladen
1. Arduino IDE öffnen
2. `src/main.cpp` öffnen
3. Board und Port auswählen
4. Upload

## Rain Bird Controller Setup

### 1. Physische Verbindung
```
Im Rain Bird ESP-BAT-BT-6 Gehäuse:
- ESP32 Hardware einbauen
- Sensor-Anschlüsse identifizieren
- PCF8574 Pin 0 an SENSOR IN
- ESP32 GND an SENSOR OUT
```

### 2. Controller Konfiguration
```
Rain Bird 2.0 App:
- Rain Sensor: "Enabled"  
- Sensor Type: "Normally Closed"
- Test: Jumper zwischen Sensor-Anschlüsse
```

## Test und Verifikation

### 1. Serial Monitor Test
```
- Baud Rate: 115200
- Nach Reset sollten Meldungen erscheinen:
  "Hardware initialized"
  "Boot #1 - Wakeup reason: 0"
  "Battery: 6.00V (100%)"
  "Temperature: 22.5°C"
```

### 2. I2C Test
```cpp
// Test Code für PCF8574
pcf.digitalWrite(0, HIGH);  // LED sollte an Rain Bird aufleuchten
delay(1000);
pcf.digitalWrite(0, LOW);   // LED sollte ausgehen
```

### 3. Weather API Test
```
Bei erstem Boot sollte WiFi verbinden:
"WiFi connected"
"Weather data updated"
"Decision: No significant rain expected"
```

### 4. Rain Bird Integration Test
```
Rain Bird App → Manual Test → Station 1
- Sollte starten wenn ESP32 "trocken" signalisiert
- Sollte gestoppt werden wenn ESP32 "Regen" signalisiert
```

## Troubleshooting

### Problem: ESP32 startet nicht
```
Lösung:
- GPIO0 auf GND beim Upload
- EN Pin Reset-Taste drücken
- USB-Kabel und Treiber prüfen
```

### Problem: PCF8574 nicht gefunden
```
Lösung:
- I2C Verkabelung prüfen
- Pull-up Widerstände (4.7kΩ) an SDA/SCL
- I2C Scanner ausführen
```

### Problem: Temperatursensor DEVICE_DISCONNECTED_C
```
Lösung:
- DS18B20 Verkabelung prüfen
- 4.7kΩ Pull-up von Data zu 3.3V
- Sensor auf Defekt prüfen
```

### Problem: WiFi verbindet nicht
```
Lösung:
- SSID/Password in config.h prüfen
- 2.4GHz Netzwerk verwenden (nicht 5GHz)
- Signal-Stärke am Installationsort messen
```

### Problem: Rain Bird reagiert nicht
```
Lösung:
- Sensor-Anschlüsse im Controller identifizieren
- Jumper-Test: Draht zwischen Anschlüsse
- App-Einstellung: "Normally Closed"
- Multimeter: Kontinuität messen
```

### Problem: Kurze Batterielaufzeit
```
Lösung:
- Serial Monitor disabled für Produktion
- Debug-Optionen in config.h ausschalten
- Deep Sleep Funktion prüfen
- Battery Monitor kalibrieren
```

## Monitoring und Wartung

### 1. Battery Status
```cpp
// Im Serial Monitor:
"Battery: 5.8V (100%)"  // Frisch
"Battery: 5.0V (60%)"   // Gut  
"Battery: 4.0V (25%)"   // Niedrig
"Battery: 3.5V (0%)"    // Kritisch
```

### 2. Statistiken
```cpp
// Gespeichert in Preferences:
- Anzahl Boots
- Block/Allow Entscheidungen
- Letzte Temperatur/Batterie
- Weather Check Timestamps
```

### 3. Wartungsintervalle
```
Monatlich:
- Battery Status prüfen
- Temperatursensor reinigen
- Rain Bird App Status checken

Jährlich:
- Batterien tauschen (prophylaktisch)
- Gehäuse-Dichtung prüfen
- Verkabelung inspizieren
```

## Erweiterte Konfiguration

### Power Optimization
```cpp
// config.h anpassen:
#define DEEP_SLEEP_HOURS 12      // Längere Sleep-Zeit
#define WEATHER_CHECK_CYCLES 2   // Häufigere Weather-Checks
```

### Algorithmus Tuning
```cpp
// main.cpp - makeIrrigationDecision() anpassen:
// Schwellwerte für Niederschlag
// Temperatur-Overrides
// Luftfeuchtigkeits-Faktoren
```

### Debug Mode
```cpp
#define DEBUG_SERIAL true
#define DEBUG_WEATHER_API true
#define DEBUG_POWER_CONSUMPTION true
```

## Backup und Updates

### 1. Firmware Backup
```bash
# Firmware aus ESP32 auslesen
esptool.py --port COM3 --baud 921600 read_flash 0 0x400000 backup.bin
```

### 2. OTA Updates (zukünftig)
```
- Web-Interface für Updates
- GitHub Releases monitoring  
- Automatic update notifications
```

Das war's! Das System sollte jetzt einsatzbereit sein und 15+ Monate mit einer Batterieladung laufen.