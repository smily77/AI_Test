#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold12pt7b.h>

// ====== PINS ======
constexpr uint8_t PIN_LED = 6;
constexpr uint8_t PIN_BUTTON = 7; // active LOW
constexpr uint8_t PIN_SDA = 8;
constexpr uint8_t PIN_SCL = 9;

// ====== DISPLAY ======
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 32;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ====== ESP-NOW ======
// Replace the MAC addresses with the real hardware addresses before uploading.
// The addresses here are placeholders.
const uint8_t CONTROLLER_MAC[6] = {0x24, 0x6F, 0x28, 0xAA, 0xAA, 0x01};
const uint8_t ACTOR_MAC[6] = {0x24, 0x6F, 0x28, 0xAA, 0xAA, 0x02};

constexpr uint8_t MSG_STATUS = 1;
constexpr uint8_t MSG_COMMAND = 2;

struct StatusMessage {
  uint8_t msgType;
  bool relayOn;
  bool powerOk;
};

struct CommandMessage {
  uint8_t msgType;
  bool desiredRelay;
};

// ====== STATE ======
bool relayOn = false;
bool powerOk = false;
bool linkOk = false;
unsigned long lastStatusMs = 0;
bool desiredRelayState = false;
bool lastSentCommandState = false;

// ====== TIMING ======
constexpr unsigned long STATUS_TIMEOUT_MS = 5000;
constexpr unsigned long COMMAND_INTERVAL_MS = 1000;
unsigned long lastCommandSentMs = 0;

// ====== BUTTON ======
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
constexpr unsigned long DEBOUNCE_DELAY_MS = 40;

void updateDisplay(const String &message, bool keepOn) {
  if (!keepOn) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    return;
  }

  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  display.setFont(&FreeSansBold12pt7b);
  display.setTextColor(SSD1306_WHITE);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
  int16_t x = (SCREEN_WIDTH - w) / 2;
  int16_t y = (SCREEN_HEIGHT + h) / 2 - 2; // visually centered
  display.setCursor(x, y);
  display.print(message);
  display.display();
}

void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // No action required, but keeps callback registered for completeness.
}

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len < 1) return;
  if (data[0] != MSG_STATUS || len != sizeof(StatusMessage)) return;

  StatusMessage incoming;
  memcpy(&incoming, data, sizeof(StatusMessage));

  relayOn = incoming.relayOn;
  powerOk = incoming.powerOk;
  linkOk = true;
  lastStatusMs = millis();

  // Synchronize desired state with actual when status arrives.
  if (!desiredRelayState && relayOn) {
    desiredRelayState = relayOn;
  }
}

void setupDisplay() {
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    // If display fails to initialize, continue without blocking.
    return;
  }
  display.clearDisplay();
  updateDisplay("Ready", true);
}

void setupEspNow() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, CONTROLLER_MAC);

  if (esp_now_init() != ESP_OK) {
    return;
  }

  esp_now_register_send_cb(onSend);
  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, ACTOR_MAC, 6);
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
}

void sendCommand(bool state) {
  CommandMessage msg{MSG_COMMAND, state};
  esp_now_send(ACTOR_MAC, reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
  lastCommandSentMs = millis();
  lastSentCommandState = state;
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  setupDisplay();
  setupEspNow();
}

void handleButton() {
  int reading = digitalRead(PIN_BUTTON);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    if (reading == LOW && lastButtonState == HIGH) {
      if (linkOk && powerOk) {
        desiredRelayState = !relayOn;
        sendCommand(desiredRelayState);
      }
    }
  }
  lastButtonState = reading;
}

void loop() {
  if (linkOk && millis() - lastStatusMs > STATUS_TIMEOUT_MS) {
    linkOk = false;
    powerOk = false;
  }

  handleButton();

  if (linkOk && powerOk) {
    bool needResend = (desiredRelayState != relayOn) || (millis() - lastCommandSentMs > COMMAND_INTERVAL_MS) || (lastSentCommandState != desiredRelayState);
    if (needResend) {
      sendCommand(desiredRelayState);
    }

    digitalWrite(PIN_LED, relayOn ? HIGH : LOW);
    updateDisplay(relayOn ? "On" : "Ready", true);
  } else {
    digitalWrite(PIN_LED, LOW);
    bool showText = digitalRead(PIN_BUTTON) == LOW;
    if (showText) {
      if (!linkOk) {
        updateDisplay("No Link", true);
      } else if (!powerOk) {
        updateDisplay("No Power", true);
      }
    } else {
      updateDisplay("", false);
    }
  }
}
