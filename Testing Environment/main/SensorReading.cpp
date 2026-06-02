#include "SensorReading.h"
#include <Wire.h>

Adafruit_BME280 bme;
BH1750 lightMeter;

bool bmeOK = false;
bool lightOK = false;

static SensorData sensorBuffer;

// Initialize the sensors
void sensorsInit() {
  Wire.begin();

  
  lightOK = lightMeter.begin();
  bmeOK   = bme.begin(0x76);

  Serial.print("[Init] BME280: ");
  Serial.println(bmeOK ? "OK" : "FAIL");

  Serial.print("[Init] BH1750: ");
  Serial.println(lightOK ? "OK" : "FAIL");

  sensorBuffer.ready = false;
}

// Read the sensors
void readSensors() {
  // ---- BME280 ----
  float pres = bme.readPressure();

  if (isnan(pres) || pres <= 0) {
    bmeOK = false;
  }
  else {
    bmeOK = true;

    sensorBuffer.temperature = bme.readTemperature();
    sensorBuffer.humidity    = bme.readHumidity();
    sensorBuffer.pressure    = pres / 100.0;
  }

  
  float lux = lightMeter.readLightLevel();

  if (isnan(lux) || lux < 0) {
    lightOK = false;
  }
  else {
    lightOK = true;
    sensorBuffer.lux = lux;
  }

  
  sensorBuffer.ready = (bmeOK || lightOK);



}


// Get the sensor data
SensorData getSensorData() {
  return sensorBuffer;
}

// Get the number of parameters
int getParameterCount() {
  int count = 0;

  if (bmeOK) {
    count += 3; 
  }

  if (lightOK) {
    count += 1; 
  }

  return count;
}

// Check if the BME280 is OK
bool isBme280Ok() {
  return bmeOK;
}

// Check if the BH1750 is OK
bool isBh1750Ok() {
  return lightOK;
}