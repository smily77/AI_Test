// Libraries required:
// - LovyanGFX (installed via Arduino Library Manager, search for "LovyanGFX")
// - NTPClient (installed via Arduino Library Manager, search for "NTPClient")

// Include necessary libraries
#include <WiFi.h>
#include <NTPClient.h>
#include <CYD_Display_Config.h> // Includes LovyanGFX library and configuration specific to the CYD board
#include <Credentials.h>        // User-defined file containing WiFi credentials:
                                // const char* ssid = "your_SSID";
                                // const char* password = "your_PASSWORD";
#include <algorithm>            // Required for std::min (though often included indirectly)

// --- LovyanGFX Display Setup ---
// Create an instance of the LGFX_Device class for the display,
// automatically configured by CYD_Display_Config.h for the CYD 2.8" board.
lgfx::LGFX_Device display;

// --- WiFi and NTP Client Setup ---
// UDP client for NTP (Network Time Protocol)
WiFiUDP ntpUDP;

// NTP client configured for Central European Time (CET) / Central European Summer Time (CEST)
// Offset is set to +1 hour (3600 seconds) from UTC.
// For CEST (+2 hours), the NTPClient handles the daylight saving automatically if the
// timezone offset is correctly set to the standard offset and the server provides UTC.
// However, if the time is directly applied without further timezone rules,
// you might need to manually adjust this offset for summer/winter time.
// For simplicity, 3600 (CET) is used as a base.
// Update interval of 60 seconds (60000 milliseconds) to prevent excessive NTP requests.
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

/**
 * @brief Connects to the configured WiFi network.
 *        Blocks until a connection is established.
 */
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Arduino setup function.
 *        Initializes Serial, display, WiFi, and NTP client.
 */
void setup() {
  Serial.begin(115200);

  // Initialize LovyanGFX display.
  // It's good practice to check if the initialization was successful.
  if (!display.begin()) {
    Serial.println("Error: Display initialization failed! Please check hardware connections and CYD_Display_Config.h.");
    // Halt execution if the display cannot be initialized.
    while (true) {
      delay(100);
    }
  }

  // Explicitly turn on the backlight.
  // The CYD_Display_Config.h typically defines TFT_BL (GPIO32) and configures LovyanGFX to control it via PWM.
  // However, sometimes explicit digital control is needed to ensure the backlight is activated,
  // especially if there are timing issues or specific board variations.
  #ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Set backlight to high (full brightness)
    Serial.println("Backlight activated.");
  #else
    Serial.println("Warning: TFT_BL not defined. Cannot explicitly control backlight.");
  #endif

  // Set rotation to 1 for landscape orientation.
  // (0=portrait, 1=landscape, 2=inverted portrait, 3=inverted landscape)
  display.setRotation(1);
  // Clear the entire screen to black before drawing anything
  display.fillScreen(TFT_BLACK);

  // Connect to the WiFi network
  connectWiFi();

  // Initialize the NTP client
  timeClient.begin();
  // Ensure the first time update is successful before proceeding
  while (!timeClient.update()) {
    Serial.println("Failed to get time from NTP server, retrying...");
    delay(1500); // Wait a bit before trying again
  }
  Serial.println("NTP time synchronized.");

  // Set default text properties for the time display
  display.setTextColor(TFT_WHITE, TFT_BLACK); // White text on a black background
  // Use a nice, bold font. FreeSansBold24pt7b is a good choice for "beautiful numbers".
  // This font uses bitmap data for fast rendering and good appearance.
  display.setFont(&fonts::FreeSansBold24pt7b);
  // Set text datum to Middle Center for easy centering of the text on the screen.
  display.setTextDatum(MC_DATUM);
}

/**
 * @brief Arduino loop function.
 *        Continuously updates and displays the current time.
 */
void loop() {
  // Update the NTP client. This will query the NTP server only at the specified interval (60 seconds).
  timeClient.update();

  // Get the formatted time string (e.g., "HH:MM:SS")
  String formattedTime = timeClient.getFormattedTime();

  // --- Dynamic Text Sizing for Full Screen Usage ---
  // To make the numbers fill as much of the screen as possible while maintaining aspect ratio:

  // 1. Measure the default size of the text string with the chosen font (scale 1)
  display.setTextSize(1); // Temporarily set base scale to 1 for accurate measurement
  int defaultTextWidth = display.textWidth("HH:MM:SS"); // Measure a typical time string
  int defaultTextHeight = display.fontHeight();

  // 2. Calculate individual scale factors for width and height
  // Determine how much we need to scale the text to fit the screen width/height.
  // A small margin (e.g., 0.95) is added to ensure it doesn't touch the edges.
  float scaleX = (float)display.width() / (float)defaultTextWidth;
  float scaleY = (float)display.height() / (float)defaultTextHeight;

  // 3. Use the smaller of the two scale factors to ensure the text fits entirely within the screen
  // This prevents the text from being cut off on either dimension.
  float finalScale = std::min(scaleX, scaleY) * 0.95; // 0.95 for a 5% margin

  // 4. Apply the calculated scale to the display.
  // For bitmap fonts like FreeSansBold24pt7b, if finalScale evaluates to < 1.0,
  // LovyanGFX might internally round it to 1 or even 0. Ensure it's at least 1.
  if (finalScale < 1.0f) finalScale = 1.0f;
  display.setTextSize(finalScale);

  // --- Displaying the Time ---
  // Draw the time string centered on the screen.
  // The setTextDatum(MC_DATUM) configured in setup() ensures that
  // display.drawString(text, x, y) centers the text around (x,y).
  // The background color set by display.setTextColor() will automatically clear
  // the area behind the text, effectively overwriting the previous time.
  display.drawString(formattedTime, display.width() / 2, display.height() / 2);

  // Output the time to Serial Monitor for debugging
  Serial.println(formattedTime);

  // Wait for 1 second before the next update to avoid rapid flickering and save CPU cycles.
  // The NTP client update interval is independent of this delay.
  delay(1000);
}
