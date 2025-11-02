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

// --- IMPORTANT: Include the specific font header for FreeSansBold72pt7b ---
// This font is typically provided within the LovyanGFX library's "src/lgfx_fonts" directory.
// The include path needs to match the actual directory structure.
#include <lgfx_fonts/lgfx_font_FreeSansBold72pt7b.h>

// --- LovyanGFX Display Setup ---
// Create an instance of the LGFX_Device class for the display,
// automatically configured by CYD_Display_Config.h for the CYD 2.8" board.
LGFX display;

// --- WiFi and NTP Client Setup ---
// UDP client for NTP (Network Time Protocol)
WiFiUDP ntpUDP;

// NTP client configured for Central European Time (CET) / Central European Summer Time (CEST)
// Offset is set to +1 hour (3600 seconds) from UTC.
// For simplicity, 3600 (CET) is used as a base.
// Update interval of 60 seconds (60000 milliseconds) to prevent excessive NTP requests.
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

// --- LDR (Light Dependent Resistor) Setup ---
// On CYD boards, the LDR is typically connected to GPIO1 (ADC1_CHANNEL0)
const int LDR_PIN = 1;

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
 *        Initializes Serial, display, WiFi, NTP client, and LDR pin.
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

  // Set rotation to 3 for inverted landscape orientation (buttons on top).
  // (0=portrait, 1=landscape, 2=inverted portrait, 3=inverted landscape)
  display.setRotation(3);
  // Clear the entire screen to black before drawing anything
  display.fillScreen(TFT_BLACK);

  // Initialize LDR pin.
  pinMode(LDR_PIN, INPUT);
  // Set an initial medium brightness (0-255).
  display.setBrightness(128);

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
  // Changed text color to yellow as requested
  display.setTextColor(TFT_YELLOW, TFT_BLACK); // Yellow text on a black background
  // Use a much larger, bold font for better visibility and "straight edges".
  // FreeSansBold72pt7b is a good choice for large, clear numbers.
  display.setFont(&lgfx_font_FreeSansBold72pt7b);
  // Set text datum to Middle Center for easy centering of the text on the screen.
  display.setTextDatum(MC_DATUM);
}

// Variable to track the last time brightness was updated
static unsigned long lastBrightnessUpdate = 0;
const long brightnessUpdateInterval = 5000; // Update brightness every 5 seconds

/**
 * @brief Arduino loop function.
 *        Continuously updates and displays the current time and adjusts backlight brightness.
 */
void loop() {
  // Update the NTP client. This will query the NTP server only at the specified interval (60 seconds).
  timeClient.update();

  // --- Format Time (HH:MM) ---
  // Get hours and minutes and format them into a string without seconds as requested.
  char timeBuffer[6]; // "HH:MM\0" needs 6 characters
  sprintf(timeBuffer, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
  String formattedTime = timeBuffer;

  // --- Dynamic Text Sizing for Full Screen Usage ---
  // To make the numbers fill as much of the screen as possible while maintaining aspect ratio:

  // 1. Measure the default size of the text string with the chosen font (scale 1)
  display.setTextSize(1); // Temporarily set base scale to 1 for accurate measurement
  int defaultTextWidth = display.textWidth("HH:MM"); // Measure a typical time string "HH:MM"
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
  // The setTextSize(float) function will handle scaling.
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

  // --- LDR Brightness Control ---
  // Adjust display brightness based on LDR reading at a regular interval.
  if (millis() - lastBrightnessUpdate >= brightnessUpdateInterval) {
    int ldrValue = analogRead(LDR_PIN);

    // Map LDR value (typically 0-4095 on ESP32 ADC) to brightness (10-255).
    // Assuming LDR is configured so that more light results in a higher analogRead value.
    // Constrain brightness to a minimum of 10 to ensure the screen is never completely off
    // and a maximum of 255.
    int brightness = map(ldrValue, 0, 4095, 10, 255);
    brightness = constrain(brightness, 10, 255); // Ensure brightness stays within valid range

    display.setBrightness(brightness);
    lastBrightnessUpdate = millis();
  }

  // Wait for 1 second before the next time display update to avoid rapid flickering and save CPU cycles.
  // The NTP client update interval is independent of this delay.
  // The LDR brightness update interval is also independent, handled by `lastBrightnessUpdate`.
  delay(1000);
}