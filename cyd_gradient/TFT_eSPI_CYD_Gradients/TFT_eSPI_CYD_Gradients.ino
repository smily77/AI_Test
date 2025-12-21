
// Board configuration for CYD
#define USER_SETUP_LOADED
#define ILI9341_DRIVER
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_MISO -1
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TFT_BL 22
#define PIN_TOUCH_CS 21
#define PIN_TOUCH_IRQ 36
#define TOUCH_MOSI 32
#define TOUCH_MISO 35
#define TOUCH_SCLK 25
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  drawGradient(0, 0, tft.width(), tft.height());
}

void loop() {
  // Empty
}

void drawGradient(int16_t x, int16_t y, int16_t w, int16_t h) {
  // Dark blue to light blue gradient
  uint8_t startR = 0, startG = 0, startB = 50;
  uint8_t endR = 173, endG = 216, endB = 230; // LightBlue

  for (int16_t i = 0; i < h; i++) {
    float ratio = (float)i / (h - 1);
    uint8_t r = startR + (endR - startR) * ratio;
    uint8_t g = startG + (endG - startG) * ratio;
    uint8_t b = startB + (endB - startB) * ratio;
    tft.drawFastHLine(x, y + i, w, tft.color565(r, g, b));
  }
}
