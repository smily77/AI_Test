#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPI(15 /* CS */, 2 /* DC */, 14 /* SCK */, 13 /* MOSI */, 12 /* MISO */);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, 4 /* RST */, 0 /* rotation */, false /* IPS */);

static inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static void drawGradientColumn(int16_t x, int16_t width,
                                uint8_t startR, uint8_t startG, uint8_t startB,
                                uint8_t endR, uint8_t endG, uint8_t endB) {
  const int16_t height = gfx->height();
  for (int16_t y = 0; y < height; ++y) {
    float ratio = (height <= 1) ? 0.0f : static_cast<float>(y) / (height - 1);
    uint8_t r = startR + static_cast<uint8_t>((endR - startR) * ratio);
    uint8_t g = startG + static_cast<uint8_t>((endG - startG) * ratio);
    uint8_t b = startB + static_cast<uint8_t>((endB - startB) * ratio);
    uint16_t color = color565(r, g, b);
    gfx->drawFastHLine(x, y, width, color);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("CYD ArduinoGFX gradient comparison");

  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);

  if (!gfx->begin()) {
    Serial.println("Display init failed");
    while (true) {
      delay(1000);
    }
  }

  gfx->setRotation(0);  // Portrait
  gfx->fillScreen(0x0000);

  const int16_t width = gfx->width();
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
