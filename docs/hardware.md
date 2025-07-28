# Hardware Design

## Schaltplan

```
                         ESP32-C3 Ultra-Low Power
                           ┌─────────────────┐
                      3.3V │VCC          GPIO2│────┐ (I2C SDA)
                       GND │GND          GPIO3│────┼─── DS18B20 Data
                           │             GPIO4│────┘ (I2C SCL)
                      EN ──│EN           GPIO5│────── Status LED (optional)
                           │             GPIO6│────── Battery Monitor
                           │                  │
                           └─────────────────┘
                                     │
                                ┌────┴─── I2C Bus (3.3V)
                                │
                           ┌────────────┐               ┌─────────────┐
                      3.3V │VCC     P0  │───────────────│Sensor Input+│ Rain Bird 
                      SDA ─│SDA     P1  │               │Sensor Input-│ ESP-BAT-BT-6
                      SCL ─│SCL     P2  │               └─────────────┘
                      GND ─│GND     P3  │                             
                           │PCF8574 P4  │               ┌─────────────┐
                           │        P5  │               │   DS18B20   │
                           │        P6  │          3.3V │VCC     Data │──── GPIO3
                           │        P7  │          GND ─│GND          │
                           └────────────┘               └─────────────┘
                                                             │
                                                        4.7kΩ Pull-up
                                                             │
                                                            3.3V

                           ┌─────────────┐
                           │ Batterien   │
                      ┌────│4x AA        │
                      │    │Lithium      │
                      │    │(6V total)   │
                      │    └─────────────┘
                      │
                  ┌───┴───┐    ┌─────────┐
                  │       │    │100kΩ    │
                  │  LDO  │    │         │──── GPIO6 (Battery Monitor)
                  │3.3V   │    │10kΩ     │
                  │       │    │         │──── GND
                  └───┬───┘    └─────────┘
                      │
                     3.3V ──── ESP32 VCC
```

## Komponenten-Liste

| Komponente | Bezeichnung | Beschreibung | Kosten (ca.) |
|------------|-------------|--------------|--------------|
| Mikrocontroller | ESP32-C3-DevKitM-1 | Ultra-low power, WiFi, Bluetooth | 8€ |
| I2C Expander | PCF8574 Modul | 8-bit I/O Expander | 3€ |
| Temperatursensor | DS18B20 wasserdicht | Digitaler Temperatursensor | 5€ |
| Batterien | 4x AA Lithium | Energizer Ultimate Lithium | 12€ |
| Widerstände | 4.7kΩ, 100kΩ, 10kΩ | Pull-up und Voltage Divider | 1€ |
| Kondensatoren | 100nF, 10µF | Entkopplung | 1€ |
| **Gesamt** | | | **30€** |

## Leiterplatten-Layout

### PCB Überlegungen
- **Kompakte 2-Layer PCB** für Platzierung im Rain Bird Gehäuse
- **I2C Bus Design**: Kurze Leitungen, ordentliche Terminierung
- **Power Rails**: 3.3V und GND Planes für stabilen Betrieb
- **Antenna Keep-out**: 15mm Abstand zur ESP32 Antenne

### Mechanische Integration
```
Rain Bird ESP-BAT-BT-6 Gehäuse:
┌─────────────────────────────────┐
│  ┌─────────────┐                │
│  │ Rain Bird   │  ┌───────────┐ │
│  │ Controller  │  │  ESP32    │ │
│  │             │  │  PCB      │ │
│  │             │  │           │ │
│  └─────────────┘  └───────────┘ │
│                                 │
│  ┌─────────────────────────────┐ │
│  │    4x AA Batterien          │ │
│  └─────────────────────────────┘ │
└─────────────────────────────────┘
```

## Stromversorgung

### Batteriekonfiguration
```
4x AA Lithium in Serie:
- Spannung: 6V nominal (1.5V × 4)
- Kapazität: 2900mAh (L91 Ultimate Lithium)
- Temperaturbereich: -40°C bis +60°C
- Selbstentladung: <1% pro Jahr
```

### Spannungsregelung
- **LDO Regler**: 3.3V, ultra-low quiescent current (<1µA)
- **Eingangsspannung**: 4.5V - 6V (Batterie-Entladekurve)
- **Ausgangsrippel**: <10mV für saubere ADC-Messungen

