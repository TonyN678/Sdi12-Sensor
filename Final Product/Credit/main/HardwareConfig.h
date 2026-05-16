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

#endif
