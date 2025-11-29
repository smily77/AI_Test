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
// GPS TX (data output) -> ESP32 RX (Pin 8)
// GPS RX (data input) <- ESP32 TX (Pin 7, as updated)
#define GPS_RX_PIN 8
#define GPS_TX_PIN 7
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
  Serial.printf("Initializing GPS (Serial1 on RX P%d, TX P%d with %d Baud)...\n", GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE);
  GPS_Serial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("OK");

  // Initialize the TFT display
  Serial.print("Initializing TFT Display...");
  lcd.init();
  // Set to portrait mode.
  // 0 (portrait), 1 (landscape), 2 (inverted portrait), 3 (inverted landscape).
  lcd.setRotation(0); // Changed to portrait mode as requested
  lcd.fillScreen(TFT_BG_COLOR); // Clear screen with black
  lcd.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR); // Set text color to green with black background
  lcd.setTextSize(1); // Changed to text size 1 for initial messages to fit portrait width
  Serial.println("OK");

  // Calculate compass drawing parameters based on display dimensions
  compassCenterX = lcd.width() / 2;
  compassCenterY = lcd.height() / 2;
  compassRadius = min(lcd.width(), lcd.height()) / 2 - 20; // 20 pixel margin
  pointerLength = compassRadius - 10; // Pointers are slightly shorter than the compass radius

  // Display initial messages on TFT, adjusted for setTextSize(1) and portrait layout
  int lineHeight = lcd.fontHeight() + 5; // Height of text + a small margin
  lcd.setCursor(5, 5);
  lcd.println("Initializing sensors...");
  lcd.setCursor(5, 5 + lineHeight);
  lcd.println("Waiting for BNO calibration...");
  lcd.setCursor(5, 5 + 2 * lineHeight);
  lcd.println("Waiting for GPS fix...");

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
  bno_heading = fmod((180.0 - bno_heading), 360.0); // Corrected line
  if (bno_heading < 0) bno_heading += 360.0; // Ensure positive angle [0, 360)

  // Get BNO055 calibration status
  // Sys: System, Gyro: Gyroscope, Accel: Accelerometer, Mag: Magnetometer
  // Calibration levels range from 0 (uncalibrated) to 3 (fully calibrated)
  uint8_t sys, gyro, accel, mag;
  bno.getCalibration(&sys, &gyro, &accel, &mag);

  // --- Display Update ---
  // Clear the entire screen for each update to prevent ghosting of old pointers/text.
  lcd.fillRect(0, 0, lcd.width(), lcd.height(), TFT_BG_COLOR); // Clear entire screen

  // Draw Compass Rose
  lcd.drawCircle(compassCenterX, compassCenterY, compassRadius, TFT_COMPASS_COLOR);

  // Draw cardinal points (N, E, S, W)
  // Using setTextSize(1) for cardinal points to fit portrait width and match status text.
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_COMPASS_COLOR);
  int charHeight1 = lcd.fontHeight(); // Get height of current font (size 1)
  int cardinalMargin = 3; // Small margin for cardinal points from the circle edge

  // N: Centered above the circle
  int nWidth = lcd.textWidth("N");
  lcd.setCursor(compassCenterX - nWidth / 2, compassCenterY - compassRadius - charHeight1 - cardinalMargin); lcd.print("N");
  // E: Right of the circle, vertically centered
  int eWidth = lcd.textWidth("E");
  lcd.setCursor(compassCenterX + compassRadius + cardinalMargin, compassCenterY - charHeight1 / 2); lcd.print("E");
  // S: Centered below the circle
  int sWidth = lcd.textWidth("S");
  lcd.setCursor(compassCenterX - sWidth / 2, compassCenterY + compassRadius + cardinalMargin); lcd.print("S");
  // W: Left of the circle, vertically centered
  int wWidth = lcd.textWidth("W");
  lcd.setCursor(compassCenterX - compassRadius - wWidth - cardinalMargin, compassCenterY - charHeight1 / 2); lcd.print("W");

  // --- Draw BNO055 Pointer ---
  // Convert BNO heading to radians.
  // x = centerX + length * sin(radians(angle))
  // y = centerY - length * cos(radians(angle)) (Y-axis typically increases downwards on display)
  float bno_rad = bno_heading * PI / 180.0;
  int bno_endX = compassCenterX + (int)(pointerLength * sin(bno_rad));
  int bno_endY = compassCenterY - (int)(pointerLength * cos(bno_rad));
  lcd.drawLine(compassCenterX, compassCenterY, bno_endX, bno_endY, TFT_BNO_COLOR);
  lcd.fillCircle(bno_endX, bno_endY, 5, TFT_BNO_COLOR); // Small circle at pointer tip

  // Label for BNO pointer
  lcd.setTextSize(1); // Already size 1
  lcd.setTextColor(TFT_BNO_COLOR);
  // Adjust label position dynamically based on pointer direction
  int bnoLabelWidth = lcd.textWidth("BNO");
  lcd.setCursor(bno_endX + (bno_endX > compassCenterX ? 5 : -(bnoLabelWidth + 5)), bno_endY);
  lcd.print("BNO");

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
    lcd.drawLine(compassCenterX, compassCenterY, gps_endX, gps_endY, TFT_GPS_COLOR);
    lcd.fillCircle(gps_endX, gps_endY, 5, TFT_GPS_COLOR); // Small circle at pointer tip

    // Label for GPS pointer
    lcd.setTextSize(1); // Already size 1
    lcd.setTextColor(TFT_GPS_COLOR);
    // Adjust label position dynamically based on pointer direction
    int gpsLabelWidth = lcd.textWidth("GPS");
    lcd.setCursor(gps_endX + (gps_endX > compassCenterX ? 5 : -(gpsLabelWidth + 5)), gps_endY + 10); // Offset Y slightly
    lcd.print("GPS");
  }

  // --- Display Status Information ---
  lcd.setTextSize(1); // All status messages use size 1 to fit portrait width
  lcd.setTextColor(TFT_TEXT_COLOR);
  int statusLineHeight = lcd.fontHeight() + 2; // Height of text + small inter-line margin
  int bottomMargin = 5; // Margin from the bottom of the screen

  // BNO055 Calibration Status (top section)
  lcd.setCursor(5, 5); // First line from the top
  lcd.printf("BNO Cal: S:%d G:%d A:%d M:%d  ", sys, gyro, accel, mag); // The spaces help clear previous text

  // GPS Fix Status and Satellites (second line from top)
  lcd.setCursor(5, 5 + statusLineHeight);
  if (gps.location.isValid()) {
    lcd.printf("GPS Fix: YES (%d Sats)  ", gps.satellites.value());
  } else {
    lcd.printf("GPS Fix: NO (%d Sats)    ", gps.satellites.value());
  }

  // Display current headings for numerical comparison (bottom section)
  // GPS Heading at the very bottom
  lcd.setCursor(5, lcd.height() - (charHeight1 + bottomMargin)); // Position relative to bottom
  if (gps.course.isValid()) {
    lcd.printf("GPS Hdg: %0.1f deg  ", gps_heading);
  } else {
    lcd.printf("GPS Hdg: --- deg    ");
  }

  // BNO Heading above GPS Heading
  lcd.setCursor(5, lcd.height() - 2 * (charHeight1 + bottomMargin)); // Two lines up from bottom
  lcd.printf("BNO Hdg: %0.1f deg  ", bno_heading);

  delay(100); // Update display approximately every 100ms
}
