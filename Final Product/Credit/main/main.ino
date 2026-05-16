#include "SensorReading.h"
#include "sdi12.h"
#include "DataLogger.h"
#include "Dashboard.h"

#define DIRO_PIN 8   // pin 7 is TFT_DC
// Default address is '0'
#define MY_ADDRESS '0'

namespace {
constexpr unsigned long kSensorPollMs = 500UL;
unsigned long lastSensorPollMs = 0;
}  // namespace

void setup() {
  Serial.begin(9600);

  sensorsInit();
  dataLoggerInit();
  dashboardInit();
  sdi12Init(MY_ADDRESS, DIRO_PIN);

  readSensors();

  Serial.println(F("SDI-12 slave + logger + dashboard ready."));
}

void loop() {
  unsigned long now = millis();
  if (now - lastSensorPollMs >= kSensorPollMs) {
    lastSensorPollMs = now;
    readSensors();
  }

  sdi12Handle();
  dataLoggerUpdate();
  dashboardUpdate();
}
