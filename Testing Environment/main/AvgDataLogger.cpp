#include "AvgDataLogger.h"  
#include "HardwareConfig.h"  
#include "SdCard.h"  
#include "SensorSampler.h"  
#include "SensorReading.h"  

#include <SdFat.h>  

// Private helpers for this file only
namespace {  

const char* kAvgLogFileName = "avg_datalog.csv";  

// Write header row once when file does not exist
void appendAvgCsvHeaderIfNew() {  
  if (!sdCardIsReady()) {  
    return;  
  }
  SdFs& sd = sdCardFs();  
  if (sd.exists(kAvgLogFileName)) {  
    return;  
  }

  FsFile file = sd.open(kAvgLogFileName, FILE_WRITE);  
  if (!file) {  
    Serial.println(F("[AvgLog] ERROR: Could not create avg_datalog.csv."));  
    return;  
  }
  file.println(F("Timestamp,Temperature_C,Humidity_pct,Pressure_hPa,Lux,SampleCount,Source"));  
  file.close();  
  Serial.println(F("[AvgLog] avg_datalog.csv header created."));  
}

// Print the averaged sensor data to the serial monitor
void printAvgRowToSerial(const char* timestamp, const SensorData& d, uint8_t sampleCount) {  
  Serial.print(F("[AvgLog] "));  
  Serial.print(timestamp);  
  Serial.print(F(" "));  
  Serial.print(d.temperature, 2);  
  Serial.print(F("C "));  
  Serial.print(d.humidity, 1);  
  Serial.print(F("% "));  
  Serial.print(d.pressure, 1);  
  Serial.print(F("hPa "));  
  Serial.print(d.lux, 1);  
  Serial.print(F(" lx ("));  
  Serial.print(sampleCount);  
  Serial.println(F(" samples)"));  
}

// Write the averaged sensor data to the SD card
void writeAvgRow(const char* source) {  
  if (!isAveragedSensorDataReady()) {  
    return;  
  }

  const SensorData d = getAveragedSensorData();  
  char timestamp[24];  
  rtcFormatTimestamp(timestamp, sizeof(timestamp));  

  if (!sdCardIsReady()) {  
    Serial.print(F("[AvgLog] SD unavailable — serial only: "));  
    printAvgRowToSerial(timestamp, d, NumberOfSamplePerAvg);  
    return;  
  }

  SdFs& sd = sdCardFs();  
  FsFile file = sd.open(kAvgLogFileName, FILE_WRITE);  
  if (!file) {  
    Serial.println(F("[AvgLog] SD write failed."));  
    printAvgRowToSerial(timestamp, d, NumberOfSamplePerAvg);  
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
  file.print(NumberOfSamplePerAvg);  
  file.print(',');  
  file.println(source);  

  file.close();  
  printAvgRowToSerial(timestamp, d, NumberOfSamplePerAvg);  
}

}  // namespace

// Initialize the averaged data logger
void avgDataLoggerInit() {  
  appendAvgCsvHeaderIfNew();  
  Serial.println(F("[AvgLog] Ready (avg_datalog.csv)."));  
}

// Update the averaged data logger
void avgDataLoggerUpdate() {  
  if (isAveragedSensorDataReady()) {  
    writeAvgRow("ISR_Avg");  
    sensorSamplerAcknowledgeAverage();  
  }
}
