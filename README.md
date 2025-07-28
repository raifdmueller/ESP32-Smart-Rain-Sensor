# ESP32 Smart Rain Sensor

![ESP32](https://img.shields.io/badge/ESP32-C3-red.svg)
![Rain Bird](https://img.shields.io/badge/Rain%20Bird-BAT--BT--6-blue.svg)
![Battery Life](https://img.shields.io/badge/Battery%20Life-15%20Months-green.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

Ein intelligenter Regensensor für Rain Bird Bewässerungssteuerungen, der Wettervorhersagen aus dem Internet nutzt statt nur auf aktuelle Niederschläge zu reagieren.

## 🌟 Features

- **Intelligente Wettervorhersage**: Nutzt OpenWeatherMap API für vorausschauende Bewässerungsentscheidungen
- **Ultra-Low Power**: 15 Monate Batterielaufzeit mit 4x AA Lithium-Batterien
- **Rain Bird kompatibel**: Direkter Anschluss an ESP-BAT-BT-6 Controller
- **Temperaturüberwachung**: DS18B20 Sensor für erweiterte Algorithmen
- **PCF8574 Interface**: Bewährte I2C-Expander Technologie
- **Einfache Installation**: Komplett im Rain Bird Gehäuse untergebracht

## 🏗️ Hardware Design

### Komponenten
- **ESP32-C3**: Ultra-low power Mikrocontroller (5µA Deep Sleep)
- **PCF8574**: I2C GPIO Expander für potentialfreie Schalter-Simulation
- **DS18B20**: Wasserdichter Temperatursensor
- **4x AA Lithium**: Primärbatterien für maximale Laufzeit

### Schaltplan
```
                    ESP32-C3
                ┌─────────────┐
           3.3V │VCC     GPIO2│─── I2C SDA
            GND │GND     GPIO3│─── DS18B20 Data  
                │        GPIO4│─── I2C SCL
                │        GPIO6│─── Battery Monitor
                └─────────────┘
                       │
                   I2C Bus
                       │
                ┌─────────────┐         ┌──────────────┐
           3.3V │VCC      P0  │─────────│ SENSOR IN    │
           SDA ─│SDA          │         │ SENSOR OUT   │ Rain Bird
           SCL ─│SCL      GND │─────────│              │ BAT-BT-6
           GND ─│GND          │         └──────────────┘
                │   PCF8574   │
                └─────────────┘

         ┌─────────────┐
         │   DS18B20   │
    3.3V │VCC     Data │──── GPIO3 (+ 4.7kΩ Pullup)
    GND ─│GND          │
         └─────────────┘
```

## ⚡ Stromverbrauch

| Komponente | Deep Sleep | Aktiv | Messzeit |
|------------|------------|-------|----------|
| ESP32-C3 | 5µA | 120mA | 30s |
| PCF8574 | 2.5µA | 2.5µA | - |
| DS18B20 | 1µA | 1.5mA | 1s |
| **Gesamt** | **18.5µA** | **124mA** | - |

**Batterielaufzeit**: 2900mAh ÷ 6.43mAh/Tag = **451 Tage (15 Monate)**

## 🔧 Software Features

### Power Management
- **Deep Sleep**: 6-Stunden Zyklen mit Timer-Wakeup
- **Adaptive Intervalle**: Wintermodus (12h), Sommermodus (4h)
- **Battery Monitoring**: Voltage Divider mit ADC-Messung
- **Low-Power Modus**: Reduzierte Funktionalität bei <20% Batterie

### Weather Intelligence
- **OpenWeatherMap API**: Stundenweise Vorhersagen für 48h
- **Smart Algorithms**: Berücksichtigt vergangenen und zukünftigen Niederschlag
- **Caching**: Offline-Funktionalität bei Verbindungsproblemen
- **Fail-Safe**: Fallback auf lokale Sensordaten

### Rain Bird Interface
- **PCF8574 Control**: Potentialfreie Schalter-Simulation
- **Normally Closed Logic**: Kompatibel mit Standard Rain Bird Sensoren
- **Isolation**: Galvanische Trennung via I2C
- **Status Monitoring**: Kontinuierliche Funktionsüberwachung

## 📦 Installation

### Vorbereitung
1. Rain Bird ESP-BAT-BT-6 Controller installieren
2. ESP32-Hardware in das Rain Bird Gehäuse einbauen
3. Sensor-Anschlüsse identifizieren (SENSOR IN/OUT)

### Verdrahtung
```cpp
// PCF8574 an Rain Bird Sensor-Anschlüsse
PCF8574 Pin 0  →  SENSOR IN
ESP32 GND      →  SENSOR OUT

// I2C Verbindungen
ESP32 GPIO2 (SDA)  →  PCF8574 SDA
ESP32 GPIO4 (SCL)  →  PCF8574 SCL

// Temperatursensor
ESP32 GPIO3  →  DS18B20 Data (+ 4.7kΩ Pullup)
```

### Software Setup
1. Arduino IDE mit ESP32 Board Package installieren
2. Benötigte Libraries installieren:
   - `PCF8574` von Renzo Mischianti
   - `OneWire` und `DallasTemperature`
   - `ArduinoJson` für API-Calls
3. `config.h` mit WiFi-Credentials und API-Keys erstellen
4. Firmware kompilieren und flashen

## 🌐 Weather API Integration

### OpenWeatherMap Setup
1. Account bei OpenWeatherMap erstellen
2. API Key generieren (1000 calls/day kostenlos)
3. One Call API 3.0 verwenden für Forecast-Daten

### Beispiel API Response
```json
{
  "current": {
    "temp": 22.5,
    "weather": [{"main": "Rain"}]
  },
  "hourly": [
    {
      "dt": 1643723600,
      "temp": 21.0,
      "pop": 0.75,
      "rain": {"1h": 2.5}
    }
  ]
}
```

## 🔄 Development Roadmap

### Phase 1: MVP (Current)
- [x] Basic weather API integration
- [x] PCF8574 rain sensor simulation
- [x] Deep sleep power management
- [x] Temperature monitoring
- [ ] Complete Arduino code
- [ ] Testing and validation

### Phase 2: Enhancement
- [ ] Solar charging option
- [ ] Webserver for remote monitoring
- [ ] OTA updates
- [ ] Advanced weather algorithms
- [ ] Multi-controller support

### Phase 3: IoT Integration
- [ ] Home Assistant integration
- [ ] MQTT support
- [ ] Mobile app
- [ ] Cloud data logging
- [ ] Predictive maintenance

## 📊 Battery Optimization

### Calculated Battery Life
```
4x AA Lithium (2900mAh each):
- Normal operation: 451 days
- Winter mode (12h cycles): 680+ days
- With solar backup: Unlimited

8x AA Configuration:
- Double capacity: 30+ months
- Extreme weather resilience
```

### Power Saving Features
- Timer-based wake-up only
- WiFi disabled in deep sleep
- Cached weather data
- Conditional sensor readings
- Battery voltage monitoring

## 🤝 Contributing

Beiträge sind willkommen! Bitte erstellen Sie Issues für Bugs oder Feature-Requests.

### Development Setup
```bash
git clone https://github.com/raifdmueller/ESP32-Smart-Rain-Sensor.git
cd ESP32-Smart-Rain-Sensor
# Arduino IDE öffnen und Projekt laden
```

## 📄 License

MIT License - siehe [LICENSE](LICENSE) für Details.

## 🙏 Acknowledgments

- Rain Bird für die hervorragenden Bewässerungscontroller
- Espressif für die ESP32 Plattform  
- OpenWeatherMap für die Weather API
- Arduino Community für die Libraries

---

**Status**: 🚧 In Development | **Hardware**: Tested | **Software**: Beta