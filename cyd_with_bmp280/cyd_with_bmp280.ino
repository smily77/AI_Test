#include <Wire.h>

#include <CYD_Display_Config.h>

#ifndef extSDA
  #define extSDA 22
#endif
#ifndef extSCL
  #define extSCL 27
#endif

#include <Adafruit_BMP280.h>

#include <Adafruit_AHTX0.h>

LGFX lcd;

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;

#define BMP280_I2C_ADDRESS 0x77
#define AHT20_I2C_ADDRESS  0x38

const int screenWidth = 320;
const int screenHeight = 240;

#define TFT_BLACK       0x0000
#define TFT_WHITE       0xFFFF
#define TFT_RED         0xF800
#define TFT_GREEN       0x07E0
#define TFT_BLUE        0x001F
#define TFT_CYAN        0x07FF
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_ORANGE      0xFD20
#define TFT_DARKGREY    0x7BEF

const int HISTORY_SIZE = 144;
float tempHistory[HISTORY_SIZE];
float humidHistory[HISTORY_SIZE];
float pressureHistory[HISTORY_SIZE];
int historyIndex = 0;

const unsigned long MEASUREMENT_INTERVAL_MS = 10UL * 60UL * 1000UL;
unsigned long lastMeasurementTime = 0;

void updateSensorData();
void initializeHistory();
void drawMainDisplay();
void drawGraph(int x, int y, int w, int h, const char* title, float* data, int dataSize, float minVal, float maxVal, uint32_t lineColor, uint32_t textColor);


void setup() {
  Serial.begin(115200);
  Serial.println("Starting CYD Sensor Display...");

  // --- Display Initialization ---
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  lcd.fillScreen(TFT_BLACK);

  // --- I2C Initialization ---
  Wire.begin(extSDA, extSCL);
  Serial.print("I2C initialized on SDA: "); Serial.print(extSDA); Serial.print(", SCL: "); Serial.println(extSCL);

  // --- BMP280 Sensor Initialization ---
  Serial.print("Initializing BMP280...");
  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("ERROR: Could not find a valid BMP280 sensor, check wiring or address!");
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("BMP280 Error!", screenWidth / 2, screenHeight / 2);
    while (1);
  }
  Serial.println("BMP280 initialized.");
  // Configure BMP280 sensor settings for optimal readings
  // For I2C communication, the setSampling method takes 5 arguments (mode, temp_sampling, pressure_sampling, standby_time, filter).
  // The 6th argument (SPI_3_WIRE_INTERFACE) is incorrect and only applies to SPI configuration, if at all.
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode: Normal
                  Adafruit_BMP280::SAMPLING_X16,    // Temperature oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::STANDBY_MS_500,  // Standby time: 500ms
                  Adafruit_BMP280::FILTER_X16);     // IIR filter sampling: x16

  // --- AHT20 Sensor Initialization ---
  Serial.print("Initializing AHT20...");
  if (!aht.begin(&Wire)) {
    Serial.println("ERROR: Could not find AHT20 sensor, check wiring or address!");
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("AHT20 Error!", screenWidth / 2, screenHeight / 2 + 20);
    while (1);
  }
  Serial.println("AHT20 initialized.");

  initializeHistory();
  updateSensorData();

  lastMeasurementTime = millis();
}

void loop() {
  if (millis() - lastMeasurementTime >= MEASUREMENT_INTERVAL_MS) {
    updateSensorData();
    lastMeasurementTime = millis();
  }

  drawMainDisplay();

  delay(500);
}

/**
 * @brief Reads temperature, humidity, and pressure from sensors and updates
 *        the circular history buffers.
 */
void updateSensorData() {
  sensors_event_t humidity, temp_aht;

  float currentTemp = bmp.readTemperature();
  float currentPressure = bmp.readPressure() / 100.0F;

  aht.getEvent(&humidity, &temp_aht);
  float currentHumidity = humidity.relative_humidity;

  tempHistory[historyIndex] = currentTemp;
  humidHistory[historyIndex] = currentHumidity;
  pressureHistory[historyIndex] = currentPressure;

  historyIndex = (historyIndex + 1) % HISTORY_SIZE;

  Serial.printf("New Data -> Temp: %.2fC, Humid: %.2f%%, Pressure: %.2fhPa\n",
                currentTemp, currentHumidity, currentPressure);
}

/**
 * @brief Fills the history arrays with an initial sensor reading.
 *        This prevents empty graphs when the program starts.
 */
void initializeHistory() {
  sensors_event_t humidity, temp_aht;
  float initialTemp = bmp.readTemperature();
  float initialPressure = bmp.readPressure() / 100.0F;
  aht.getEvent(&humidity, &temp_aht);
  float initialHumidity = humidity.relative_humidity;

  for (int i = 0; i < HISTORY_SIZE; i++) {
    tempHistory[i] = initialTemp;
    humidHistory[i] = initialHumidity;
    pressureHistory[i] = initialPressure;
  }
}

/**
 * @brief Draws the main display, including current sensor values and graphs.
 */
