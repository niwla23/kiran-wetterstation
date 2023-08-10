// Wind
// Gelb: 3V3
// Rot: Ground
// Grün: G35
// schwarz: kommt noch

// Temperatur und Hum
// Grün: D0
// Schwarz: GND
// Rot: 5V

// Regen
// Pin: 23


#include <Arduino.h>
#include "DHT.h"

#define BLYNK_TEMPLATE_ID "TMPL4T81nWScz"
#define BLYNK_TEMPLATE_NAME "Wetterstation"
#define BLYNK_AUTH_TOKEN "sTJ5DdZa3VB9luckBCxEAoDv9h_uzsvX"
#include <BlynkSimpleEsp32.h>

const double RAIN_TICK_TO_MM = 0.12;

double totalRain = 0;
double windSpeed = 0;
double windspeed = 0;

unsigned long lastRainTick = millis();
unsigned long lastWindSpeedTick = millis();

DHT dht(0, DHT11);

void IRAM_ATTR rain_isr() {
  if (millis() - lastRainTick > 500) {
    totalRain += RAIN_TICK_TO_MM;
    lastRainTick = millis();
  }
}

void IRAM_ATTR wind_isr() {
  Serial.println("wind");
  if (millis() - lastWindSpeedTick > 25) {
    windSpeed = millis() - lastWindSpeedTick;
    lastWindSpeedTick = millis();
  }
}


int getWindDir() {
  auto windDirRaw = analogRead(35);
  return map(windDirRaw, 0, 4096, 0, 360);
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, "Re-Ho-Ind", "husarenstrasse84aneuhaus");
  pinMode(23, INPUT_PULLUP);
  pinMode(35, INPUT_PULLDOWN);
  pinMode(32, INPUT_PULLUP);
  analogSetAttenuation(ADC_11db);  //For all pins
  attachInterrupt(23, rain_isr, RISING);
  attachInterrupt(32, wind_isr, RISING);
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int windDir = getWindDir();

  Blynk.virtualWrite(0, totalRain);
  Blynk.virtualWrite(1, t);
  Blynk.virtualWrite(2, h);
  Blynk.virtualWrite(3, windDir);
  Blynk.run();
  windspeed=(0.314/windSpeed*1000);


  Serial.print(windspeed);
  Serial.println("m/s");
  Serial.print(windspeed*3.6);
  Serial.println("km/h");






  delay(100);
}