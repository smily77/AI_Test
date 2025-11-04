#include <Arduino_GFX_Library.h>

constexpr int8_t TFT_CS = 15;
constexpr int8_t TFT_DC = 2;
constexpr int8_t TFT_SCK = 14;
constexpr int8_t TFT_MOSI = 13;
constexpr int8_t TFT_MISO = 12;
constexpr int8_t TFT_RST = -1;
constexpr int8_t TFT_BL = 21;
constexpr uint8_t TFT_BL_CHANNEL = 7;
constexpr uint32_t TFT_BL_FREQ = 44100;

Arduino_DataBus *bus =
    new Arduino_ESP32SPI(TFT_CS /* CS */, TFT_DC /* DC */, TFT_SCK /* SCK */,
                         TFT_MOSI /* MOSI */, TFT_MISO /* MISO */);
Arduino_GFX *gfx =
    new Arduino_ILI9341(bus, TFT_RST /* RST */, 0 /* rotation */, false /* IPS */);

static inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static void drawGradientColumn(int16_t x, int16_t width,
                               uint8_t startR, uint8_t startG, uint8_t startB,
                               uint8_t endR, uint8_t endG, uint8_t endB) {
  const int16_t height = gfx->height();
  for (int16_t y = 0; y < height; ++y) {
    const float ratio = (height <= 1) ? 0.0f : static_cast<float>(y) / (height - 1);
    const uint8_t r = startR + static_cast<uint8_t>((endR - startR) * ratio);
    const uint8_t g = startG + static_cast<uint8_t>((endG - startG) * ratio);
    const uint8_t b = startB + static_cast<uint8_t>((endB - startB) * ratio);
    const uint16_t color = color565(r, g, b);
    gfx->drawFastHLine(x, y, width, color);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("CYD ArduinoGFX gradient comparison");

  ledcSetup(TFT_BL_CHANNEL, TFT_BL_FREQ, 8);
  ledcAttachPin(TFT_BL, TFT_BL_CHANNEL);
  ledcWrite(TFT_BL_CHANNEL, 255);

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

void loop() {}
