// Required Libraries:
// 1. LovyanGFX: Search in Arduino Library Manager for "LovyanGFX" by lovyan03.
//    Install version 1.1.9 or later. This library handles the TFT display and touch.
// 2. TinyGPSPlus: Search in Arduino Library Manager for "TinyGPSPlus" by Mikal Hart.
//    Install the latest version. This library parses NMEA sentences from the GPS module.

// Include the specific configuration for the ESP32-S3 display.
// This file is expected to set up the LovyanGFX library for your board's specific TFT and touch.
#include <CYD_Display_Config.h>

// Include the core LovyanGFX library. CYD_Display_Config.h sets up the specific driver details.
#include <LovyanGFX.hpp>

// Include the TinyGPSPlus library for parsing GPS data.
#include <TinyGPSPlus.h>

// Define the LGFX object as requested.
// CYD_Display_Config.h should have prepared the necessary configurations for this instance.
LGFX lcd; 

// Define the RX pin for the GPS module.
// The Quectel L80-R GPS module will send data to this pin.
// For ESP32-S3, hardware serial ports (Serial1, Serial2) can have their pins remapped.
#define GPS_RX_PIN 8 

// Define the baud rate for the GPS module.
#define GPS_BAUD_RATE 9600

// Create a TinyGPSPlus object to handle GPS data parsing.
TinyGPSPlus gps;

// Create a HardwareSerial object pointer for the GPS module.
// We'll use Serial1, which can be remapped to any GPIO pins on ESP32-S3.
HardwareSerial *gpsSerial = &Serial1;

// Variables to track the last display update time to control screen refresh rate.
unsigned long lastDisplayUpdateTime = 0;
const unsigned long displayUpdateInterval = 1000; // Update display every 1 second (1000 ms)

/**
 * @brief Displays GPS information on the TFT screen.
 *        Clears the screen and prints various GPS data fields.
 */
void displayGPSInfo() {
  lcd.fillScreen(TFT_BLACK); // Clear the screen for a fresh update
  lcd.setCursor(0, 0);       // Start drawing from the top-left corner

  // Set a larger font for the title
  lcd.setFont(&fonts::Font4); 
  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.println("  GPS Info");
  lcd.println("-----------");

  // Set a smaller font for detailed data
  lcd.setFont(&fonts::Font2); 
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  // Check if GPS has a valid location fix
  if (gps.location.isValid()) {
    lcd.print(" Lat: ");
    lcd.println(gps.location.lat(), 6); // Display latitude with 6 decimal places
    lcd.print(" Lng: ");
    lcd.println(gps.location.lng(), 6); // Display longitude with 6 decimal places

    lcd.print(" Alt: ");
    if (gps.altitude.isValid()) {
      lcd.print(gps.altitude.meters(), 1); // Display altitude in meters with 1 decimal place
      lcd.println(" m");
    } else {
      lcd.println("---"); // Indicate no valid altitude data
    }

    lcd.print(" Spd: ");
    if (gps.speed.isValid()) {
      lcd.print(gps.speed.kmph(), 1); // Display speed in kilometers per hour
      lcd.println(" km/h");
    } else {
      lcd.println("---"); // Indicate no valid speed data
    }

    lcd.print(" Crs: ");
    if (gps.course.isValid()) {
      lcd.print(gps.course.deg(), 1); // Display course in degrees
      lcd.println(" deg");
    } else {
      lcd.println("---"); // Indicate no valid course data
    }

    lcd.print(" Sat: ");
    if (gps.satellites.isValid()) {
      lcd.println(gps.satellites.value()); // Display number of satellites
    } else {
      lcd.println("---"); // Indicate no valid satellite count
    }

    lcd.print(" Fix: ");
    lcd.print(gps.sentencesWithFix()); // Display count of sentences with fix
    lcd.print("/");
    lcd.println(gps.charsProcessed()); // Display total characters processed

    lcd.print(" Date: ");
    if (gps.date.isValid()) {
      lcd.printf("%02d/%02d/%04d\n", gps.date.day(), gps.date.month(), gps.date.year()); // Format date
    } else {
      lcd.println("---"); // Indicate no valid date data
    }

    lcd.print(" Time: ");
    if (gps.time.isValid()) {
      lcd.printf("%02d:%02d:%02d\n", gps.time.hour(), gps.time.minute(), gps.time.second()); // Format time
    } else {
      lcd.println("---"); // Indicate no valid time data
    }

  } else {
    // If no valid GPS fix, display a waiting message and satellite count if available
    lcd.setFont(&fonts::Font4);
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.setCursor(0, lcd.fontHeight() * 3); // Position below "GPS Info"
    lcd.println("\n  Waiting for\n   GPS Fix...");
    
    // Display satellite count while waiting for a fix
    lcd.setFont(&fonts::Font2); // Revert to a smaller font for satellite info
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.print("\n  Sat: ");
    if (gps.satellites.isValid()) {
      lcd.println(gps.satellites.value()); // Display number of satellites
    } else {
      lcd.println("---"); // Indicate no valid satellite data yet
    }
    
    lcd.setFont(&fonts::Font4); // Revert back to larger font for "No valid data"
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.println("\n   No valid\n    data.");
  }
}

/**
 * @brief Arduino setup function. Runs once on startup.
 *        Initializes serial communication, TFT display, and GPS module.
 */
void setup() {
  // Initialize USB Serial for debugging output to the computer.
  Serial.begin(115200);
  Serial.println("ESP32-S3 GPS Display Initialization");

  // Initialize the TFT display using the configuration provided by CYD_Display_Config.h.
  // This call sets up the display hardware based on the board's specifics.
  lcd.init();

  // Set display rotation. Adjust the value (0, 1, 2, 3) as needed for your screen's orientation.
  // Value 0 typically provides a portrait mode suitable for text display.
  lcd.setRotation(0); 

  // Set the default font, text size, and colors for the display.
  lcd.setFont(&fonts::Font2);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK); // White text on black background

  // Fill the entire screen with a black background.
  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(0, 0); // Reset cursor to top-left
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.println("Display Initialized!");
  lcd.println("Starting GPS module...");
  delay(1000); // Short delay to show initialization messages

  // Initialize the hardware serial port (Serial1) for the GPS module.
  // SERIAL_8N1: 8 data bits, no parity, 1 stop bit (standard for GPS).
  // GPS_RX_PIN: The GPIO pin for receiving data from the GPS module.
  // -1: Indicates that the TX pin for Serial1 is not being explicitly remapped here (it's not used by GPS).
  gpsSerial->begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, -1);
  Serial.printf("GPS Serial on RX Pin %d at %d baud.\n", GPS_RX_PIN, GPS_BAUD_RATE);

  // Perform an initial display update to show "Waiting for GPS Fix..." or any initial data.
  displayGPSInfo(); 
}

/**
 * @brief Arduino loop function. Runs repeatedly after setup.
 *        Reads GPS data and updates the TFT display periodically.
 */
void loop() {
  // Continuously read any available bytes from the GPS serial port
  // and feed them into the TinyGPSPlus parser.
  while (gpsSerial->available()) {
    gps.encode(gpsSerial->read());
  }

  // Check if enough time has passed since the last display update.
  if (millis() - lastDisplayUpdateTime >= displayUpdateInterval) {
    lastDisplayUpdateTime = millis();

    // Call the function to update the information on the TFT screen.
    // This will redraw the screen with the latest GPS data or the "waiting" message.
    displayGPSInfo();
  }
}