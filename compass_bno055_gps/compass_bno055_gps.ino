// Required Libraries:
// 1. LovyanGFX: Used for the display. The CYD_Display_Config.h typically configures it.
//    Installation: Go to Arduino IDE -> Sketch -> Include Library -> Manage Libraries...
//    Search for "LovyanGFX" and install the latest version.
//    Ensure you have the 'CYD_Display_Config.h' file. This file customizes LovyanGFX for your specific ESP32-S3 TFT.
//    It should be placed in your sketch folder or be part of a board support package.
// 2. Adafruit BNO055: For the IMU sensor.
//    Installation: Go to Arduino IDE -> Sketch -> Include Library -> Manage Libraries...
//    Search for "Adafruit BNO055" and install it. Also install "Adafruit Unified Sensor" if prompted.
// 3. TinyGPSPlus: For parsing NMEA data from the GPS module.
//    Installation: Go to Arduino IDE -> Sketch -> Include Library -> Manage Libraries...
//    Search for "TinyGPSPlus" and install the latest version.

// Include necessary libraries
#include <Wire.h>          // For I2C communication with BNO055
#include <Adafruit_Sensor.h> // Base class for Adafruit sensors
#include <Adafruit_BNO055.h> // For the BNO055 IMU
#include <TinyGPSPlus.h>   // For GPS data parsing

// Include the display configuration for your specific ESP32-S3 TFT
// This header is expected to define and configure an LGFX object (e.g., LGFX lcd;)
#include <CYD_Display_Config.h>

// --- Pin and Hardware Definitions ---

// GPS Module: Quectel L80-R
// Connected to Hardware Serial 1 (Serial1) on ESP32-S3
// IMPORTANT: The definitions below (GPS_RX_PIN, GPS_TX_PIN) refer to the ESP32's GPIO numbers.
// The most common cause for "0 satellites" when hardware is believed functional is
// a confusion in connecting RX/TX lines. For this fix, we are assuming a "straight-through" wiring
// where the ESP32 pin *labeled* RX (e.g. on a breakout) might be connected to the GPS RX.
//
// To correct this common mistake:
//   - ESP32's RX (data input) should receive from the GPS module's TX (data output).
//   - ESP32's TX (data output) should transmit to the GPS module's RX (data input).
//
// The code below configures Serial1 such that:
//   ESP32's RX (UART1_RX) is assigned to GPIO 7 (GPS_TX_PIN)
//   ESP32's TX (UART1_TX) is assigned to GPIO 8 (GPS_RX_PIN)
//
// Therefore, ensure your physical wiring is:
//   GPS module's TX (data output) -> ESP32 GPIO 7
//   GPS module's RX (data input) <- ESP32 GPIO 8
#define GPS_RX_PIN 8 // This ESP32 pin is used as Serial1's TX (connected to GPS_RX)
#define GPS_TX_PIN 7 // This ESP32 pin is used as Serial1's RX (connected to GPS_TX)
#define GPS_BAUD_RATE 9600

// BNO055 IMU:
// I2C Address: 0x29 (as specified in the request)
// SDA Pin: 1 (as specified in the request)
// SCL Pin: 2 (as specified in the request)
#define BNO_I2C_ADDRESS 0x29
#define BNO_SDA_PIN 1
#define BNO_SCL_PIN 2

// --- Global Objects ---

// Display object (initialized by CYD_Display_Config.h, just declare it here)
LGFX lcd;

// Sprite object for flicker-free drawing, leveraging PSRAM if available
LGFX_Sprite sprite(&lcd);

// BNO055 sensor object
// The '55' is a sensor ID, can be any unique number.
Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO_I2C_ADDRESS);

// TinyGPSPlus object
TinyGPSPlus gps;

// HardwareSerial for GPS communication
HardwareSerial GPS_Serial(1); // Use Serial1, mapping to GPS_RX_PIN and GPS_TX_PIN

// --- Display Parameters ---
int compassCenterX;
int compassCenterY;
int compassRadius;
int pointerLength;

