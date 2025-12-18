/*
  GridPowerGauge for M5Stack AtomS3

  This program displays the grid power (import/export) from a PV system
  as a gauge on the M5Stack AtomS3 display. It receives data via UDP
  broadcast from a "Poller" device on the local network.

  Instructions:
  1. Fill in your WiFi credentials (ssid and password).
  2. Upload the code to your M5Stack AtomS3.
*/

#include <M5AtomS3.h>
#include <WiFi.h>
#include <AsyncUDP.h>

// --- Configuration ---
//-!!- IMPORTANT: Replace with your WiFi credentials -!!-
const char* ssid = "IHR_WLAN_NAME";
const char* password = "IHR_WLAN_PASSWORT";

// UDP Multicast settings (from your pv_batt project)
const IPAddress MCAST_GRP(239, 12, 12, 12);
const uint16_t MCAST_PORT = 55221;

// Gauge display settings
const int GAUGE_CENTER_X = 64;
const int GAUGE_CENTER_Y = 70;
const int GAUGE_RADIUS = 60;
const int NEEDLE_RADIUS = 55;
const int MAX_GRID_W = 4000; // Max value for the gauge (e.g., 4000W)

// --- Data Structure for PV Frame ---
// This must match the structure sent by the Poller
#define PV_MAGIC 0xBEEF
#define PV_VERSION 4

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint8_t version;
  uint32_t seq;
  uint32_t ts;
  int32_t pvW;
  int32_t gridW;
  int32_t battW;
  int32_t loadW;
  int16_t temp10;
  uint16_t socx10;
  float pvTodayKWh;
  float gridExpToday;
  float gridImpToday;
  float loadTodayKWh;
  // ... other fields from V4 if necessary, ensure total size matches sender
} PvFrameV4;

// --- Global Variables ---
AsyncUDP udp;
PvFrameV4 lastFrame;
volatile bool newDataAvailable = false;
int32_t lastDisplayedGridW = -99999;
float lastAngle = -999;

// --- Function Declarations ---
void drawGauge(int32_t gridW);
void setupWiFi();
void setupUDP();

// --- Main Setup ---
void setup() {
  M5.begin(true, true, true, false); // Init M5AtomS3 (Lcd, Serial, I2C, but not IMU)
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  drawGauge(0); // Draw initial empty gauge
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("Connecting to WiFi...", 64, 64);

  setupWiFi();
  setupUDP();
}

// --- Main Loop ---
void loop() {
  if (newDataAvailable) {
    newDataAvailable = false;
    // Only update display if the value has changed
    if (lastFrame.gridW != lastDisplayedGridW) {
      drawGauge(lastFrame.gridW);
      lastDisplayedGridW = lastFrame.gridW;
    }
  }
  delay(50); // Small delay to prevent high CPU usage
}

// --- Drawing Functions ---

// Draws the complete gauge with needle and text
void drawGauge(int32_t gridW) {
  // Clamp the value to the gauge's min/max
  int32_t displayW = constrain(gridW, -MAX_GRID_W, MAX_GRID_W);

  // Map the grid power to an angle (150 degrees for Export, 30 for Import)
  float angle = map(displayW, -MAX_GRID_W, MAX_GRID_W, 150, 30);
  float angleRad = radians(angle);

  // Don't redraw if the angle hasn't changed
  if (abs(angle - lastAngle) < 1.0) return;

  // Clear previous needle by drawing it in black
  if (lastAngle > -999) {
    float lastAngleRad = radians(lastAngle);
    int x_end_old = GAUGE_CENTER_X + NEEDLE_RADIUS * cos(lastAngleRad);
    int y_end_old = GAUGE_CENTER_Y - NEEDLE_RADIUS * sin(lastAngleRad);
    M5.Lcd.drawLine(GAUGE_CENTER_X, GAUGE_CENTER_Y, x_end_old, y_end_old, TFT_BLACK);
  }

  // Draw gauge background and arc
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.drawArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS, GAUGE_RADIUS-2, 210, 330, TFT_DARKGREY, TFT_BLACK); // 150 to 30 is 210 to 330 in TFT_eSPI angles
  
  // Draw green part for export (left side)
  M5.Lcd.drawArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS, GAUGE_RADIUS-2, 210, 270, TFT_GREEN, TFT_BLACK);
  
  // Draw red part for import (right side)
  M5.Lcd.drawArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS, GAUGE_RADIUS-2, 270, 330, TFT_RED, TFT_BLACK);

  // Draw labels
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextColor(TFT_GREEN);
  M5.Lcd.drawString("Export", GAUGE_CENTER_X - 40, GAUGE_CENTER_Y + 15);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.drawString("Import", GAUGE_CENTER_X + 40, GAUGE_CENTER_Y + 15);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.drawString("0", GAUGE_CENTER_X, GAUGE_CENTER_Y - 45);

  // Draw digital value
  M5.Lcd.setTextFont(4);
  String powerText = String(gridW) + " W";
  M5.Lcd.drawString(powerText, GAUGE_CENTER_X, GAUGE_CENTER_Y + 35);
  M5.Lcd.setTextFont(2);

  // Draw the new needle
  int x_end = GAUGE_CENTER_X + NEEDLE_RADIUS * cos(angleRad);
  int y_end = GAUGE_CENTER_Y - NEEDLE_RADIUS * sin(angleRad);
  M5.Lcd.drawLine(GAUGE_CENTER_X, GAUGE_CENTER_Y, x_end, y_end, TFT_ORANGE);
  M5.Lcd.fillCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 4, TFT_WHITE);
  
  lastAngle = angle;
}

// --- Network Setup ---

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("
WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("WiFi Connected", 64, 80);
    delay(1000);
  } else {
    Serial.println("
WiFi Connection Failed!");
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("WiFi Failed!", 64, 64);
    // Halt execution if WiFi fails
    while(1);
  }
}

void setupUDP() {
  if (udp.listenMulticast(MCAST_GRP, MCAST_PORT)) {
    Serial.println("UDP Listening on port " + String(MCAST_PORT));
    udp.onPacket([](AsyncUDPPacket packet) {
      // Check if packet size matches the data structure
      if (packet.length() != sizeof(PvFrameV4)) {
        return;
      }
      
      PvFrameV4 tempFrame;
      memcpy(&tempFrame, packet.data(), sizeof(PvFrameV4));
      
      // Check for magic number to validate the frame
      if (tempFrame.magic == PV_MAGIC) {
        memcpy(&lastFrame, &tempFrame, sizeof(PvFrameV4));
        newDataAvailable = true;
      }
    });
  } else {
    Serial.println("UDP Listen failed");
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("UDP Failed!", 64, 64);
  }
}
