#include <M5Unified.h>
#include <WiFi.h>
#include <AsyncUDP.h>

// Required Libraries:
// 1. M5Atom: Install via Arduino Library Manager -> Search for "M5Atom" by M5Stack.
// 2. AsyncTCP (dependency for AsyncUDP): Install via Arduino Library Manager -> Search for "AsyncTCP" by Marvin Roger.
// 3. WiFi (built-in for ESP32 boards).

// --- WiFi Configuration ---
const char* ssid = "Rohan";         // Your WiFi network SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi network password

// --- Multicast Configuration ---
static const IPAddress MCAST_GRP(239, 12, 12, 12); // Multicast group IP address as specified
static const uint16_t  MCAST_PORT = 55221;       // Multicast port as specified

AsyncUDP udpFrame; // Async UDP object for sending/receiving frames

// --- Frame V4 Definition ---
#define PV_MAGIC   0xBEEF
#define PV_VERSION 4

// Structure for PV data frame, packed to ensure no padding bytes are added by the compiler.
// This is crucial for correct interpretation of the received binary data.
typedef struct __attribute__((packed)) {
  uint16_t magic;         // PV_MAGIC
  uint8_t  version;       // PV_VERSION (=4)
  uint32_t seq;           // laufende Nummer (sequence number)
  uint32_t ts;            // UNIX time (seconds)

  // Hauptwerte (Main values)
  int32_t  pvW;           // PV power in Watts
  int32_t  gridW;         // Grid power in Watts (positive for import, negative for export)
  int32_t  battW;         // Battery power in Watts (positive for charging, negative for discharging)
  int32_t  loadW;         // Load power in Watts

  int16_t  temp10;        // Temperature in 0.1°C
  uint16_t socx10;        // State of Charge in 0.1% (e.g., 855 for 85.5%)

  float    pvTodayKWh;    // PV energy today in kWh
  float    gridExpToday;  // Grid export energy today in kWh
  float    gridImpToday;  // Grid import energy today in kWh
  float    loadTodayKWh;  // Load energy today in kWh

  int32_t  eta20s;        // Seconds until 20% SOC (or -1 if unknown)

  // Zusatz (Skalen s. Namen) (Additional values, scales as per names)
  int16_t  pv1Voltage_x10_V; // PV1 Voltage in 0.1V
  int16_t  pv1Current_x10_A; // PV1 Current in 0.1A
  int16_t  pv2Voltage_x10_V; // PV2 Voltage in 0.1V
  int16_t  pv2Current_x10_A; // PV2 Current in 0.1A

  int32_t  gridVoltageA_x10_V; // Grid Voltage Phase A in 0.1V
  int32_t  gridVoltageB_x10_V; // Grid Voltage Phase B in 0.1V
  int32_t  gridVoltageC_x10_V; // Grid Voltage Phase C in 0.1V

  int32_t  gridCurrentA_x100_A; // Grid Current Phase A in 0.01A
  int32_t  gridCurrentB_x100_A; // Grid Current Phase B in 0.01A
  int32_t  gridCurrentC_x100_A; // Grid Current Phase C in 0.01A

  uint16_t crc;           // CRC-16 (Modbus) over all bytes before 'crc'
} PvFrameV4;

// Global variables to store the last received frame data and its status.
// `volatile` keyword ensures that the compiler doesn't optimize access to these
// variables, as they are modified by an interrupt-like UDP callback and read in loop().
PvFrameV4 lastF;
volatile bool haveFrame = false;
volatile uint32_t lastSeq = 0;
volatile unsigned long lastRxMs = 0; // Timestamp (millis()) of last valid frame reception

// --- CRC16 Modbus Calculation Function ---
// This function calculates the CRC-16 Modbus for a given data buffer,
// exactly as provided in the user's request.
static inline uint16_t crc16_modbus(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF; // Initial CRC value for Modbus
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i]; // XOR with current byte
    for (int j = 0; j < 8; j++) {
      if (crc & 1) crc = (crc >> 1) ^ 0xA001; // If LSB is 1, XOR with polynomial
      else         crc >>= 1;                 // If LSB is 0, just shift
    }
  }
  return crc;
}

// --- Display Functions ---

