// Wind
// Gelb: 3V3
// Rot: Ground
// Grün: G35 (dir)
// schwarz: G32 (speed)

// Temperatur und Hum
// Grün: D0
// Schwarz: GND
// Rot: 5V

// Regen
// Pin: 23

#include <Arduino.h>

#include "DHT.h"
#include "mqtt.hpp"
#include "secrets.hpp"

double totalRain = 0;
double timeSinceLastWindTick = 5000;
unsigned long lastRainTick = millis();
unsigned long lastWindSpeedTick = millis();

DHT dht(0, DHT11);

void IRAM_ATTR rain_isr() {
    if (millis() - lastRainTick > 500) {

        totalRain += C_RAIN_TICK_TO_MM;
        lastRainTick = millis();
    }
}
void IRAM_ATTR wind_isr() {
    if (millis() - lastWindSpeedTick > 25) {
        timeSinceLastWindTick = millis() - lastWindSpeedTick;
        lastWindSpeedTick = millis();
    }
}

int getWindDir() {
    auto windDirRaw = analogRead(35);
    return map(windDirRaw, 0, 4096, 0, 360);
}

void setup() {
    Serial.begin(115200);
    connect_mqtt(C_WIFI_SSID, C_WIFI_PASSWORD, C_MQTT_SERVER);
    pinMode(23, INPUT_PULLUP);
    pinMode(35, INPUT_PULLDOWN);
    pinMode(32, INPUT_PULLUP);
    analogSetAttenuation(ADC_11db);  // For all pins
    attachInterrupt(23, rain_isr, RISING);
    attachInterrupt(32, wind_isr, RISING);
    Serial.println("setup ");
    dht.begin();
}

void loop() {
    connect_mqtt(C_WIFI_SSID, C_WIFI_PASSWORD, C_MQTT_SERVER);
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int windDir = getWindDir();

    double windspeed = (0.314 / timeSinceLastWindTick * 1000);
    if (millis() - lastWindSpeedTick > 4000) {
        windspeed = 0;
    }
    double windspeedKmh = windspeed * 3.6;

    client.publish("weather/wetterstation1/totalRainMM", String(totalRain, 3).c_str());
    client.publish("weather/wetterstation1/temperature", String(temperature, 2).c_str());
    client.publish("weather/wetterstation1/humidity", String(humidity, 2).c_str());
    client.publish("weather/wetterstation1/windDirection", String(windDir).c_str());
    client.publish("weather/wetterstation1/windSpeed", String(windspeed, 2).c_str());
    client.publish("weather/wetterstation1/windSpeedKmh", String(windspeedKmh, 2).c_str());
    delay(1000);
}
