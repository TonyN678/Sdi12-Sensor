#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   10
#define TFT_RST  6  
#define TFT_DC   7
#define TFT_SCLK   13
#define TFT_MOSI   11
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

float temp = 24.6;        //just dummy variables for now 
float humidity = 67;
float pressure = 420;
float light = 911;

void setup() {
tft.initR(INITR_BLACKTAB);
tft.setRotation(1);
}

void loop() {
Serial.begin(9600);

tft.fillScreen(ST77XX_BLACK);
tft.setTextSize(2);
    
tft.setTextColor(ST77XX_RED);
tft.setCursor(10, 10);
tft.print("Temp:");
tft.print(temp);

tft.setTextColor(ST77XX_CYAN);  
tft.setCursor(10, 30);
tft.print("Humidity:");
tft.print(humidity);
    
tft.setTextColor(ST77XX_YELLOW);    
tft.setCursor(10, 65);
tft.print("Pressure:");
tft.print(pressure);

tft.setTextColor(ST77XX_GREEN);    
tft.setCursor(10, 110);
tft.print("Light:");
tft.print(light);

delay(1000);

}