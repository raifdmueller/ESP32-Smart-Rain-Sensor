# ESP32 Smart Rain Sensor

![ESP32](https://img.shields.io/badge/ESP32-C3-red.svg)
![Rain Bird](https://img.shields.io/badge/Rain%20Bird-BAT--BT--6-blue.svg)
![Battery Life](https://img.shields.io/badge/Battery%20Life-15%20Months-green.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

**🌍 Languages**: [English](README.md) | [Deutsch](README_DE.md)

> **⚠️ WARNING - EXPERIMENTAL PROJECT**  
> This project is currently **UNTESTED** and in early development phase. The hardware design and software have not been validated in real-world conditions. Use at your own risk and expect potential issues. We recommend waiting for the first stable release or helping with testing and validation.

An intelligent rain sensor for Rain Bird irrigation controllers that uses weather forecasts from the internet instead of just reacting to current precipitation.

## 🚨 Project Status

- **Hardware Design**: ⚠️ Theoretical - Not physically tested
- **Software**: ⚠️ Compiled but not field-tested  
- **Rain Bird Integration**: ⚠️ Not verified with actual controllers
- **Battery Life Claims**: ⚠️ Based on calculations, not measured
- **Weather API**: ⚠️ Basic implementation, needs validation

**Please contribute to testing and validation if you build this project!**

## 🌟 Features

- **Intelligent Weather Forecasting**: Uses OpenWeatherMap API for predictive irrigation decisions
- **Ultra-Low Power**: 15-month battery life with 4x AA lithium batteries *(calculated, not tested)*
- **Rain Bird Compatible**: Direct connection to ESP-BAT-BT-6 controller *(untested)*
- **Temperature Monitoring**: DS18B20 sensor for advanced algorithms
- **PCF8574 Interface**: Proven I2C expander technology
- **Easy Installation**: Completely housed within Rain Bird enclosure

## 🏗️ Hardware Design

### Components
- **ESP32-C3**: Ultra-low power microcontroller (5µA Deep Sleep)
- **PCF8574**: I2C GPIO expander for isolated switch simulation
- **DS18B20**: Waterproof temperature sensor
- **4x AA Lithium**: Primary batteries for maximum runtime

### Schematic
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

## ⚡ Power Consumption

| Component | Deep Sleep | Active | Measurement Time |
|-----------|------------|--------|------------------|
| ESP32-C3 | 5µA | 120mA | 30s |
| PCF8574 | 2.5µA | 2.5µA | - |
| DS18B20 | 1µA | 1.5mA | 1s |
| **Total** | **18.5µA** | **124mA** | - |

**Battery Life**: 2900mAh ÷ 6.43mAh/day = **451 days (15 months)** ⚠️ *Theoretical calculation*

## 🔧 Software Features

### Power Management
- **Deep Sleep**: 6-hour cycles with timer wakeup
- **Adaptive Intervals**: Winter mode (12h), summer mode (4h)
- **Battery Monitoring**: Voltage divider with ADC measurement
- **Low-Power Mode**: Reduced functionality below 20% battery

### Weather Intelligence
- **OpenWeatherMap API**: Hourly forecasts for 48h
- **Smart Algorithms**: Considers past and future precipitation
- **Caching**: Offline functionality during connection issues
- **Fail-Safe**: Fallback to local sensor data

### Rain Bird Interface
- **PCF8574 Control**: Isolated switch simulation
- **Normally Closed Logic**: Compatible with standard Rain Bird sensors
- **Isolation**: Galvanic separation via I2C
- **Status Monitoring**: Continuous function monitoring

## 📦 Installation

> **⚠️ CAUTION**: This installation guide is theoretical and untested. Proceed with extreme caution and expect potential issues.

### Prerequisites
1. Install Rain Bird ESP-BAT-BT-6 controller
2. Install ESP32 hardware in Rain Bird enclosure
3. Identify sensor connections (SENSOR IN/OUT)

### Wiring
```cpp
// PCF8574 to Rain Bird sensor connections
PCF8574 Pin 0  →  SENSOR IN
ESP32 GND      →  SENSOR OUT

// I2C connections
ESP32 GPIO2 (SDA)  →  PCF8574 SDA
ESP32 GPIO4 (SCL)  →  PCF8574 SCL

// Temperature sensor
ESP32 GPIO3  →  DS18B20 Data (+ 4.7kΩ Pullup)
```

### Software Setup
1. Install Arduino IDE with ESP32 board package
2. Install required libraries:
   - `PCF8574` by Renzo Mischianti
   - `OneWire` and `DallasTemperature`
   - `ArduinoJson` for API calls
3. Create `config.h` with WiFi credentials and API keys
4. Compile and flash firmware

⚠️ **Test thoroughly before connecting to your irrigation system!**

## 🌐 Weather API Integration

### OpenWeatherMap Setup
1. Create account at OpenWeatherMap
2. Generate API key (1000 calls/day free)
3. Use One Call API 3.0 for forecast data

### Example API Response
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

### Phase 1: MVP & Testing (Current)
- [x] Basic weather API integration
- [x] PCF8574 rain sensor simulation
- [x] Deep sleep power management
- [x] Temperature monitoring
- [ ] **Hardware prototyping and testing**
- [ ] **Rain Bird integration validation**
- [ ] **Field testing and validation**

### Phase 2: Stable Release
- [ ] Proven hardware design
- [ ] Validated battery life measurements
- [ ] Comprehensive testing documentation
- [ ] Solar charging option
- [ ] Webserver for remote monitoring

### Phase 3: Advanced Features
- [ ] Home Assistant integration
- [ ] MQTT support
- [ ] Mobile app
- [ ] Cloud data logging
- [ ] Predictive maintenance

## 🧪 Help Us Test!

**We need your help to make this project reliable!** If you decide to build this project:

1. **Document everything** - Share your build process, issues, and solutions
2. **Test thoroughly** - Validate power consumption, Rain Bird integration, weather accuracy
3. **Report back** - Create GitHub issues with your findings
4. **Share improvements** - Pull requests with fixes and enhancements welcome

### What We Need to Validate
- [ ] Actual power consumption measurements
- [ ] Rain Bird controller compatibility
- [ ] PCF8574 interface reliability  
- [ ] Weather API accuracy and decision logic
- [ ] Long-term stability and battery life
- [ ] Environmental durability

## 📊 Battery Optimization

### Calculated Battery Life ⚠️ *Unverified*
```
4x AA Lithium (2900mAh each):
- Normal operation: 451 days (theoretical)
- Winter mode (12h cycles): 680+ days (theoretical)
- With solar backup: Unlimited

8x AA Configuration:
- Double capacity: 30+ months (theoretical)
- Extreme weather resilience
```

### Power Saving Features
- Timer-based wake-up only
- WiFi disabled in deep sleep
- Cached weather data
- Conditional sensor readings
- Battery voltage monitoring

## 🤝 Contributing

**Contributions are urgently needed!** This project requires testing and validation.

Priority contributions:
- **Hardware testing and validation**
- **Rain Bird integration testing**
- **Power consumption measurements**
- **Field testing reports**
- **Bug fixes and improvements**

### Development Setup
```bash
git clone https://github.com/raifdmueller/ESP32-Smart-Rain-Sensor.git
cd ESP32-Smart-Rain-Sensor
# Open Arduino IDE and load project
```

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

## 🙏 Acknowledgments

- Rain Bird for excellent irrigation controllers
- Espressif for the ESP32 platform
- OpenWeatherMap for weather API
- Arduino community for libraries

---

**Status**: 🚧 **EXPERIMENTAL - UNTESTED** | **Hardware**: Theoretical | **Software**: Alpha | **Contributors Needed**: Yes!