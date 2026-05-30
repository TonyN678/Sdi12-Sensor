#include "SensorSampler.h"
#include "HardwareConfig.h"

#include <DueTimer.h>

// DueTimer period in microseconds (2000 ms = 2 s sample tick)
constexpr long kSamplePeriodUs =
    static_cast<long>(kSensorSampleMs * 1000UL);

// ----------------------------------------------------
// Module state
// ----------------------------------------------------

static volatile bool samplePending = false;
volatile uint32_t g_hwTimerIsrCount = 0;

static uint8_t samplesInCurrentWindow = 0;

static float tempSum = 0.0f;
static float humidSum = 0.0f;
static float pressSum = 0.0f;
static float luxSum = 0.0f;
static uint16_t bmeSampleCount = 0;
static uint16_t luxSampleCount = 0;

static SensorData avgBuffer;
static volatile bool newAverageAvailable = false;
static volatile unsigned long ledPulseOffAtMs = 0;

// ----------------------------------------------------
// Autostart in Setup()
// ----------------------------------------------------

void sensorSamplerInit() {
  pinMode(kIsrActivityLedPin, OUTPUT);
  setActivityLed(false);

  samplePending = false;
  avgBuffer.ready = false;
  newAverageAvailable = false;
  samplesInCurrentWindow = 0;
  ledPulseOffAtMs = 0;
  tempSum = humidSum = pressSum = luxSum = 0.0f;
  bmeSampleCount = luxSampleCount = 0;

  Timer6.attachInterrupt(onSampleTimerISR);
  Timer6.setPeriod(kSamplePeriodUs);
  Timer6.start();

  Serial.print(F("[Sampler] DueTimer6 every "));
  Serial.print(kSensorSampleMs);
  Serial.println(F(" ms (hardware timer)."));
}

// ISR to request a sample every kSensorSampleMs
static void onSampleTimerISR() {
  g_hwTimerIsrCount++;
  samplePending = true;
}

// LED to pulse for 200ms after a successful average
static void serviceActivityLed() {
  if (ledPulseOffAtMs == 0) {
    return;
  }
  if (static_cast<long>(millis() - ledPulseOffAtMs) >= 0) {
    setActivityLed(false);
    ledPulseOffAtMs = 0;
  }
}

// Called from loop(), processes pending ISR ticks: read sensors and build averages
void sensorSamplerService() {
  serviceActivityLed();

  if (!samplePending) {
    return;
  }
  // Disable interrupts to avoid race conditions
  noInterrupts();
  samplePending = false;
  interrupts();

  accumulateOneSample();
  samplesInCurrentWindow++;

  if (samplesInCurrentWindow >= kSamplesPerAverageWindow) {
    finalizeAverageWindow();
    samplesInCurrentWindow = 0;
  }
}

// ----------------------------------------------------
// Sampling
// ----------------------------------------------------

static void accumulateOneSample() {
  readSensors();
  const SensorData sample = getSensorData();

  if (isBme280Ok()) {
    tempSum += sample.temperature;
    humidSum += sample.humidity;
    pressSum += sample.pressure;
    bmeSampleCount++;
  }

  if (isBh1750Ok()) {
    luxSum += sample.lux;
    luxSampleCount++;
  }
}

static void finalizeAverageWindow() {
  if (bmeSampleCount > 0) {
    const float n = static_cast<float>(bmeSampleCount);
    avgBuffer.temperature = tempSum / n;
    avgBuffer.humidity = humidSum / n;
    avgBuffer.pressure = pressSum / n;
  }

  if (luxSampleCount > 0) {
    avgBuffer.lux = luxSum / static_cast<float>(luxSampleCount);
  }

  avgBuffer.ready = (bmeSampleCount > 0 || luxSampleCount > 0);
  newAverageAvailable = avgBuffer.ready;

  if (avgBuffer.ready) {
    pulseActivityLed();  // 200 ms LED ON
    printAverageReady();
  } else {
    Serial.println(F("[Sampler] AVG skipped (no valid sensor samples)."));
  }

  tempSum = 0.0f;
  humidSum = 0.0f;
  pressSum = 0.0f;
  luxSum = 0.0f;
  bmeSampleCount = 0;
  luxSampleCount = 0;
}

// ----------------------------------------------------
// LED
// ----------------------------------------------------

static void pulseActivityLed() {
  setActivityLed(true);
  // LED stays on for 200ms after a successful average
  ledPulseOffAtMs = millis() + kIsrActivityLedPulseMs;
}

static void setActivityLed(bool on) {
  digitalWrite(kIsrActivityLedPin, HIGH);
}

static void printAverageReady() {
  Serial.print(F("[Sampler] AVG ready T="));
  Serial.print(avgBuffer.temperature, 2);
  Serial.print(F("C H="));
  Serial.print(avgBuffer.humidity, 2);
  Serial.print(F("% P="));
  Serial.print(avgBuffer.pressure, 2);
  Serial.print(F("hPa L="));
  Serial.print(avgBuffer.lux, 2);
  Serial.println(F("lx"));
}

SensorData getAveragedSensorData() {
  return avgBuffer;
}

bool isAveragedSensorDataReady() {
  return newAverageAvailable;
}

void sensorSamplerAcknowledgeAverage() {
  newAverageAvailable = false;
}
