#include <M5AtomS3.h> // M5Stack AtomS3 specific functions, including display control
#include <WiFi.h>     // Standard ESP32 Wi-Fi library
#include <time.h>     // Standard C library for time functions (tm struct, getLocalTime)
#include "esp_sntp.h" // For sntp_set_sync_interval

// --- Wi-Fi Configuration ---
// IMPORTANT: Replace with your actual Wi-Fi network credentials
const char *ssid = "Rohan";     // Your Wi-Fi network name (SSID)
const char *password = "Good Morning... Darling! @224477%"; // Your Wi-Fi network password

// --- NTP (Network Time Protocol) Configuration ---
const char *ntpServer = "pool.ntp.org"; // NTP server address (e.g., "pool.ntp.org")

// For Europe/Zurich:
// GMT Offset: +1 hour (CET) = 3600 seconds
// Daylight Saving Time Offset: +1 hour (additional to GMT offset for CEST) = 3600 seconds
const long gmtOffset_sec = 3600;       // Standard time offset from UTC in seconds (e.g., CET is UTC+1)
const int daylightOffset_sec = 3600;   // Additional offset for Daylight Saving Time in seconds (e.g., CEST is UTC+2)

// Timezone string for Zürich. This string defines the rules for when DST applies.
// CET-1CEST,M3.5.0,M10.5.0/3 means:
// CET (Central European Time) is 1 hour ahead of UTC (handled by gmtOffset_sec).
// CEST (Central European Summer Time) is also used (handled by daylightOffset_sec).
// DST starts on the 5th Sunday of March at 00:00 (M3.5.0).
// DST ends on the 5th Sunday of October at 03:00 (M10.5.0/3).
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// --- SETUP Function ---
void setup() {
  // Initialize the M5AtomS3. This sets up the serial communication,
  // initializes the display, and other core components.
  M5.begin();

  // Initialize Serial Monitor for debugging output
  Serial.begin(115200);
  Serial.println("M5AtomS3 NTP Time and Date Display");
  Serial.println("----------------------------------");

  // Clear the display and show a connecting message
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextFont(0); // Use default font
  M5.Lcd.setTextColor(WHITE, BLACK); // Set text color to white, background to black
  M5.Lcd.setTextSize(1); // Small text size for connection messages
  M5.Lcd.setCursor(0, 0); // Start text from top-left
  M5.Lcd.print("Connecting to WiFi");
  Serial.print("Connecting to WiFi");

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  int attempt_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    M5.Lcd.print("."); // Show progress on display
    attempt_count++;
    if (attempt_count > 20) { // After 10 seconds of trying
      Serial.println("\nFailed to connect. Retrying...");
      M5.Lcd.setCursor(0, M5.Lcd.getCursorY() + 10); // Move cursor down
      M5.Lcd.print("Failed. Retrying...");
      attempt_count = 0; // Reset counter to keep trying indefinitely
    }
  }

  // Wi-Fi connection successful
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  M5.Lcd.fillScreen(BLACK); // Clear display
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("WiFi Connected!");
  M5.Lcd.setCursor(0, 15);
  M5.Lcd.print("IP: ");
  M5.Lcd.print(WiFi.localIP());
  delay(2000); // Display IP for 2 seconds

  // --- NTP Time Synchronization Setup ---
  Serial.println("Setting up NTP and Timezone...");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Syncing Time...");

  // Set the timezone environment variable for DST rules.
  // This is used by getLocalTime() to determine if DST is active based on the date.
  setenv("TZ", TZ_INFO, 1);
  tzset(); // Update the C library's timezone information based on TZ_INFO

  // Initialize SNTP client with NTP server, GMT offset, and Daylight Saving Time offset.
  // The gmtOffset_sec and daylightOffset_sec are fixed values (e.g., for CET and CEST).
  // The TZ_INFO string then provides the rules for when to apply daylightOffset_sec.
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Set the NTP synchronization interval to 12 hours (in milliseconds)
  // This tells the ESP32's internal SNTP service to only request time from the server every 12 hours.
  sntp_set_sync_interval(12UL * 60 * 60 * 1000); // 12 hours * 60 min * 60 sec * 1000 ms

  // Wait for the initial time synchronization to complete
  struct tm timeinfo;
  int sync_attempts = 0;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
    M5.Lcd.print(".");
    sync_attempts++;
    if (sync_attempts > 120) { // After 60 seconds (120 * 500ms)
      Serial.println("\nFailed to obtain time from NTP. Retrying...");
      M5.Lcd.setCursor(0, M5.Lcd.getCursorY() + 10);
      M5.Lcd.print("NTP Failed. Retrying...");
      sync_attempts = 0; // Reset counter to keep trying indefinitely
    }
  }
  Serial.println("\nNTP Time synchronized.");
  M5.Lcd.fillScreen(BLACK); // Clear display
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Time Synced!");
  delay(2000); // Display for 2 seconds
}

