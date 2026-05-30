#ifndef HARDWARECONFIG_H
#define HARDWARECONFIG_H

#include <Arduino.h>

// SD card (soft SPI) — same wiring as project.ino
constexpr uint8_t kSdCsPin = A3;
constexpr uint8_t kSoftMisoPin = 12;
constexpr uint8_t kSoftMosiPin = 11;
constexpr uint8_t kSoftSckPin = 13;

// Pushbuttons: active LOW with INPUT_PULLUP
constexpr int kBtnManualLogPin = 2;
constexpr int kBtnClearSdPin = 3;

// ST7735 TFT (hardware SPI — shares MOSI/SCK with SD soft SPI)
constexpr uint8_t kTftCsPin = 10;
constexpr uint8_t kTftDcPin = 7;
constexpr uint8_t kTftRstPin = 6;
constexpr uint8_t kTftMosiPin = 11;
constexpr uint8_t kTftSclkPin = 13;

constexpr unsigned long kLogIntervalMs = 60000UL;
constexpr unsigned long kDashboardRefreshMs = 1000UL;

// Arduino Due DueTimer: one I2C sample request every 2 s.
constexpr unsigned long kSensorSampleMs = 2000UL;   // 2s sample period
constexpr uint8_t kSamplesPerAverageWindow = 10;    // average of 10 samples


// External activity LED pin (you set this to D9).
constexpr uint8_t kIsrActivityLedPin = 9;

// LED stays on for 200ms after a successful average
constexpr unsigned long kIsrActivityLedPulseMs = 200UL;

#endif
