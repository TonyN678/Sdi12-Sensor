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

bool sdOk = false;
bool rtcOk = false;

Bounce debouncerManual;
Bounce debouncerClear;

void formatTimestamp(char* out, size_t outLen) {
  if (rtcOk) {
    DateTime now = rtc.now();
    snprintf(out, outLen, "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
  } else {
    unsigned long sec = millis() / 1000UL;
    snprintf(out, outLen, "uptime_%lu", sec);
  }
}

void appendCsvHeaderIfNew() {
  if (!sdOk) {
    return;
  }
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

void printLogToSerial(const char* source, const char* timestamp, const SensorData& d) {
  Serial.print(F("[Log] ("));
  Serial.print(source);
  Serial.print(F(") "));
  Serial.print(timestamp);
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

void logDataInternal(const char* source) {
  char timestamp[24];
  formatTimestamp(timestamp, sizeof(timestamp));

  readSensors();
  SensorData d = getSensorData();

  if (!sdOk) {
    Serial.print(F("[Log] SD unavailable — serial only: "));
    printLogToSerial(source, timestamp, d);
    return;
  }

  file = sd.open(logFileName, FILE_WRITE);
  if (!file) {
    Serial.println(F("[Log] SD write failed (card removed?)."));
    sdOk = false;
    printLogToSerial(source, timestamp, d);
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
  printLogToSerial(source, timestamp, d);
}

void clearSdLogFile() {
  if (!sdOk) {
    Serial.println(F("[Log] SD unavailable — cannot clear."));
    return;
  }
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

}  // namespace

void dataLoggerInit() {
  pinMode(kBtnManualLogPin, INPUT_PULLUP);
  pinMode(kBtnClearSdPin, INPUT_PULLUP);

  debouncerManual.attach(kBtnManualLogPin);
  debouncerManual.interval(50);
  debouncerClear.attach(kBtnClearSdPin);
  debouncerClear.interval(50);

  rtcOk = rtc.begin();
  if (!rtcOk) {
    Serial.println(F("[Log] WARN: RTC not found — timestamps use uptime."));
  } else if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("[Log] RTC stopped — time set from compile time."));
  }

  sdOk = sd.begin(SD_CONFIG);
  if (!sdOk) {
    Serial.println(F("[Log] WARN: SD card init failed — logging disabled, rest of system runs."));
  } else {
    appendCsvHeaderIfNew();
    Serial.println(F("[Log] SD card ready."));
  }

  lastLogTimeMs = millis();
  Serial.println(F("[Log] Buttons active (manual log / clear)."));
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
    if (sdOk) {
      Serial.println(F("[Log] Manual entry saved to SD."));
    }
  }

  if (debouncerClear.fell()) {
    clearSdLogFile();
  }
}