// --- LOOP Function ---
void loop() {
  // Check if Wi-Fi is still connected. If not, try to reconnect.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected. Attempting to reconnect...");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(RED, BLACK); // Indicate issue with red text
    M5.Lcd.print("WiFi Disconnected!");
    M5.Lcd.setCursor(0, 15);
    M5.Lcd.print("Reconnecting...");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi Reconnected.");
    M5.Lcd.fillScreen(BLACK); // Clear display after successful reconnect
    M5.Lcd.setTextColor(WHITE, BLACK); // Reset text color to white
  }

  // Get current time and date using the configured timezone
  struct tm timeinfo;
  // getLocalTime returns true if time has been synchronized and is valid.
  // The second parameter (0) tells it to wait for time if not yet available,
  // but we already waited in setup, so this should generally be quick.
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time. NTP sync might be lost or ongoing.");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.print("NTP Sync Error!");
    M5.Lcd.setCursor(0, 15);
    M5.Lcd.print("Retrying...");
    delay(1000); // Wait a bit before retrying display
    return; // Skip drawing this loop if time not available
  }

  // Extract time components
  int currentHours = timeinfo.tm_hour;
  int currentMinutes = timeinfo.tm_min;
  int currentSeconds = timeinfo.tm_sec;

  // Extract date components
  int currentDay = timeinfo.tm_mday;
  int currentMonth = timeinfo.tm_mon + 1; // tm_mon is 0-11, so add 1 for actual month
  int currentYear = timeinfo.tm_year + 1900; // tm_year is years since 1900, so add 1900

  // Format the time as HH:MM with leading zeros
  char timeBuffer[6]; // HH:MM\0
  sprintf(timeBuffer, "%02d:%02d", currentHours, currentMinutes);
  String displayTime = String(timeBuffer);

  // Format the date as DD.MM.YYYY with leading zeros
  char dateBuffer[11]; // DD.MM.YYYY\0
  sprintf(dateBuffer, "%02d.%02d.%04d", currentDay, currentMonth, currentYear);
  String displayDate = String(dateBuffer);

  // Print the current time and date to the Serial Monitor for debugging
  Serial.printf("Current Time: %s:%02d Date: %s\n", displayTime.c_str(), currentSeconds, displayDate.c_str());

  // --- Display Time and Date on M5AtomS3 LCD ---
  const int lineSep = 26;
  M5.Lcd.fillScreen(BLACK); // Clear the display to prevent ghosting from previous text

  // Set text properties for the HH:MM display
  M5.Lcd.setTextColor(GREEN, BLACK); // Set text color to green, background to black
  M5.Lcd.setTextFont(4);             // Use font 4 (a larger, "nice" sans-serif font)
  M5.Lcd.setTextSize(2);             // Text size 1 is appropriate for font 4 to fit

  // Set the text datum (anchor point) to the middle center
  M5.Lcd.setTextDatum(MC_DATUM);

  // Display time in the exact vertical center of the screen
  // M5AtomS3 screen height is 240 pixels. Center is 120.
  M5.Lcd.drawString(displayTime, M5.Lcd.width() / 2, M5.Lcd.height() / 2 - lineSep);

  // Set text properties for the Date display
  M5.Lcd.setTextColor(WHITE, BLACK); // Set text color to white
  M5.Lcd.setTextFont(4);             // Use font 2 for the date (clean, smaller font)
  M5.Lcd.setTextSize(1);             // Text size 1 for font 2

  // Calculate position for date, below the time.
  // Font 4 height is approximately 48 pixels. Font 2 height is approximately 16 pixels.
  // Time is centered at Y=120. Its bottom edge is 120 + (48/2) = 144.
  // Add a 10 pixel gap, so the top of the date text is 144 + 10 = 154.
  // Since date uses MC_DATUM, its center Y will be 154 + (16/2) = 162.
  M5.Lcd.drawString(displayDate, M5.Lcd.width() / 2, M5.Lcd.height() / 2 + lineSep);

  // --- Display Seconds as a Progress Bar ---
  int barHeight = 8; // Height of the seconds bar in pixels
  // Y position for the bottom bar, ensuring it's at the very bottom
  int barY = M5.Lcd.height() - barHeight;

  // Calculate the width of the bar for one side (from center to edge)
  // map(value, fromLow, fromHigh, toLow, toHigh) scales the seconds to half the display width
  int halfBarWidth = map(currentSeconds, 0, 59, 0, M5.Lcd.width() / 2);

  // Calculate the starting X position for the bar to be centered
  int barX = M5.Lcd.width() / 2 - halfBarWidth;

  // Calculate the total width of the bar (growing from center outwards)
  int barDisplayWidth = 2 * halfBarWidth;

  // Draw the seconds bar (e.g., in DARKGREY color)
  M5.Lcd.fillRect(barX, barY, barDisplayWidth, barHeight, DARKGREY);

  // Pause for 1 second before the next display update
  // This makes the clock update every second.
  delay(1000);
}
