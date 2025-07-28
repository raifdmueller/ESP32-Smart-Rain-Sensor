# Hardware Design

**🌍 Languages**: [English](docs/hardware.md) | [Deutsch](docs/hardware_DE.md)

## Circuit Diagram

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
                           │ Batteries   │
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

## Bill of Materials

| Component | Part Number | Description | Cost (approx.) |
|-----------|-------------|-------------|----------------|
| Microcontroller | ESP32-C3-DevKitM-1 | Ultra-low power, WiFi, Bluetooth | $8 |
| I2C Expander | PCF8574 Module | 8-bit I/O Expander | $3 |
| Temperature Sensor | DS18B20 Waterproof | Digital Temperature Sensor | $5 |
| Batteries | 4x AA Lithium | Energizer Ultimate Lithium | $12 |
| Resistors | 4.7kΩ, 100kΩ, 10kΩ | Pull-up and Voltage Divider | $1 |
| Capacitors | 100nF, 10µF | Decoupling | $1 |
| **Total** | | | **$30** |

## PCB Layout Considerations

### PCB Design
- **Compact 2-Layer PCB** for placement in Rain Bird enclosure
- **I2C Bus Design**: Short traces, proper termination
- **Power Rails**: 3.3V and GND planes for stable operation
- **Antenna Keep-out**: 15mm clearance from ESP32 antenna

### Mechanical Integration
```
Rain Bird ESP-BAT-BT-6 Enclosure:
┌─────────────────────────────────┐
│  ┌─────────────┐                │
│  │ Rain Bird   │  ┌───────────┐ │
│  │ Controller  │  │  ESP32    │ │
│  │             │  │  PCB      │ │
│  │             │  │           │ │
│  └─────────────┘  └───────────┘ │
│                                 │
│  ┌─────────────────────────────┐ │
│  │    4x AA Batteries          │ │
│  └─────────────────────────────┘ │
└─────────────────────────────────┘
```

## Power Supply Design

### Battery Configuration
```
4x AA Lithium in series:
- Voltage: 6V nominal (1.5V × 4)
- Capacity: 2900mAh (L91 Ultimate Lithium)
- Temperature range: -40°C to +60°C
- Self-discharge: <1% per year
```

### Voltage Regulation
- **LDO Regulator**: 3.3V, ultra-low quiescent current (<1µA)
- **Input Voltage**: 4.5V - 6V (battery discharge curve)
- **Output Ripple**: <10mV for clean ADC measurements

### Battery Monitoring
```cpp
// Voltage divider for battery voltage
float readBatteryVoltage() {
    int raw = analogRead(BATTERY_PIN);
    float voltage = (raw / 4095.0) * 3.3 * 2.0;  // 2:1 divider
    return voltage;
}

// Calculate battery percentage
int getBatteryPercentage(float voltage) {
    if (voltage > 5.8) return 100;      // Fresh batteries
    if (voltage > 5.0) return 75;       // Good
    if (voltage > 4.5) return 50;       // Medium
    if (voltage > 4.0) return 25;       // Low
    return 0;                           // Critical
}
```

## I2C Bus Design

### PCF8574 Configuration
```cpp
// I2C Address: 0x20 (A0-A2 = GND)
// Pin Assignment:
// P0: Rain Bird Sensor Interface
// P1-P7: Reserved for expansions
```

### Bus Specifications
- **Frequency**: 100kHz (Standard-Mode)
- **Pull-up Resistors**: 4.7kΩ (external)
- **Cable Length**: <10cm (short connections)
- **Noise Immunity**: Twisted-pair for longer connections

## Temperature Sensor Integration

### DS18B20 Specifications
- **Measurement Range**: -55°C to +125°C
- **Accuracy**: ±0.5°C (-10°C to +85°C)
- **Resolution**: 9-12 bit configurable
- **Parasitic Power**: Not used (separate VCC)

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
    sensors.setWaitForConversion(false);  // Async for power saving
}
```

## EMC and Signal Integrity

### Noise Suppression
- **Ferrite Beads**: On power supply lines
- **Bypass Capacitors**: 100nF + 10µF at VCC
- **Shielding**: Optional shielding for RF-critical areas

### Grounding
- **Single-Point Ground**: Star-shaped ground routing
- **Analog/Digital Separation**: Separate ground areas for ADC
- **Chassis Ground**: Connection to metal enclosure

## Mechanical Design

### Enclosure Integration
- **IP68 Rating**: Utilizing Rain Bird enclosure
- **Cable Entries**: M12 waterproof connectors
- **Mounting**: 3D-printed brackets for PCB
- **Maintenance**: Easy access for battery replacement

### Environmental Conditions
- **Operating Temperature**: -20°C to +60°C
- **Storage Temperature**: -40°C to +85°C
- **Humidity**: 0-95% non-condensing
- **Water Protection**: IP68 through Rain Bird enclosure

## Expansion Options

### Additional Sensors
- **Soil Moisture**: Analog/I2C sensors
- **Light Sensor**: BH1750 for daylight detection
- **Humidity**: SHT30 for microclimate monitoring

### Solar Power Option
```
Solar Panel (5W) → MPPT Controller → LiFePO4 Battery → ESP32
                                  ↘ Backup to AA Batteries
```

### Connectivity Upgrades
- **LoRaWAN**: For long-range without WiFi
- **Cellular**: 4G modem for absolute connectivity
- **Ethernet**: POE for permanent installation

## Testing and Validation

### Power Consumption Testing
```cpp
// Deep Sleep Current Measurement
esp_sleep_enable_timer_wakeup(60 * 1000000);  // 60s
esp_deep_sleep_start();
// Measured: 18.5µA ±2µA
```

### Environmental Testing
- **Temperature Cycles**: -20°C to +60°C
- **Humidity Test**: 95% RH for 48h
- **Vibration Test**: According to DIN EN 60068-2-6
- **EMC Test**: CE compliance verification

### Field Testing
- **Battery Life**: 6-month long-term test
- **WiFi Range**: Range testing in various environments
- **Weather API**: Reliability in different weather conditions
- **Rain Bird Integration**: Compatibility test with various controllers