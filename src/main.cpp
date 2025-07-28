/*
 * ESP32 Smart Rain Sensor
 * 
 * Intelligenter Regensensor für Rain Bird ESP-BAT-BT-6 Controller
 * Nutzt Wettervorhersagen statt nur aktuelle Niederschläge
 * 
 * Hardware:
 * - ESP32-C3 (Ultra-Low Power)
 * - PCF8574 I2C GPIO Expander
 * - DS18B20 Temperatursensor
 * - 4x AA Lithium Batterien
 * 
 * Batterielaufzeit: ~15 Monate
 * 
 * Author: [Your Name]
 * License: MIT
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PCF8574.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include "config.h"

// Hardware Pin Definitionen
#define I2C_SDA 2
#define I2C_SCL 4
#define TEMP_PIN 3
#define BATTERY_PIN 6
#define STATUS_LED 5

// PCF8574 Pins
#define RAIN_SENSOR_PIN 0

// Power Management
#define SLEEP_DURATION_SEC (6 * 3600)  // 6 Stunden
#define WEATHER_CHECK_INTERVAL (24)    // Alle 24 Zyklen = 1x täglich
#define BATTERY_LOW_THRESHOLD 4.0      // Volt
#define BATTERY_CRITICAL_THRESHOLD 3.5 // Volt

// Weather API
#define MAX_WEATHER_AGE (12 * 3600)    // 12 Stunden max Alter

// Globale Objekte
PCF8574 pcf(0x20);
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);
Preferences prefs;

// RTC Memory für Persistent Data
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR bool lastIrrigationDecision = false;
RTC_DATA_ATTR unsigned long lastWeatherCheck = 0;
RTC_DATA_ATTR float lastTemperature = 0.0;
RTC_DATA_ATTR bool weatherRainForecast = false;

// Struktur für Wetterdaten
struct WeatherData {
  float temperature;
  float humidity;
  float precipitation_1h;
  float precipitation_forecast_6h;
  float precipitation_forecast_24h;
  bool isRaining;
  unsigned long timestamp;
};

void setup() {
  Serial.begin(115200);
  
  // Hardware initialisieren
  initHardware();
  
  bootCount++;
  Serial.printf("Boot #%d - Wakeup reason: %d\n", bootCount, esp_sleep_get_wakeup_cause());
  
  // Batteriespannung prüfen
  float batteryVoltage = readBatteryVoltage();
  Serial.printf("Battery: %.2fV (%d%%)\n", batteryVoltage, getBatteryPercentage(batteryVoltage));
  
  // Kritische Batterie = Notfallmodus
  if (batteryVoltage < BATTERY_CRITICAL_THRESHOLD) {
    Serial.println("CRITICAL BATTERY - Emergency mode");
    emergencyMode();
    return;
  }
  
  // Temperatursensor lesen
  float temperature = readTemperature();
  lastTemperature = temperature;
  Serial.printf("Temperature: %.1f°C\n", temperature);
  
  // Wettercheck bei jedem n-ten Boot oder wenn Daten zu alt
  bool needWeatherUpdate = (bootCount % WEATHER_CHECK_INTERVAL == 0) || 
                          (millis() - lastWeatherCheck > MAX_WEATHER_AGE * 1000);
  
  if (needWeatherUpdate) {
    Serial.println("Performing weather check...");
    WeatherData weather = getWeatherData();
    updateWeatherDecision(weather);
  }
  
  // Bewässerungsentscheidung treffen
  bool shouldBlockIrrigation = makeIrrigationDecision(temperature, batteryVoltage);
  
  // Rain Bird Controller steuern
  controlRainSensor(shouldBlockIrrigation);
  
  // Status ausgeben
  Serial.printf("Irrigation %s (Weather: %s, Temp: %.1f°C)\n", 
                shouldBlockIrrigation ? "BLOCKED" : "ALLOWED",
                weatherRainForecast ? "Rain" : "Clear",
                temperature);
  
  // Statistiken speichern
  saveStatistics(temperature, batteryVoltage, shouldBlockIrrigation);
  
  // Power-optimiertes Deep Sleep
  enterDeepSleep(batteryVoltage);
}

void loop() {
  // Nie erreicht - ESP32 wacht nur aus Deep Sleep auf
}

void initHardware() {
  // I2C Bus initialisieren
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // PCF8574 initialisieren
  if (!pcf.begin()) {
    Serial.println("ERROR: PCF8574 not found!");
  }
  pcf.pinMode(RAIN_SENSOR_PIN, OUTPUT);
  
  // Temperatursensor initialisieren
  tempSensor.begin();
  tempSensor.setResolution(12);  // Höchste Auflösung
  
  // Preferences für persistente Speicherung
  prefs.begin("rain-sensor", false);
  
  // Status LED (optional)
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);
  
  Serial.println("Hardware initialized");
}

float readTemperature() {
  tempSensor.requestTemperatures();
  delay(750);  // Conversion time für 12-bit
  float temp = tempSensor.getTempCByIndex(0);
  
  if (temp == DEVICE_DISCONNECTED_C) {
    Serial.println("ERROR: Temperature sensor disconnected!");
    return lastTemperature;  // Fallback auf letzten Wert
  }
  
  return temp;
}

float readBatteryVoltage() {
  // ADC kalibrieren für bessere Genauigkeit
  int raw = 0;
  for (int i = 0; i < 10; i++) {
    raw += analogRead(BATTERY_PIN);
    delay(10);
  }
  raw /= 10;  // Mittelwert
  
  // Voltage Divider: 100kΩ + 10kΩ = 11:1 Teiler
  // 6V Batterie -> 0.55V am ADC bei 3.3V Referenz
  float voltage = (raw / 4095.0) * 3.3 * 11.0;
  
  return voltage;
}

int getBatteryPercentage(float voltage) {
  // Lithium AA Entladekurve approximiert
  if (voltage > 5.8) return 100;
  if (voltage > 5.4) return 80;
  if (voltage > 5.0) return 60;
  if (voltage > 4.6) return 40;
  if (voltage > 4.2) return 20;
  if (voltage > 3.8) return 10;
  return 0;
}

WeatherData getWeatherData() {
  WeatherData weather = {0};
  
  // WiFi verbinden mit Timeout
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
    Serial.print(".");
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection failed - using cached data");
    return loadCachedWeatherData();
  }
  
  Serial.println("\nWiFi connected");
  
  // OpenWeatherMap API Call
  HTTPClient http;
  String url = String("http://api.openweathermap.org/data/2.5/forecast?lat=") + 
               LATITUDE + "&lon=" + LONGITUDE + 
               "&appid=" + OPENWEATHER_API_KEY + "&units=metric";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    weather = parseWeatherData(payload);
    cacheWeatherData(weather);
    lastWeatherCheck = millis();
    Serial.println("Weather data updated");
  } else {
    Serial.printf("HTTP Error: %d\n", httpCode);
    weather = loadCachedWeatherData();
  }
  
  http.end();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  return weather;
}

WeatherData parseWeatherData(String json) {
  WeatherData weather = {0};
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.printf("JSON parsing failed: %s\n", error.c_str());
    return loadCachedWeatherData();
  }
  
  // Aktuelle Bedingungen (erste Vorhersage)
  auto current = doc["list"][0];
  weather.temperature = current["main"]["temp"];
  weather.humidity = current["main"]["humidity"];
  
  // Niederschlag der letzten Stunde
  if (current["rain"].containsKey("1h")) {
    weather.precipitation_1h = current["rain"]["1h"];
  }
  
  // Regen-Status
  String weatherMain = current["weather"][0]["main"];
  weather.isRaining = (weatherMain == "Rain" || weatherMain == "Drizzle");
  
  // Vorhersage für nächste 6 und 24 Stunden
  float rain_6h = 0, rain_24h = 0;
  int count_6h = 0, count_24h = 0;
  
  for (int i = 0; i < min(8, (int)doc["list"].size()); i++) {  // 8 x 3h = 24h
    auto forecast = doc["list"][i];
    float rain = 0;
    
    if (forecast["rain"].containsKey("3h")) {
      rain = forecast["rain"]["3h"];
    }
    
    if (i < 2) {  // 6 Stunden
      rain_6h += rain;
      count_6h++;
    }
    
    rain_24h += rain;
    count_24h++;
  }
  
  weather.precipitation_forecast_6h = rain_6h;
  weather.precipitation_forecast_24h = rain_24h;
  weather.timestamp = millis();
  
  return weather;
}

void cacheWeatherData(WeatherData weather) {
  prefs.putFloat("temp", weather.temperature);
  prefs.putFloat("humidity", weather.humidity);
  prefs.putFloat("rain_1h", weather.precipitation_1h);
  prefs.putFloat("rain_6h", weather.precipitation_forecast_6h);
  prefs.putFloat("rain_24h", weather.precipitation_forecast_24h);
  prefs.putBool("raining", weather.isRaining);
  prefs.putULong("timestamp", weather.timestamp);
}

WeatherData loadCachedWeatherData() {
  WeatherData weather = {0};
  
  weather.temperature = prefs.getFloat("temp", 20.0);
  weather.humidity = prefs.getFloat("humidity", 50.0);
  weather.precipitation_1h = prefs.getFloat("rain_1h", 0.0);
  weather.precipitation_forecast_6h = prefs.getFloat("rain_6h", 0.0);
  weather.precipitation_forecast_24h = prefs.getFloat("rain_24h", 0.0);
  weather.isRaining = prefs.getBool("raining", false);
  weather.timestamp = prefs.getULong("timestamp", 0);
  
  Serial.println("Using cached weather data");
  return weather;
}

void updateWeatherDecision(WeatherData weather) {
  // Intelligente Regenvorhersage-Logik
  weatherRainForecast = false;
  
  // 1. Aktuell regnet es
  if (weather.isRaining) {
    weatherRainForecast = true;
    Serial.println("Decision: Currently raining");
    return;
  }
  
  // 2. Viel Regen in letzter Stunde
  if (weather.precipitation_1h > 2.0) {
    weatherRainForecast = true;
    Serial.printf("Decision: Heavy recent rain (%.1fmm)\n", weather.precipitation_1h);
    return;
  }
  
  // 3. Regen in nächsten 6 Stunden erwartet
  if (weather.precipitation_forecast_6h > 3.0) {
    weatherRainForecast = true;
    Serial.printf("Decision: Rain forecast 6h (%.1fmm)\n", weather.precipitation_forecast_6h);
    return;
  }
  
  // 4. Leichter Regen in nächsten 24h bei hoher Luftfeuchtigkeit
  if (weather.precipitation_forecast_24h > 1.0 && weather.humidity > 80) {
    weatherRainForecast = true;
    Serial.printf("Decision: Light rain + high humidity (%.1fmm, %.0f%%)\n", 
                  weather.precipitation_forecast_24h, weather.humidity);
    return;
  }
  
  Serial.println("Decision: No significant rain expected");
}

bool makeIrrigationDecision(float temperature, float batteryVoltage) {
  // Low Battery = Conservative Mode (weniger blockieren)
  if (batteryVoltage < BATTERY_LOW_THRESHOLD) {
    Serial.println("Low battery - conservative mode");
    return weatherRainForecast && (lastTemperature < 15.0);  // Nur bei Regen + Kälte blockieren
  }
  
  // Standard-Entscheidungslogik
  bool shouldBlock = false;
  
  // 1. Wettervorhersage hat Vorrang
  if (weatherRainForecast) {
    shouldBlock = true;
  }
  
  // 2. Temperaturfaktor (bei sehr hohen Temperaturen weniger blockieren)
  if (temperature > 35.0) {
    shouldBlock = false;  // Hitzestress - Bewässerung wichtiger
    Serial.println("Override: High temperature - irrigation needed");
  }
  
  // 3. Bei sehr niedrigen Temperaturen immer blockieren
  if (temperature < 5.0) {
    shouldBlock = true;  // Frostgefahr - keine Bewässerung
    Serial.println("Override: Low temperature - frost protection");
  }
  
  return shouldBlock;
}

void controlRainSensor(bool blockIrrigation) {
  // PCF8574 Open-Drain Logik:
  // HIGH = Open (Rain detected) = Bewässerung blockiert
  // LOW = Closed (Dry) = Bewässerung erlaubt
  
  if (blockIrrigation) {
    pcf.digitalWrite(RAIN_SENSOR_PIN, HIGH);  // Kontakt öffnen = "Regen"
  } else {
    pcf.digitalWrite(RAIN_SENSOR_PIN, LOW);   // Kontakt schließen = "Trocken"
  }
  
  lastIrrigationDecision = blockIrrigation;
  
  // Status LED (falls vorhanden)
  digitalWrite(STATUS_LED, blockIrrigation ? HIGH : LOW);
}

void saveStatistics(float temperature, float batteryVoltage, bool irrigationBlocked) {
  // Einfache Statistiken für Monitoring
  prefs.putUInt("boots", bootCount);
  prefs.putFloat("last_temp", temperature);
  prefs.putFloat("last_battery", batteryVoltage);
  prefs.putBool("last_decision", irrigationBlocked);
  
  // Zähler für Entscheidungen
  int blockCount = prefs.getUInt("block_count", 0);
  int allowCount = prefs.getUInt("allow_count", 0);
  
  if (irrigationBlocked) {
    blockCount++;
  } else {
    allowCount++;
  }
  
  prefs.putUInt("block_count", blockCount);
  prefs.putUInt("allow_count", allowCount);
  
  Serial.printf("Stats: %d boots, %d blocked, %d allowed\n", bootCount, blockCount, allowCount);
}

void emergencyMode() {
  Serial.println("EMERGENCY MODE - Critical battery");
  
  // Bewässerung erlauben (sicherer Zustand)
  pcf.digitalWrite(RAIN_SENSOR_PIN, LOW);
  
  // Minimal funktionalität aufrechterhalten
  prefs.putBool("emergency", true);
  
  // 24h Sleep für maximale Batterieerhaltung
  esp_sleep_enable_timer_wakeup(24 * 3600 * 1000000ULL);  // 24 Stunden
  esp_deep_sleep_start();
}

void enterDeepSleep(float batteryVoltage) {
  Serial.println("Entering deep sleep...");
  
  // Sleep-Dauer basierend auf Batteriestatus
  uint64_t sleepTime = SLEEP_DURATION_SEC * 1000000ULL;  // Mikrosekunden
  
  if (batteryVoltage < BATTERY_LOW_THRESHOLD) {
    sleepTime *= 2;  // Doppelte Sleep-Zeit bei niedrigem Akku
    Serial.println("Extended sleep due to low battery");
  }
  
  // Saisonale Anpassung (optional)
  // Winter: Längere Sleep-Zeiten
  // Sommer: Kürzere Sleep-Zeiten
  
  // Timer Wakeup konfigurieren
  esp_sleep_enable_timer_wakeup(sleepTime);
  
  // Alle GPIO auf sicheren Zustand
  digitalWrite(STATUS_LED, LOW);
  
  Serial.printf("Sleep for %d hours\n", (int)(sleepTime / 3600000000ULL));
  Serial.flush();
  
  // Deep Sleep aktivieren
  esp_deep_sleep_start();
}