// Colors for display elements
#define TFT_BNO_COLOR TFT_RED
#define TFT_GPS_COLOR TFT_BLUE
#define TFT_COMPASS_COLOR TFT_WHITE
#define TFT_TEXT_COLOR TFT_GREEN
#define TFT_BG_COLOR TFT_BLACK

// --- Setup Function ---
void setup() {
  Serial.begin(115200); // Initialize serial for debugging
  while (!Serial); // Wait for Serial Monitor to open
  Serial.println("ESP32-S3 Compass & GPS Display Starting...");

  // Initialize I2C for BNO055
  // Note: For ESP32, Wire.begin(sda, scl) allows remapping I2C pins.
  Wire.begin(BNO_SDA_PIN, BNO_SCL_PIN);
  delay(10); // Small delay for I2C bus to stabilize

  // Initialize BNO055
  Serial.print("Initializing BNO055...");
  if (!bno.begin()) {
    Serial.println("Ooops, no BNO055 detected ... Check wiring or I2C address!");
    // Attempt to initialize display to show error message
    // This part still uses lcd directly as sprite might not be fully set up yet
    lcd.init();
    lcd.setRotation(0);
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_RED);
    lcd.setCursor(0, 0);
    lcd.println("BNO055 Error!");
    while (1); // Halt if BNO055 is not found
  }
  Serial.println("OK");
  bno.setExtCrystalUse(true); // Use external crystal for better accuracy

  // Set up HardwareSerial for GPS
  Serial.printf("Initializing GPS (Serial1 on RX P%d, TX P%d with %d Baud)...\n", GPS_TX_PIN, GPS_RX_PIN, GPS_BAUD_RATE); // Log new RX/TX assignments
  // FIX: Swapped GPS_RX_PIN and GPS_TX_PIN arguments.
  // This configures ESP32's GPIO7 as RX and GPIO8 as TX, addressing common wiring confusion.
  GPS_Serial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);
  Serial.println("OK");

  // Initialize the TFT display
  Serial.print("Initializing TFT Display...");
  lcd.init();
  // Set to portrait mode.
  // 0 (portrait), 1 (landscape), 2 (inverted portrait), 3 (inverted landscape).
  lcd.setRotation(0); // Set to portrait mode
  lcd.fillScreen(TFT_BG_COLOR); // Clear screen with black
  Serial.println("OK");

  // Initialize the sprite for flicker-free drawing
  // Create sprite with full screen dimensions. LovyanGFX will automatically try to use PSRAM
  // if LGFX_USE_PSRAM is defined in CYD_Display_Config.h or similar config.
  sprite.createSprite(lcd.width(), lcd.height());
  // The 'usePSRAM' method does not exist for LGFX_Sprite. PSRAM usage is typically configured
  // at a lower level or handled automatically by createSprite if PSRAM is enabled.
  // Removed: sprite.usePSRAM(true);
  sprite.fillScreen(TFT_BG_COLOR); // Clear sprite with background color

  // Set initial text properties for the sprite
  sprite.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR); // Set text color to green with black background
  sprite.setTextSize(2); // Changed to text size 2 for initial messages for better readability

  // Calculate compass drawing parameters based on display dimensions
  // The compass radius is reduced to make space for larger cardinal direction text (N, E, S, W).
  int estimatedCharHeightSize2 = sprite.fontHeight(); // Get height of current font (size 2) from sprite
  int minCompassMargin = estimatedCharHeightSize2 + 10; // Buffer for cardinal points + a small safety margin
  compassCenterX = lcd.width() / 2;
  compassCenterY = lcd.height() / 2;
  compassRadius = min(lcd.width(), lcd.height()) / 2 - minCompassMargin;
  pointerLength = compassRadius - 10; // Pointers are slightly shorter than the compass radius

  // Display initial messages on sprite, adjusted for setTextSize(2) and portrait layout
  int lineHeight = sprite.fontHeight() + 5; // Height of text (now size 2) + a small margin
  sprite.setCursor(5, 5);
  sprite.println("Initializing sensors...");
  sprite.setCursor(5, 5 + lineHeight);
  sprite.println("Waiting for BNO cal..."); // Shortened message to fit on smaller screens
  sprite.setCursor(5, 5 + 2 * lineHeight);
  sprite.println("Waiting for GPS fix...");
  sprite.pushSprite(0, 0); // Push the initial messages to the display

  delay(100); // Small delay before loop starts
}

