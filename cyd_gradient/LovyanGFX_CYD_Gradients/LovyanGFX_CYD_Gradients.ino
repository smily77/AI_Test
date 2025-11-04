#include "LovyanGFX_CYD_Config.h"

LGFX tft;

static void drawGradientColumn(int16_t x, int16_t width,
                                uint8_t startR, uint8_t startG, uint8_t startB,
                                uint8_t endR, uint8_t endG, uint8_t endB) {
  const int16_t height = tft.height();
  for (int16_t y = 0; y < height; ++y) {
    float ratio = (height <= 1) ? 0.0f : static_cast<float>(y) / (height - 1);
    uint8_t r = startR + static_cast<uint8_t>((endR - startR) * ratio);
    uint8_t g = startG + static_cast<uint8_t>((endG - startG) * ratio);
    uint8_t b = startB + static_cast<uint8_t>((endB - startB) * ratio);
    uint16_t color = tft.color565(r, g, b);
    tft.drawFastHLine(x, y, width, color);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("CYD LovyanGFX gradient comparison");

  tft.init();
  tft.setRotation(0);  // Portrait
  tft.setBrightness(255);

  const int16_t width = tft.width();
  tft.fillScreen(tft.color565(0, 0, 0));

  const int16_t columnWidth = width / 3;
  int16_t x = 0;

  drawGradientColumn(x, columnWidth,
                     255, 200, 200,
                     60, 0, 0);
  x += columnWidth;

  drawGradientColumn(x, columnWidth,
                     200, 255, 200,
                     0, 60, 0);
  x += columnWidth;

  drawGradientColumn(x, width - x,
                     200, 200, 255,
                     0, 0, 60);

  Serial.println("Gradients drawn.");
}

void loop() {
}
