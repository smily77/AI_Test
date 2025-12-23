/*
  GridPowerGauge for M5Stack AtomS3

  This program displays the grid power (import/export) from a PV system
  as a gauge on the M5Stack AtomS3 display. It receives data via UDP
  broadcast from a "Poller" device on the local network.

  Instructions:
  1. Ensure you have a 'Credentials.h' file with your WiFi credentials.
  2. Upload the code to your M5Stack AtomS3.
  3. Open the Serial Monitor to view diagnostic messages.
*/

#include <M5AtomS3.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <Credentials.h> // Include for WiFi credentials

// UDP Multicast settings
const IPAddress MCAST_GRP(239, 12, 12, 12);
const uint16_t MCAST_PORT = 55221;

// Gauge display settings
const int GAUGE_CENTER_X = 64;
const int GAUGE_CENTER_Y = 70;
const int GAUGE_RADIUS = 60;
const int NEEDLE_RADIUS = 55;
const int MAX_GRID_W = 4000; // Max value for the gauge (e.g., 4000W)

// --- Data Structure for PV Frame ---
// This MUST match the structure sent by the Poller (pv_batt)
#define PV_MAGIC 0xBEEF
#define PV_VERSION 4

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint8_t  version;
  uint32_t seq;
  uint32_t ts;
  int32_t  pvW;
  int32_t  gridW;
  int32_t  battW;
  int32_t  loadW;
  int16_t  temp10;
  uint16_t socx10;
  float    pvTodayKWh;
  float    gridExpToday;
  float    gridImpToday;
  float    loadTodayKWh;
  int32_t  eta20s;
  int16_t  pv1Voltage_x10_V;
  int16_t  pv1Current_x10_A;
  int16_t  pv2Voltage_x10_V;
  int16_t  pv2Current_x10_A;
  int32_t  gridVoltageA_x10_V;
  int32_t  gridVoltageB_x10_V;
  int32_t  gridVoltageC_x10_V;
  int32_t  gridCurrentA_x100_A;
  int32_t  gridCurrentB_x100_A;
  int32_t  gridCurrentC_x100_A;
  uint16_t crc;
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
static inline uint16_t crc16_modbus(const uint8_t* data, size_t len);

// --- Main Setup ---
void setup() {
  M5.begin();
  Serial.begin(115200);
  delay(100);
  Serial.println("Serial Monitor started.");

  M5.Lcd.setRotation(1);
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  Serial.println("M5.Lcd initialized.");

  drawGauge(0);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("Connecting to WiFi...", 64, 64);
  Serial.println("Initial gauge drawn, attempting WiFi connection...");

  setupWiFi();
  setupUDP();
  Serial.println("Setup complete.");
}

// --- Main Loop ---
void loop() {
  if (newDataAvailable) {
    newDataAvailable = false;
    if (lastFrame.gridW != lastDisplayedGridW) {
      drawGauge(lastFrame.gridW);
      lastDisplayedGridW = lastFrame.gridW;
    }
  }
  delay(50);
}

// --- Drawing Functions ---
void drawGauge(int32_t gridW) {
  int32_t displayW = constrain(gridW, -MAX_GRID_W, MAX_GRID_W);
  float angle = map(displayW, -MAX_GRID_W, MAX_GRID_W, 210, -30);

  if (abs(angle - lastAngle) < 1.0) return;

  float lastAngleRad = radians(lastAngle);
  int x_end_old = GAUGE_CENTER_X + NEEDLE_RADIUS * cos(lastAngleRad);
  int y_end_old = GAUGE_CENTER_Y - NEEDLE_RADIUS * sin(lastAngleRad);
  if (lastAngle > -999) {
    M5.Lcd.drawLine(GAUGE_CENTER_X, GAUGE_CENTER_Y, x_end_old, y_end_old, TFT_BLACK);
  }

  M5.Lcd.fillArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS + 4, GAUGE_RADIUS -2, 210, -30, TFT_BLACK);

  M5.Lcd.drawArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS, GAUGE_RADIUS-2, 210, 30, TFT_DARKGREY);
  M5.Lcd.drawArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS, GAUGE_RADIUS-2, 210, 270, TFT_GREEN);
  M5.Lcd.drawArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS, GAUGE_RADIUS-2, 270, 330, TFT_RED);

  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextColor(TFT_GREEN);
  M5.Lcd.drawString("Export", GAUGE_CENTER_X - 40, GAUGE_CENTER_Y + 15);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.drawString("Import", GAUGE_CENTER_X + 40, GAUGE_CENTER_Y + 15);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.drawString("0", GAUGE_CENTER_X, GAUGE_CENTER_Y - 45);
  M5.Lcd.drawString(String(MAX_GRID_W/1000)+"kW", GAUGE_CENTER_X - 50, GAUGE_CENTER_Y - 25);
  M5.Lcd.drawString(String(MAX_GRID_W/1000)+"kW", GAUGE_CENTER_X + 50, GAUGE_CENTER_Y - 25);

  M5.Lcd.setTextFont(4);
  String powerText = String(gridW) + " W";
  M5.Lcd.drawString(powerText, GAUGE_CENTER_X, GAUGE_CENTER_Y + 35);
  M5.Lcd.setTextFont(2);

  float angleRad = radians(angle);
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
  Serial.print("Connecting to WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("WiFi Connected", 64, 80);
    delay(1000);
    drawGauge(0);
  } else {
    Serial.println("\nWiFi Connection Failed!");
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("WiFi Failed!", 64, 64);
    while(1);
  }
}

void setupUDP() {
  if (udp.listenMulticast(MCAST_GRP, MCAST_PORT)) {
    Serial.println("UDP Listening on port " + String(MCAST_PORT));
    udp.onPacket([](AsyncUDPPacket packet) {
      if (packet.length() != sizeof(PvFrameV4)) {
        Serial.printf("Error: Invalid packet size. Expected: %d, Received: %d\n", sizeof(PvFrameV4), packet.length());
        return;
      }
      
      const PvFrameV4* f = (const PvFrameV4*)packet.data();

      if (f->magic != PV_MAGIC || f->version != PV_VERSION) {
        Serial.printf("Error: Invalid magic/version. Expected: %X/%d, Received: %X/%d\n", PV_MAGIC, PV_VERSION, f->magic, f->version);
        return;
      }

      uint16_t check_crc = crc16_modbus((const uint8_t*)f, sizeof(PvFrameV4) - 2);
      if (check_crc != f->crc) {
        Serial.printf("Error: CRC mismatch. Expected: %X, Calculated: %X\n", f->crc, check_crc);
        return;
      }
      
      memcpy(&lastFrame, f, sizeof(PvFrameV4));
      newDataAvailable = true;

      Serial.printf("Success: Frame parsed. Grid Power: %d W\n", lastFrame.gridW);
    });
  } else {
    Serial.println("UDP Listen failed");
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("UDP Failed!", 64, 64);
  }
}

// --- CRC16 Modbus Calculation ---
static inline uint16_t crc16_modbus(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) crc = (crc >> 1) ^ 0xA001;
      else crc >>= 1;
    }
  }
  return crc;
}
