#include <Wire.h> // For I2C communication

// Include the CYD Display Configuration.
// This file initializes the LovyanGFX LGFX class for the screen and touchscreen,
// and may define I2C pins (extSDA, extSCL).
#include <CYD_Display_Config.h>

// Define I2C pins if they were not defined by CYD_Display_Config.h
// These are common default pins for external I2C devices on ESP32 CYD boards.
#ifndef extSDA
  #define extSDA 22
#endif
#ifndef extSCL
  #define extSCL 27
#endif

// External Libraries:
// 1. Adafruit BMP280 Library
//    Install via Arduino Library Manager: Search for "Adafruit BMP280 Library"
#include <Adafruit_BMP280.h>

// 2. Adafruit AHTX0 Library
//    Install via Arduino Library Manager: Search for "Adafruit AHTX0 Library"
#include <Adafruit_AHTX0.h>

// LovyanGFX display object.
// The CYD_Display_Config.h file should have configured this 'lcd' object
// with the correct display and touch controller settings for your CYD board.
LGFX lcd;

// Sensor Objects
Adafruit_BMP280 bmp; // I2C BMP280 sensor object. Uses the global 'Wire' object by default.
Adafruit_AHTX0 aht;  // I2C AHT20 sensor object

// I2C Sensor Addresses
#define BMP280_I2C_ADDRESS 0x77 // Default I2C address for BMP280
#define AHT20_I2C_ADDRESS  0x38 // Default I2C address for AHT20

// --- Display Configuration ---
// Assuming a common CYD screen resolution. Adjust if your CYD has a different size.
const int screenWidth = 320;
const int screenHeight = 240;

// Custom colors for better readability
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

// --- Data Logging for Graphs (24 hours history) ---
// We'll store data points every 10 minutes for 24 hours.
// Total points = 24 hours * (60 minutes / 10 minutes) = 24 * 6 = 144 points.
const int HISTORY_SIZE = 144;
float tempHistory[HISTORY_SIZE];     // Array to store temperature history
float humidHistory[HISTORY_SIZE];    // Array to store humidity history
float pressureHistory[HISTORY_SIZE]; // Array to store pressure history
int historyIndex = 0;                // Current index in the circular history arrays

// Time interval for sensor measurements and data logging (10 minutes)
const unsigned long MEASUREMENT_INTERVAL_MS = 10UL * 60UL * 1000UL;
unsigned long lastMeasurementTime = 0; // Stores the time of the last measurement

// --- Function Prototypes ---
void updateSensorData();
void initializeHistory();
void drawMainDisplay();
void drawGraph(int x, int y, int w, int h, const char* title, float* data, int dataSize, float minVal, float maxVal, uint32_t lineColor, uint32_t textColor);


void setup() {
  Serial.begin(115200);
  Serial.println("Starting CYD Sensor Display...");

  // --- Display Initialization ---
  lcd.init();         // Initialize LovyanGFX display
  lcd.setRotation(1); // Set display rotation (0=Portrait, 1=Landscape with buttons right, 2=Portrait, 3=Landscape with buttons left)
                      // Adjust 'setRotation()' value based on your CYD's physical orientation.
  lcd.setBrightness(128); // Set backlight brightness (0-255)
  lcd.fillScreen(TFT_BLACK); // Clear the entire screen to black

  // --- I2C Initialization ---
  // Initialize I2C bus with the specified SDA and SCL pins.
  Wire.begin(extSDA, extSCL);
  Serial.print("I2C initialized on SDA: "); Serial.print(extSDA); Serial.print(", SCL: "); Serial.println(extSCL);

  // --- BMP280 Sensor Initialization ---
  Serial.print("Initializing BMP280...");
  // The Adafruit_BMP280 object 'bmp' uses the global 'Wire' object by default.
  // After 'Wire.begin()' is called, we only need to pass the I2C address to 'bmp.begin()'.
  // The second argument of 'bmp.begin()' is for an optional chip ID, not a TwoWire object pointer.
  if (!bmp.begin(BMP280_I2C_ADDRESS)) { // Corrected: Removed '&Wire' from here
    Serial.println("ERROR: Could not find a valid BMP280 sensor, check wiring or address!");
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("BMP280 Error!", screenWidth / 2, screenHeight / 2);
    while (1); // Halt execution if sensor not found
  }
  Serial.println("BMP280 initialized.");
  // Configure BMP280 sensor settings for optimal readings
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode: Normal
                  Adafruit_BMP280::SAMPLING_X16,         // Corrected: Use SAMPLING_X16 for Temperature oversampling
                  Adafruit_BMP280::SAMPLING_X16,            // Corrected: Use SAMPLING_X16 for Pressure oversampling
                  Adafruit_BMP280::STANDBY_MS_500,   // Standby time: 500ms
                  Adafruit_BMP280::FILTER_X16,       // IIR filter sampling: x16
                  Adafruit_BMP280::SPI_3_WIRE);       // Corrected: Use SPI_3_WIRE (enum for 3-wire SPI, irrelevant for I2C but required for correct compilation)

  // --- AHT20 Sensor Initialization ---
  Serial.print("Initializing AHT20...");
  // The AHT20 begin method correctly accepts a TwoWire* argument if a non-default bus is used.
  if (!aht.begin(&Wire)) {
    Serial.println("ERROR: Could not find AHT20 sensor, check wiring or address!");
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("AHT20 Error!", screenWidth / 2, screenHeight / 2 + 20);
    while (1); // Halt execution if sensor not found
  }
  Serial.println("AHT20 initialized.");

  // Initialize history arrays with current sensor data to avoid empty graphs at start
  initializeHistory();
  updateSensorData(); // Take an initial reading after history is filled

  lastMeasurementTime = millis(); // Set initial time for the next measurement
}