// --- Loop Function ---
void loop() {
  // --- GPS Data Processing ---
  // Read any available data from the GPS module and feed it to TinyGPSPlus
  while (GPS_Serial.available() > 0) {
    gps.encode(GPS_Serial.read());
  }

  // --- BNO055 Data Reading ---
  sensors_event_t event;
  bno.getEvent(&event);

  float bno_heading = event.orientation.x; // Yaw (heading) is typically X-axis in Euler angles

  // Adjust BNO055 heading for upside-down mounting:
  // If the sensor is mounted upside down, its reported yaw will be 180 degrees off
  // AND its rotation direction will be reversed. The formula (180 - original_yaw)
  // correctly handles both the 180-degree offset and the reversal of direction.
  bno_heading = fmod((180.0 - bno_heading), 360.0);
  if (bno_heading < 0) bno_heading += 360.0; // Ensure positive angle [0, 360)

  // Get BNO055 calibration status
  // Sys: System, Gyro: Gyroscope, Accel: Accelerometer, Mag: Magnetometer
  // Calibration levels range from 0 (uncalibrated) to 3 (fully calibrated)
  uint8_t sys, gyro, accel, mag;
  bno.getCalibration(&sys, &gyro, &accel, &mag);

  // --- Display Update ---
  // Clear the entire sprite for each update to prevent ghosting of old pointers/text.
  sprite.fillScreen(TFT_BG_COLOR); // Clear entire sprite

  // Draw Compass Rose
  sprite.drawCircle(compassCenterX, compassCenterY, compassRadius, TFT_COMPASS_COLOR);

  // Draw cardinal points (N, E, S, W) - Text size increased for readability
  sprite.setTextSize(2);
  sprite.setTextColor(TFT_COMPASS_COLOR);
  int charHeight_size2 = sprite.fontHeight(); // Get height of current font (size 2)
  int cardinalMargin = 5; // Small margin for cardinal points from the circle edge

  // N: Centered above the circle
  int nWidth = sprite.textWidth("N");
  sprite.setCursor(compassCenterX - nWidth / 2, compassCenterY - compassRadius - charHeight_size2 - cardinalMargin); sprite.print("N");
  // E: Right of the circle, vertically centered
  int eWidth = sprite.textWidth("E");
  sprite.setCursor(compassCenterX + compassRadius + cardinalMargin, compassCenterY - charHeight_size2 / 2); sprite.print("E");
  // S: Centered below the circle
  int sWidth = sprite.textWidth("S");
  sprite.setCursor(compassCenterX - sWidth / 2, compassCenterY + compassRadius + cardinalMargin); sprite.print("S");
  // W: Left of the circle, vertically centered
  int wWidth = sprite.textWidth("W");
  sprite.setCursor(compassCenterX - compassRadius - wWidth - cardinalMargin, compassCenterY - charHeight_size2 / 2); sprite.print("W");

  // --- Draw BNO055 Pointer ---
  // Convert BNO heading to radians.
  // x = centerX + length * sin(radians(angle))
  // y = centerY - length * cos(radians(angle)) (Y-axis typically increases downwards on display)
  float bno_rad = bno_heading * PI / 180.0;
  int bno_endX = compassCenterX + (int)(pointerLength * sin(bno_rad));
  int bno_endY = compassCenterY - (int)(pointerLength * cos(bno_rad));
  sprite.drawLine(compassCenterX, compassCenterY, bno_endX, bno_endY, TFT_BNO_COLOR);
  sprite.fillCircle(bno_endX, bno_endY, 5, TFT_BNO_COLOR); // Small circle at pointer tip

  // Label for BNO pointer - Text size increased for readability
  sprite.setTextSize(2);
  sprite.setTextColor(TFT_BNO_COLOR);
  // Adjust label position dynamically based on pointer direction
  int bnoLabelWidth = sprite.textWidth("BNO");
  sprite.setCursor(bno_endX + (bno_endX > compassCenterX ? 8 : -(bnoLabelWidth + 8)), bno_endY); // Adjusted offset
  sprite.print("BNO");

  // --- Draw GPS Pointer ---
  float gps_heading = 0.0;
  if (gps.course.isValid()) { // Check if GPS has a valid course (heading)
    gps_heading = gps.course.deg();
  }

  // Only draw GPS pointer if it has a valid course and fix
  if (gps.location.isValid() && gps.course.isValid()) {
    float gps_rad = gps_heading * PI / 180.0;
    // Make GPS pointer slightly shorter for visual distinction
    int gps_endX = compassCenterX + (int)((pointerLength - 10) * sin(gps_rad));
    int gps_endY = compassCenterY - (int)((pointerLength - 10) * cos(gps_rad));
    sprite.drawLine(compassCenterX, compassCenterY, gps_endX, gps_endY, TFT_GPS_COLOR);
    sprite.fillCircle(gps_endX, gps_endY, 5, TFT_GPS_COLOR); // Small circle at pointer tip

    // Label for GPS pointer - Text size increased for readability
    sprite.setTextSize(2);
    sprite.setTextColor(TFT_GPS_COLOR);
    // Adjust label position dynamically based on pointer direction
    int gpsLabelWidth = sprite.textWidth("GPS");
    sprite.setCursor(gps_endX + (gps_endX > compassCenterX ? 8 : -(gpsLabelWidth + 8)), gps_endY + 15); // Adjusted offset
    sprite.print("GPS");
  }

  // --- Display Status Information ---
  // BNO055 Calibration Status (top section) - MODIFIED to setTextSize(2) and shortened string to fit line length
  sprite.setTextSize(2); // Increased text size as requested
  sprite.setTextColor(TFT_TEXT_COLOR);
  int statusLineHeight_size2 = sprite.fontHeight() + 2; // Height for size 2 text + small inter-line margin
  sprite.setCursor(5, 5); // First line from the top
  sprite.printf("BNO:S%d G%d A%d M%d  ", sys, gyro, accel, mag); // Shortened string for setTextSize(2)

  // GPS Fix Status and Satellites (second line from top) - MODIFIED to setTextSize(2) and shortened string to fit line length
  sprite.setCursor(5, 5 + statusLineHeight_size2);
  if (gps.location.isValid()) {
    sprite.printf("GPS:FIX (%dSats)  ", gps.satellites.value()); // Shortened string for setTextSize(2)
  } else {
    sprite.printf("GPS:NOFIX (%dSats) ", gps.satellites.value()); // Shortened string for setTextSize(2)
  }
  // Display current headings for numerical comparison (bottom section)
  // BNO Heading and GPS Heading will use setTextSize(2) for better readability
  sprite.setTextSize(2); // Text size remains 2 as it was already
  sprite.setTextColor(TFT_TEXT_COLOR);
  int charHeight_size2_for_status = sprite.fontHeight(); // Height for size 2 text
  int bottomMargin = 5; // Margin from the bottom of the screen

  // GPS Heading at the very bottom
  sprite.setCursor(5, sprite.height() - (charHeight_size2_for_status + bottomMargin)); // Position relative to bottom
  if (gps.course.isValid()) {
    sprite.printf("GPS Hdg: %0.1f deg  ", gps_heading);
  } else {
    sprite.printf("GPS Hdg: --- deg    ");
  }

  // BNO Heading above GPS Heading
  sprite.setCursor(5, sprite.height() - 2 * (charHeight_size2_for_status + bottomMargin)); // Two lines up from bottom
  sprite.printf("BNO Hdg: %0.1f deg  ", bno_heading);

  // Push the entire sprite to the actual display, updating the screen in one go
  sprite.pushSprite(0, 0);

  delay(100); // Update display approximately every 100ms
}
