#ifndef SDCARD_H  
#define SDCARD_H  

#include <Arduino.h> 
#include <SdFat.h>    


void sdCardInit();  
bool sdCardIsReady();  
bool rtcIsReady();  
void rtcFormatTimestamp(char* out, size_t outLen);  
SdFs& sdCardFs();  

#endif  
//functions and variables related to SD card initialization, RTC timestamping, and providing access to the SdFs instance for file operations.