void loop() {
  // Check if the measurement interval has passed to take new readings and update history
  if (millis() - lastMeasurementTime >= MEASUREMENT_INTERVAL_MS) {
    updateSensorData();          // Read sensors and update history arrays
    lastMeasurementTime = millis(); // Reset timer for the next interval
  }

  // Draw or refresh the display with current values and graphs
  drawMainDisplay();

  // Small delay to prevent too rapid screen redrawing, which saves CPU cycles
  // and makes updates visually smoother.
  delay(500);
}

/**
 * @brief Reads temperature, humidity, and pressure from sensors and updates
 *        the circular history buffers.
 */
void updateSensorData() {
  sensors_event_t humidity, temp_aht; // Objects to hold AHT20 sensor data

  // Read data from BMP280
  float currentTemp = bmp.readTemperature();
  float currentPressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa

  // Read data from AHT20
  aht.getEvent(&humidity, &temp_aht); // Populate humidity and temperature objects
  float currentHumidity = humidity.relative_humidity;
  // AHT20 also provides temperature, but BMP280 is generally used for Temp+Pressure,
  // while AHT20 is focused on Humidity (and provides temperature as a secondary value).
  // If you prefer AHT20's temperature: float currentTemp_aht = temp_aht.temperature;

  // Store current sensor data into the history arrays at the current index.
  tempHistory[historyIndex] = currentTemp;
  humidHistory[historyIndex] = currentHumidity;
  pressureHistory[historyIndex] = currentPressure;

  // Advance the history index, wrapping around to the beginning if it reaches the end.
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;

  Serial.printf("New Data -> Temp: %.2fC, Humid: %.2f%%, Pressure: %.2fhPa\n",
                currentTemp, currentHumidity, currentPressure);
}

/**
 * @brief Fills the history arrays with an initial sensor reading.
 *        This prevents empty graphs when the program starts.
 */
