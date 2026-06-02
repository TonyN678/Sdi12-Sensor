#include "AvgDataLogger.h" 
#include "HardwareConfig.h" 
#include "SdCard.h"
#include "SensorSampler.h" 
#include "SensorReading.h"  

#include <SdFat.h>  


//local helper function creation
namespace { 
const char* kAvgLogFileName = "avg_datalog.csv";  

//function to append to the SD card
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

//function to format and print sensor averages
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

//function to append one row of average data to the SD card
void writeAvgRow(const char* source) {  
  if (!isAveragedSensorDataReady()) { 
    return;  
  }

  const SensorData d = getAveragedSensorData(); 
  char timestamp[24]; 
  rtcFormatTimestamp(timestamp, sizeof(timestamp));  

  if (!sdCardIsReady()) {  
    Serial.print(F("[AvgLog] SD unavailable — serial only: ")); 
    printAvgRowToSerial(timestamp, d, kSamplesPerAverageWindow); 
    return;  
  }

  SdFs& sd = sdCardFs(); 
  FsFile file = sd.open(kAvgLogFileName, FILE_WRITE);  
  if (!file) { 
    Serial.println(F("[AvgLog] SD write failed.")); 
    printAvgRowToSerial(timestamp, d, kSamplesPerAverageWindow); 
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
  file.print(kSamplesPerAverageWindow); 
  file.print(','); 
  file.println(source);  

  file.close(); 
  printAvgRowToSerial(timestamp, d, kSamplesPerAverageWindow);  
}

} 

//function to intialise the SD card for writing average data logs
void avgDataLoggerInit() {  
  appendAvgCsvHeaderIfNew(); 
  Serial.println(F("[AvgLog] Ready (avg_datalog.csv)."));  
}

//Function to call the loop and append a new row once every set interval
void avgDataLoggerUpdate() {  
  if (isAveragedSensorDataReady()) {  
    writeAvgRow("ISR_Avg");  
    sensorSamplerAcknowledgeAverage(); 
  }
}