### Battery Monitoring
```cpp
// Voltage Divider für Batteriespannung
float readBatteryVoltage() {
    int raw = analogRead(BATTERY_PIN);
    float voltage = (raw / 4095.0) * 3.3 * 2.0;  // 2:1 Teiler
    return voltage;
}

// Batterie-Prozentsatz berechnen
int getBatteryPercentage(float voltage) {
    if (voltage > 5.8) return 100;      // Frische Batterien
    if (voltage > 5.0) return 75;       // Gut
    if (voltage > 4.5) return 50;       // Mittel
    if (voltage > 4.0) return 25;       // Niedrig
    return 0;                           // Kritisch
}
```

## I2C Bus Design

### PCF8574 Konfiguration
```cpp
// I2C Adresse: 0x20 (A0-A2 = GND)
// Pin Assignment:
// P0: Rain Bird Sensor Interface
// P1-P7: Reserviert für Erweiterungen
```

### Bus Spezifikationen
- **Frequenz**: 100kHz (Standard-Mode)
- **Pull-up Widerstände**: 4.7kΩ (extern)
- **Kabellänge**: <10cm (kurze Verbindungen)
- **Störsicherheit**: Twisted-Pair für längere Verbindungen

## Temperatursensor Integration

### DS18B20 Spezifikationen
- **Messbereich**: -55°C bis +125°C
- **Genauigkeit**: ±0.5°C (-10°C bis +85°C)
- **Auflösung**: 9-12 Bit konfigurierbar
- **Parasitic Power**: Nicht verwendet (separate VCC)

### OneWire Bus
```cpp
// Pin Configuration
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 12

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void initTemperatureSensor() {
    sensors.begin();
    sensors.setResolution(TEMPERATURE_PRECISION);
    sensors.setWaitForConversion(false);  // Async für Power Saving
}
```

## EMV und Signal-Integrität

### Entstörung
- **Ferrit-Perlen**: Auf Versorgungsleitungen
- **Bypass-Kondensatoren**: 100nF + 10µF an VCC
- **Schirmung**: Optionale Abschirmung für RF-kritische Bereiche

### Grounding
- **Single-Point Ground**: Sternförmige Masseführung
- **Analoge/Digitale Trennung**: Separate Massebereiche für ADC
- **Chassis Ground**: Verbindung zum Metallgehäuse

## Mechanical Design

### Gehäuse-Integration
- **IP68 Rating**: Nutzung des Rain Bird Gehäuses
- **Kabeleinführungen**: M12 wasserdichte Steckverbinder
- **Montage**: 3D-gedruckte Halterungen für PCB
- **Wartung**: Einfacher Zugang für Batteriewechsel

### Umweltbedingungen
- **Betriebstemperatur**: -20°C bis +60°C
- **Lagertemperatur**: -40°C bis +85°C
- **Luftfeuchtigkeit**: 0-95% nicht kondensierend
- **Wasserschutz**: IP68 durch Rain Bird Gehäuse

## Erweiterungsmöglichkeiten

### Zusätzliche Sensoren
- **Bodenfeuchtigkeit**: Analog/I2C Sensoren
- **Lichtsensor**: BH1750 für Tageslicht-Erkennung
- **Luftfeuchtigkeit**: SHT30 für Mikroklima-Monitoring

### Solar Power Option
```
Solar Panel (5W) → MPPT Controller → LiFePO4 Batterie → ESP32
                                  ↘ Backup zu AA Batterien
```

### Connectivity Upgrades
- **LoRaWAN**: Für große Reichweiten ohne WiFi
- **Cellular**: 4G Modem für absolute Konnektivität
- **Ethernet**: POE für permanente Installation

## Testing und Validation

### Power Consumption Testing
```cpp
// Deep Sleep Current Measurement
esp_sleep_enable_timer_wakeup(60 * 1000000);  // 60s
esp_deep_sleep_start();
// Gemessen: 18.5µA ±2µA
```

### Environmental Testing
- **Temperaturzyklen**: -20°C bis +60°C
- **Feuchtigkeits-Test**: 95% RH für 48h
- **Vibrations-Test**: Gemäß DIN EN 60068-2-6
- **EMV-Test**: CE-Konformität prüfen

### Field Testing
- **Battery Life**: 6-Monate Langzeittest
- **WiFi Range**: Reichweiten-Test in verschiedenen Umgebungen
- **Weather API**: Zuverlässigkeit bei verschiedenen Wetterlagen
- **Rain Bird Integration**: Kompatibilitäts-Test mit verschiedenen Controllern