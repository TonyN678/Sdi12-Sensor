#include "SensorReading.h"
#include "SensorSampler.h"
#include "sdi12.h"
#include "SdCard.h"
#include "DataLogger.h"
#include "AvgDataLogger.h"
#include "Dashboard.h"
#include "Watchdog.h"

#define DIRO_PIN 8
#define MY_ADDRESS '0'

// Initialise sensors, SD card, timer sampler, loggers, dashboard, SDI-12, and watchdog
void setup() {
  Serial.begin(9600);

  sensorsInit();
  sdCardInit();
  sensorSamplerInit();
  dataLoggerInit();
  avgDataLoggerInit();
  dashboardInit();
  sdi12Init(MY_ADDRESS, DIRO_PIN);

  readSensors();
  watchdogInit();

  Serial.println(F("SDI-12 slave + ISR sampler + loggers + dashboard ready."));
}

// Continuously run the main system tasks and reset the watchdog timer
void loop() {
  watchdogKick();

  sensorSamplerService();
  watchdogKick();

  sdi12Handle();
  watchdogKick();

  avgDataLoggerUpdate();
  watchdogKick();

  dataLoggerUpdate();
  watchdogKick();

  dashboardUpdate();
  watchdogKick();
}