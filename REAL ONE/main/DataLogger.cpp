#include "DataLogger.h"
#include "HardwareConfig.h"
#include "SensorReading.h"

#include <Bounce2.h>
#include <RTClib.h>
#include <SdFat.h>

//Everything inside is private to THIS .cpp file only.
namespace {

SdFs sd;
FsFile file;

SoftSpiDriver<kSoftMisoPin, kSoftMosiPin, kSoftSckPin> softSpi;
#define SD_CONFIG SdSpiConfig(kSdCsPin, SHARED_SPI, SD_SCK_MHZ(4), &softSpi)

RTC_DS1307 rtc;

const char* logFileName = "datalog.csv";

unsigned long lastLogTimeMs = 0;

Bounce debouncerManual;
Bounce debouncerClear;

void appendCsvHeaderIfNew() {
  if (sd.exists(logFileName)) {
    return;
  }
  file = sd.open(logFileName, FILE_WRITE);
  if (!file) {
    Serial.println(F("[Log] ERROR: Could not create CSV."));
    return;
  }
  file.println(F("Timestamp,Temperature_C,Humidity_pct,Pressure_hPa,Lux,Source"));
  file.close();
  Serial.println(F("[Log] CSV header created."));
}

void logDataInternal(const char* source) {
  DateTime now = rtc.now();

  readSensors();
  SensorData d = getSensorData();

  char timestamp[20];
  snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());

  Serial.print(F("[Log] ("));
  Serial.print(source);
  Serial.print(F(") "));
  Serial.print(timestamp);

  file = sd.open(logFileName, FILE_WRITE);
  if (!file) {
    Serial.println(F(" — open failed."));
    return;
  }

  file.print(timestamp);

  file.print(',');
  if (isBme280Ok()) {
    file.print(d.temperature, 2);
  }

  file.print(',');
  if (isBme280Ok()) {
    file.print(d.humidity, 2);
  }

  file.print(',');
  if (isBme280Ok()) {
    file.print(d.pressure, 2);
  }

  file.print(',');
  if (isBh1750Ok()) {
    file.print(d.lux, 2);
  }

  file.print(',');
  file.println(source);

  file.close();

  Serial.print(F(" "));
  Serial.print(d.temperature, 2);
  Serial.print(F("C "));
  Serial.print(d.humidity, 1);
  Serial.print(F("% "));
  Serial.print(d.pressure, 1);
  Serial.print(F("hPa "));
  Serial.print(d.lux, 1);
  Serial.println(F(" lx"));
}

void clearSdLogFile() {
  Serial.println(F("[Log] Clearing log file..."));
  if (sd.exists(logFileName)) {
    if (!sd.remove(logFileName)) {
      Serial.println(F("[Log] ERROR: Could not delete file."));
      return;
    }
  }
  appendCsvHeaderIfNew();
  Serial.println(F("[Log] Log file reset."));
}

}  // END of namespace

void dataLoggerInit() {
  pinMode(kBtnManualLogPin, INPUT_PULLUP);
  pinMode(kBtnClearSdPin, INPUT_PULLUP);

  debouncerManual.attach(kBtnManualLogPin);
  debouncerManual.interval(50);
  debouncerClear.attach(kBtnClearSdPin);
  debouncerClear.interval(50);

  if (!rtc.begin()) {
    Serial.println(F("[Log] ERROR: RTC not found."));
    while (1) {
      delay(1000);
    }
  }
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("[Log] RTC stopped — time set from compile time."));
  }

  if (!sd.begin(SD_CONFIG)) {
    Serial.println(F("[Log] ERROR: SD init failed."));
    while (1) {
      delay(1000);
    }
  }

  appendCsvHeaderIfNew();
  lastLogTimeMs = millis();
  Serial.println(F("[Log] Ready (60s auto + manual / clear buttons)."));
}

void dataLoggerUpdate() {
  unsigned long now = millis();
  if (now - lastLogTimeMs >= kLogIntervalMs) {
    lastLogTimeMs = now;
    logDataInternal("Periodic");
  }

  debouncerManual.update();
  debouncerClear.update();

  if (debouncerManual.fell()) {
    logDataInternal("Manual");
    Serial.println(F("[Log] Manual entry saved."));
  }

  if (debouncerClear.fell()) {
    clearSdLogFile();
  }
}
