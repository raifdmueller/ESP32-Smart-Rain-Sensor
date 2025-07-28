# ESP32 Smart Rain Sensor

![ESP32](https://img.shields.io/badge/ESP32-C3-red.svg)
![Rain Bird](https://img.shields.io/badge/Rain%20Bird-BAT--BT--6-blue.svg)
![Battery Life](https://img.shields.io/badge/Battery%20Life-15%20Months-green.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

**🌍 Sprachen**: [English](README.md) | [Deutsch](README_DE.md)

> **⚠️ WARNUNG - EXPERIMENTELLES PROJEKT**  
> Dieses Projekt ist aktuell **UNGETESTET** und befindet sich in einer frühen Entwicklungsphase. Das Hardware-Design und die Software wurden noch nicht unter realen Bedingungen validiert. Nutzung auf eigene Gefahr - rechnen Sie mit möglichen Problemen. Wir empfehlen, auf das erste stabile Release zu warten oder beim Testen und Validieren zu helfen.

Ein intelligenter Regensensor für Rain Bird Bewässerungssteuerungen, der Wettervorhersagen aus dem Internet nutzt statt nur auf aktuelle Niederschläge zu reagieren.

## 🚨 Projekt-Status

- **Hardware Design**: ⚠️ Theoretisch - Nicht physisch getestet
- **Software**: ⚠️ Kompiliert aber nicht feldgetestet  
- **Rain Bird Integration**: ⚠️ Nicht mit echten Controllern verifiziert
- **Batterielaufzeit-Angaben**: ⚠️ Basieren auf Berechnungen, nicht gemessen
- **Weather API**: ⚠️ Basis-Implementierung, braucht Validierung

**Bitte helfen Sie beim Testen und Validieren, falls Sie dieses Projekt nachbauen!**

## 🌟 Features

- **Intelligente Wettervorhersage**: Nutzt OpenWeatherMap API für vorausschauende Bewässerungsentscheidungen
- **Ultra-Low Power**: 15 Monate Batterielaufzeit mit 4x AA Lithium-Batterien *(berechnet, nicht getestet)*
- **Rain Bird kompatibel**: Direkter Anschluss an ESP-BAT-BT-6 Controller *(ungetestet)*
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

**Batterielaufzeit**: 2900mAh ÷ 6.43mAh/Tag = **451 Tage (15 Monate)** ⚠️ *Theoretische Berechnung*

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

> **⚠️ VORSICHT**: Diese Installationsanleitung ist theoretisch und ungetestet. Gehen Sie mit äußerster Vorsicht vor und rechnen Sie mit möglichen Problemen.

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

⚠️ **Testen Sie gründlich, bevor Sie das System an Ihr Bewässerungssystem anschließen!**

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

### Phase 1: MVP & Testing (Aktuell)
- [x] Basis Weather API Integration
- [x] PCF8574 Regensensor-Simulation
- [x] Deep Sleep Power Management
- [x] Temperatur-Monitoring
- [ ] **Hardware-Prototyping und Testen**
- [ ] **Rain Bird Integration Validierung**
- [ ] **Feldtest und Validierung**

### Phase 2: Stabiles Release
- [ ] Bewiesenes Hardware-Design
- [ ] Validierte Batterielaufzeit-Messungen
- [ ] Umfassende Test-Dokumentation
- [ ] Solar-Charging Option
- [ ] Webserver für Remote-Monitoring

### Phase 3: Erweiterte Features
- [ ] Home Assistant Integration
- [ ] MQTT Support
- [ ] Mobile App
- [ ] Cloud Data Logging
- [ ] Predictive Maintenance

## 🧪 Helfen Sie uns beim Testen!

**Wir brauchen Ihre Hilfe, um dieses Projekt zuverlässig zu machen!** Falls Sie sich entscheiden, dieses Projekt nachzubauen:

1. **Dokumentieren Sie alles** - Teilen Sie Ihren Bauprozess, Probleme und Lösungen
2. **Testen Sie gründlich** - Validieren Sie Stromverbrauch, Rain Bird Integration, Wetter-Genauigkeit
3. **Berichten Sie zurück** - Erstellen Sie GitHub Issues mit Ihren Erkenntnissen
4. **Teilen Sie Verbesserungen** - Pull Requests mit Fixes und Verbesserungen sind willkommen

### Was wir validieren müssen
- [ ] Echte Stromverbrauchs-Messungen
- [ ] Rain Bird Controller Kompatibilität
- [ ] PCF8574 Interface Zuverlässigkeit  
- [ ] Weather API Genauigkeit und Entscheidungslogik
- [ ] Langzeit-Stabilität und Batterielaufzeit
- [ ] Umwelt-Haltbarkeit

## 📊 Batterie-Optimierung

### Berechnete Batterielaufzeit ⚠️ *Unverified*
```
4x AA Lithium (2900mAh pro Batterie):
- Normal-Betrieb: 451 Tage (theoretisch)
- Winter-Modus (12h Zyklen): 680+ Tage (theoretisch)
- Mit Solar-Backup: Unbegrenzt

8x AA Konfiguration:
- Doppelte Kapazität: 30+ Monate (theoretisch)
- Extreme Wetter-Resistenz
```

### Stromspar-Features
- Nur Timer-basiertes Aufwachen
- WiFi im Deep Sleep deaktiviert
- Gecachte Wetterdaten
- Bedingte Sensor-Abfragen
- Batteriespannungs-Monitoring

## 🤝 Contributing

**Beiträge sind dringend benötigt!** Dieses Projekt braucht Testen und Validierung.

Prioritäts-Beiträge:
- **Hardware-Testen und Validierung**
- **Rain Bird Integration Testen**
- **Stromverbrauchs-Messungen**
- **Feldtest-Berichte**
- **Bug-Fixes und Verbesserungen**

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

**Status**: 🚧 **EXPERIMENTELL - UNGETESTET** | **Hardware**: Theoretisch | **Software**: Alpha | **Mitwirkende benötigt**: Ja!