// Function to draw the battery symbol and SOC on the M5AtomS3 screen.
// The AtomS3 has a 128x128 pixel display, so dimensions are chosen to fit.
void drawBatteryStatus() {
  M5.Lcd.fillScreen(BLACK); // Clear the screen with black background

  // Define dimensions and position for the battery symbol
  int battX = 20; // X-coordinate of battery left edge
  int battY = 30; // Y-coordinate of battery top edge
  int battWidth = 80; // Width of the battery body
  int battHeight = 40; // Height of the battery body
  int battTerminalWidth = 10; // Width of the positive terminal
  int battTerminalHeight = 15; // Height of the positive terminal

  // Draw battery outline
  M5.Lcd.drawRect(battX, battY, battWidth, battHeight, WHITE); // Main body rectangle
  // Draw the positive terminal (small rectangle on the right side)
  M5.Lcd.fillRect(battX + battWidth, battY + (battHeight - battTerminalHeight) / 2,
                  battTerminalWidth, battTerminalHeight, WHITE);

  if (lastRxMs != 0 && (millis() - lastRxMs < 10000)) { // Only display data if a frame was recently received
    // Calculate fill level based on socx10 (0.1% increments)
    float socPercentage = lastF.socx10 / 10.0;
    // Map the SOC percentage (0-100%) to the fill width of the battery symbol.
    // Subtract 4 from battWidth to create a 2-pixel inner padding on each side.
    int fillWidth = map(constrain(socPercentage * 10, 0, 1000), 0, 1000, 0, battWidth - 4);
    int fillX = battX + 2; // X-coordinate for the filled part (inner padding)
    int fillY = battY + 2; // Y-coordinate for the filled part (inner padding)
    int fillHeight = battHeight - 4; // Height for the filled part (inner padding)

    uint32_t battColor = GREEN; // Default color for high charge
    if (socPercentage < 20) {
      battColor = RED; // Red for low charge
    } else if (socPercentage < 50) {
      battColor = YELLOW; // Yellow for medium charge
    }

    // Draw the filled part of the battery, representing the charge level
    M5.Lcd.fillRect(fillX, fillY, fillWidth, fillHeight, battColor);

    // Display SOC percentage text
    M5.Lcd.setTextFont(4); // Choose a font size (1, 2, 4, 6, 7, 8 are common M5GFX fonts)
    M5.Lcd.setTextColor(WHITE, BLACK); // Set text color to white with black background
    M5.Lcd.setCursor(battX, battY + battHeight + 30); // Position text below the battery symbol
    M5.Lcd.printf("%.1f%%", socPercentage); // Print SOC with one decimal place

    // Display "Last Rx" status for debugging/info
    //M5.Lcd.setCursor(battX, battY + battHeight + 40);
    //M5.Lcd.setTextFont(1); // Smaller font for this line
    //M5.Lcd.printf("Last Rx: %lu ms ago", millis() - lastRxMs);
  } else {
    // If no frame has been received yet or has timed out
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(battX, battY + battHeight + 10);
    M5.Lcd.println("No PV Data");
    M5.Lcd.setCursor(battX, battY + battHeight + 40);
    M5.Lcd.println("Waiting for UDP...");
  }

  // M5GFX (used by M5.Lcd with M5Unified) typically updates the display automatically
  // after drawing commands or via M5.update() in the loop. The .show() method is not part of its API.
}

// --- UDP Listener Setup ---
// Initializes the UDP listener for multicast frames on the specified group and port.
static void beginListenFrames() {
  // Attempt to listen for multicast packets on the given IP and Port, on the STA (station) interface.
  if (!udpFrame.listenMulticast(MCAST_GRP, MCAST_PORT, 1, TCPIP_ADAPTER_IF_STA)) {
    Serial.println("[ERR] listenMulticast failed");
    return; // Exit if listening setup fails
  }
  Serial.printf("Listening for multicast on %s:%d\n", MCAST_GRP.toString().c_str(), MCAST_PORT);

  // Define the callback function that will be executed when a UDP packet is received.
  udpFrame.onPacket([](AsyncUDPPacket p) {
    // 1. Basic length check: Ensure the packet is at least the size of our PvFrameV4 structure.
    if (p.length() < sizeof(PvFrameV4)) {
      Serial.printf("[WARN] Received packet too short (%d bytes)\n", p.length());
      return;
    }

    // 2. Cast received data to our PvFrameV4 structure for easier access.
    //    It's important to use `const PvFrameV4*` to indicate that `p.data()`
    //    returns a pointer to constant data.
    const PvFrameV4* f = (const PvFrameV4*)p.data();

    // 3. Validate magic and version numbers to ensure it's a known PV frame.
    if (f->magic != PV_MAGIC || f->version != PV_VERSION) {
      Serial.printf("[WARN] Invalid magic/version (0x%X/0x%X, expected 0x%X/0x%X)\n",
                    f->magic, f->version, PV_MAGIC, PV_VERSION);
      return;
    }

    // 4. Calculate CRC and compare with the CRC included in the frame.
    //    The CRC is calculated over all bytes up to, but not including, the 'crc' field itself.
    uint16_t check = crc16_modbus((const uint8_t*)p.data(), sizeof(PvFrameV4) - 2);
    if (check != f->crc) {
      Serial.printf("[WARN] CRC mismatch (calculated 0x%X, received 0x%X)\n", check, f->crc);
      return;
    }

    // 5. Check sequence number: Only process frames that are newer than the last one received.
    //    The `lastRxMs != 0` check ensures that the very first valid frame is always accepted.
    if (f->seq <= lastSeq && lastRxMs != 0) {
      // Serial.printf("[INFO] Ignoring old sequence number (current %u, last %u)\n", f->seq, lastSeq);
      return;
    }

    // If all validations pass, the frame is considered valid and new.
    // Store the data and update status flags.
    lastSeq = f->seq;      // Update the last sequence number
    lastRxMs = millis();   // Record the time of reception
    lastF = *f;            // Copy the entire valid frame data into our global variable
    haveFrame = true;      // Set the flag to indicate new data is available for display

    // Optional: Print some key data to Serial for real-time debugging
    Serial.printf("[INFO] Received valid frame. Seq: %u, SOC: %.1f%%, PV_W: %d\n", lastF.seq, lastF.socx10 / 10.0, lastF.pvW);
    // It's generally not recommended to call display update functions directly from
    // the UDP callback to avoid blocking the network task. Instead, set a flag and update in loop().
  });
}