void initializeHistory() {
  // Take an initial sensor reading
  sensors_event_t humidity, temp_aht;
  float initialTemp = bmp.readTemperature();
  float initialPressure = bmp.readPressure() / 100.0F;
  aht.getEvent(&humidity, &temp_aht);
  float initialHumidity = humidity.relative_humidity;

  // Fill all entries in the history arrays with this initial reading
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
  // Clear the top third of the screen where current values are displayed
  lcd.fillRect(0, 0, screenWidth, screenHeight / 3, TFT_BLACK);
  // Set text datum to middle-center for easy positioning of value text
  lcd.setTextDatum(MC_DATUM);

  // --- Display Current Values ---
  // Retrieve the most recent values from the history (the one written just before the current historyIndex)
  float currentTemp = tempHistory[(historyIndex + HISTORY_SIZE - 1) % HISTORY_SIZE];
  float currentHumidity = humidHistory[(historyIndex + HISTORY_SIZE - 1) % HISTORY_SIZE];
  float currentPressure = pressureHistory[(historyIndex + HISTORY_SIZE - 1) % HISTORY_SIZE];

  // Display Temperature
  lcd.setFont(&FreeSansBold24pt7b); // Use a large, bold font for values
  lcd.setTextColor(TFT_RED, TFT_BLACK); // Red text on black background
  lcd.drawString("Temp:", screenWidth / 6, screenHeight / 10);
  lcd.drawString(String(currentTemp, 1) + " C", screenWidth / 6, screenHeight / 4); // 1 decimal place

  // Display Humidity
  lcd.setTextColor(TFT_CYAN, TFT_BLACK); // Cyan text on black background
  lcd.drawString("Humid:", screenWidth / 2, screenHeight / 10);
  lcd.drawString(String(currentHumidity, 0) + " %", screenWidth / 2, screenHeight / 4); // 0 decimal places

  // Display Pressure
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK); // Yellow text on black background
  lcd.drawString("Press:", screenWidth * 5 / 6, screenHeight / 10);
  lcd.drawString(String(currentPressure, 0) + " hPa", screenWidth * 5 / 6, screenHeight / 4); // 0 decimal places

  // --- Graph Area Configuration ---
  const int graphYStart = screenHeight / 3 + 10; // Start graphs below the value display area, with some padding
  const int graphPadding = 5;                    // Vertical padding between graphs
  const int graphHeight = (screenHeight - graphYStart - (graphPadding * 2) - 10) / 3; // Divide remaining space into 3 for graphs
  const int graphWidth = screenWidth - 20;       // Graph width with 10px padding on each side
  const int graphX = 10;                         // X-coordinate for the left edge of graphs

  // Calculate dynamic min/max values for each graph based on its current 24-hour history.
  // This ensures the graph adapts to the actual range of values over time.
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

  // Add a small buffer to min/max values to prevent graph lines from touching the edges,
  // and to provide better visual stability.
  maxTemp += 1.0; minTemp -= 1.0; // 1 degree C buffer
  maxHumid += 2.0; minHumid -= 2.0; // 2% buffer, ensuring 0-100 range stays reasonable
  maxPressure += 2.0; minPressure -= 2.0; // 2 hPa buffer

  // Ensure that the min and max values are not identical, to prevent division by zero in scaling
  // and to ensure a visible graph range even if data is constant.
  if (fabs(maxTemp - minTemp) < 0.1) { maxTemp += 0.5; minTemp -= 0.5; }
  if (fabs(maxHumid - minHumid) < 0.1) { maxHumid += 1.0; minHumid -= 1.0; }
  if (fabs(maxPressure - minPressure) < 0.1) { maxPressure += 1.0; minPressure -= 1.0; }

  // --- Draw individual graphs ---
  // Temperature Graph
  drawGraph(graphX, graphYStart, graphWidth, graphHeight, "Temperature (24h)", tempHistory, HISTORY_SIZE, minTemp, maxTemp, TFT_RED, TFT_WHITE);

  // Humidity Graph
  drawGraph(graphX, graphYStart + graphHeight + graphPadding, graphWidth, graphHeight, "Humidity (24h)", humidHistory, HISTORY_SIZE, minHumid, maxHumid, TFT_CYAN, TFT_WHITE);

  // Pressure Graph
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
  // Clear the background of the graph area and draw a border
  lcd.fillRect(x, y, w, h, TFT_DARKGREY); // Dark grey background
  lcd.drawRect(x, y, w, h, TFT_WHITE);   // White border around the graph

  // Set a smaller font for graph titles and labels
  lcd.setFont(&FreeSansBold9pt7b);
  lcd.setTextDatum(TL_DATUM); // Set text datum to Top-Left for easy positioning

  // Draw the graph title
  lcd.setTextColor(textColor, TFT_DARKGREY); // Text color on graph background
  lcd.drawString(title, x + 5, y + 2);

  // Draw Y-axis min/max labels
  lcd.setTextDatum(TR_DATUM); // Set text datum to Top-Right for right-aligned text
  lcd.drawString(String(maxVal, 0), x + w - 5, y + 2);     // Max value at top-right
  lcd.drawString(String(minVal, 0), x + w - 5, y + h - 12); // Min value at bottom-right (adjusted for font height)

  lcd.setTextDatum(TL_DATUM); // Reset text datum to Top-Left for subsequent drawing

  // Calculate scaling factors for mapping data values to screen pixels
  float valueRange = maxVal - minVal;
  if (valueRange == 0) valueRange = 1.0; // Prevent division by zero if minVal == maxVal

  float xScale = (float)w / (dataSize - 1); // X-scale: width per data point
  float yScale = (float)h / valueRange;     // Y-scale: height per value unit

  // Plot the historical data
  // The 'historyIndex' points to the *next* write location.
  // To plot from oldest to newest data (left to right), we need to start
  // from 'historyIndex' and wrap around, treating it as a circular buffer.
  for (int i = 0; i < dataSize - 1; i++) {
    // Determine the current and next data points in the circular buffer
    int currentDataIdx = (historyIndex + i) % dataSize;
    int nextDataIdx = (historyIndex + i + 1) % dataSize;

    // Convert data values to X, Y screen coordinates
    // X-coordinate: linearly spaced from left (x) to right (x + w)
    // Y-coordinate: scaled from valueRange to graph height,
    //               and inverted because screen Y increases downwards.
    int x1 = x + (int)(i * xScale);
    int y1 = y + h - (int)((data[currentDataIdx] - minVal) * yScale);

    int x2 = x + (int)((i + 1) * xScale);
    int y2 = y + h - (int)((data[nextDataIdx] - minVal) * yScale);

    // Draw a line segment between the current and next data points
    lcd.drawLine(x1, y1, x2, y2, lineColor);
  }
}