void drawMainDisplay() {
  lcd.fillRect(0, 0, screenWidth, screenHeight / 3, TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);

  float currentTemp = tempHistory[(historyIndex + HISTORY_SIZE - 1) % HISTORY_SIZE];
  float currentHumidity = humidHistory[(historyIndex + HISTORY_SIZE - 1) % HISTORY_SIZE];
  float currentPressure = pressureHistory[(historyIndex + HISTORY_SIZE - 1) % HISTORY_SIZE];

  lcd.setFont(&FreeSansBold24pt7b);
  lcd.setTextColor(TFT_RED, TFT_BLACK);
  lcd.drawString("Temp:", screenWidth / 6, screenHeight / 10);
  lcd.drawString(String(currentTemp, 1) + " C", screenWidth / 6, screenHeight / 4);

  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.drawString("Humid:", screenWidth / 2, screenHeight / 10);
  lcd.drawString(String(currentHumidity, 0) + " %", screenWidth / 2, screenHeight / 4);

  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.drawString("Press:", screenWidth * 5 / 6, screenHeight / 10);
  lcd.drawString(String(currentPressure, 0) + " hPa", screenWidth * 5 / 6, screenHeight / 4);

  const int graphYStart = screenHeight / 3 + 10;
  const int graphPadding = 5;
  const int graphHeight = (screenHeight - graphYStart - (graphPadding * 2) - 10) / 3;
  const int graphWidth = screenWidth - 20;
  const int graphX = 10;

  float minTemp = 1000, maxTemp = -1000;
  float minHumid = 1000, maxHumid = -1000;
  float minPressure = 100000, maxPressure = -100000;

  for (int i = 0; i < HISTORY_SIZE; i++) {
    minTemp = min(minTemp, tempHistory[i]);
    maxTemp = max(maxTemp, tempHistory[i]);
    minHumid = min(minHumid, humidHistory[i]);
    maxHumid = max(maxHumid, humidHistory[i]);
    minPressure = min(minPressure, pressureHistory[i]);
    maxPressure = max(maxPressure, pressureHistory[i]);
  }

  maxTemp += 1.0; minTemp -= 1.0;
  maxHumid += 2.0; minHumid -= 2.0;
  maxPressure += 2.0; minPressure -= 2.0;

  if (fabs(maxTemp - minTemp) < 0.1) { maxTemp += 0.5; minTemp -= 0.5; }
  if (fabs(maxHumid - minHumid) < 0.1) { maxHumid += 1.0; minHumid -= 1.0; }
  if (fabs(maxPressure - minPressure) < 0.1) { maxPressure += 1.0; minPressure -= 1.0; }

  drawGraph(graphX, graphYStart, graphWidth, graphHeight, "Temperature (24h)", tempHistory, HISTORY_SIZE, minTemp, maxTemp, TFT_RED, TFT_WHITE);

  drawGraph(graphX, graphYStart + graphHeight + graphPadding, graphWidth, graphHeight, "Humidity (24h)", humidHistory, HISTORY_SIZE, minHumid, maxHumid, TFT_CYAN, TFT_WHITE);

  drawGraph(graphX, graphYStart + (graphHeight + graphPadding) * 2, graphWidth, graphHeight, "Pressure (24h)", pressureHistory, HISTORY_SIZE, minPressure, maxPressure, TFT_YELLOW, TFT_WHITE);
}

/**
 * @brief Draws a single line graph within a specified area on the display.
 *
 * @param x         X-coordinate of the top-left corner of the graph area.
 * @param y         Y-coordinate of the top-left corner of the graph area.
 * @param w         Width of the graph area.
 * @param h         Height of the graph area.
 * @param title     Title of the graph.
 * @param data      Pointer to the array containing historical data points.
 * @param dataSize  Number of data points in the history array.
 * @param minVal    Minimum value for the Y-axis scaling.
 * @param maxVal    Maximum value for the Y-axis scaling.
 * @param lineColor Color of the graph line.
 * @param textColor Color of the graph title and labels.
 */
void drawGraph(int x, int y, int w, int h, const char* title, float* data, int dataSize, float minVal, float maxVal, uint32_t lineColor, uint32_t textColor) {
  lcd.fillRect(x, y, w, h, TFT_DARKGREY);
  lcd.drawRect(x, y, w, h, TFT_WHITE);

  lcd.setFont(&FreeSansBold9pt7b);
  lcd.setTextDatum(TL_DATUM);

  lcd.setTextColor(textColor, TFT_DARKGREY);
  lcd.drawString(title, x + 5, y + 2);

  lcd.setTextDatum(TR_DATUM);
  lcd.drawString(String(maxVal, 0), x + w - 5, y + 2);
  lcd.drawString(String(minVal, 0), x + w - 5, y + h - 12);

  lcd.setTextDatum(TL_DATUM);

  float valueRange = maxVal - minVal;
  if (valueRange == 0) valueRange = 1.0;

  float xScale = (float)w / (dataSize - 1);
  float yScale = (float)h / valueRange;

  for (int i = 0; i < dataSize - 1; i++) {
    int currentDataIdx = (historyIndex + i) % dataSize;
    int nextDataIdx = (historyIndex + i + 1) % dataSize;

    int x1 = x + (int)(i * xScale);
    int y1 = y + h - (int)((data[currentDataIdx] - minVal) * yScale);

    int x2 = x + (int)((i + 1) * xScale);
    int y2 = y + h - (int)((data[nextDataIdx] - minVal) * yScale);

    lcd.drawLine(x1, y1, x2, y2, lineColor);
  }
}