// --- Setup Function ---
void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  M5.begin();           // Initialize M5Stack AtomS3 (includes display, power management, etc.)

  M5.Lcd.setTextFont(2); // Set default font for initial messages on the screen
  M5.Lcd.setTextColor(WHITE, BLACK); // Set text color to white with a black background
  M5.Lcd.setTextDatum(MC_DATUM); // Set text alignment to Middle-Center for easy positioning

  // Display initial WiFi connection message
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 20);
  M5.Lcd.println("Connecting to WiFi...");
  M5.Lcd.setCursor(M5.Lcd.width() / 2, M5.Lcd.height() / 2);
  M5.Lcd.println(ssid);
  // M5.Lcd.show(); // Removed: M5GFX does not have a .show() method.

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA); // Set the ESP32 to Wi-Fi station mode
  WiFi.begin(ssid, password);
  unsigned long wifiConnectStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    M5.Lcd.print("."); // Show connection progress on screen
    if (millis() - wifiConnectStart > 30000) { // Timeout after 30 seconds
      Serial.println("\n[ERR] WiFi connection failed. Please check credentials.");
      M5.Lcd.fillScreen(RED); // Indicate failure with a red screen
      M5.Lcd.setCursor(M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 10);
      M5.Lcd.println("WiFi Failed!");
      M5.Lcd.setCursor(M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 10);
      M5.Lcd.println("Restart!");
      // M5.Lcd.show(); // Removed: M5GFX does not have a .show() method.
      while (true) delay(1000); // Halt the program if WiFi fails to connect
    }
  }
  Serial.println("\n[INFO] WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Display successful WiFi connection message and IP address
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 20);
  M5.Lcd.println("WiFi Connected!");
  M5.Lcd.setCursor(M5.Lcd.width() / 2, M5.Lcd.height() / 2);
  M5.Lcd.println(WiFi.localIP().toString());
  // M5.Lcd.show(); // Removed: M5GFX does not have a .show() method.
  delay(2000); // Show connection info for a brief moment

  // Start listening for UDP frames from the PV poller
  beginListenFrames();

  // Perform an initial display update to show "No PV Data" or current status.
  drawBatteryStatus();
}

// --- Loop Function ---
void loop() {
  // Check the `haveFrame` flag. If true, a new valid frame has been received.
  if (haveFrame) {
    drawBatteryStatus(); // Update the display with the new data
    haveFrame = false;   // Reset the flag so it only draws when new data arrives
  }

  // Periodically check for a data timeout. If no new frame for 10 seconds,
  // display a "No Data" message to indicate the source might be down or disconnected.
  // The `lastRxMs != 0` check prevents this from firing before any data is ever received.
/*
  if (lastRxMs != 0 && (millis() - lastRxMs > 10000)) {
    // If more than 10 seconds since last valid frame, reset status and update display.
    // This condition will keep the "No PV Data" message on screen until a new frame arrives.
    if (lastRxMs != 0) { // Only draw if it wasn't already in the "No PV Data" state
      drawBatteryStatus(); // Draw the "No PV Data" state
      lastRxMs = 0; // Reset lastRxMs to prevent repeated drawing of "No PV Data" until a new frame arrives.
    }
  }
*/
  M5.update(); // M5AtomS3 specific: required to keep internal services (like buttons, IMU) updated.
  delay(100); // Small delay to prevent busy-waiting and allow other tasks to run (e.g., WiFi, internal M5Atom tasks).
}