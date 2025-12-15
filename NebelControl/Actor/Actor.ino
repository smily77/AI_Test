#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Arduino.h>

// ====== PINS ======
constexpr uint8_t PIN_RELAY = 10;
constexpr uint8_t PIN_SENSE = 4; // digital input (voltage divider)

// ====== ESP-NOW ======
// Replace the MAC addresses with the real hardware addresses before uploading.
// The addresses here are placeholders.
const uint8_t ACTOR_MAC[6] = {0x24, 0x6F, 0x28, 0xAA, 0xAA, 0x02};
const uint8_t CONTROLLER_MAC[6] = {0x24, 0x6F, 0x28, 0xAA, 0xAA, 0x01};

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
unsigned long lastControllerMsg = 0;

// ====== TIMING ======
constexpr unsigned long STATUS_INTERVAL_MS = 1000;
unsigned long lastStatusSentMs = 0;

constexpr unsigned long CONTROLLER_TIMEOUT_MS = 5000;

void updatePower() {
  powerOk = digitalRead(PIN_SENSE) == HIGH;
}

void applyRelay(bool on) {
  relayOn = on && powerOk;
  digitalWrite(PIN_RELAY, relayOn ? HIGH : LOW);
}

void sendStatus() {
  StatusMessage msg{MSG_STATUS, relayOn, powerOk};
  esp_now_send(CONTROLLER_MAC, reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
  lastStatusSentMs = millis();
}

void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // callback reserved for debugging
}

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len < 1) return;
  if (data[0] != MSG_COMMAND || len != sizeof(CommandMessage)) return;

  CommandMessage incoming;
  memcpy(&incoming, data, sizeof(CommandMessage));
  lastControllerMsg = millis();

  updatePower();
  applyRelay(incoming.desiredRelay);
  sendStatus();
}

void setupEspNow() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, ACTOR_MAC);

  if (esp_now_init() != ESP_OK) {
    return;
  }

  esp_now_register_send_cb(onSend);
  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, CONTROLLER_MAC, 6);
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
}

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  applyRelay(false);

  pinMode(PIN_SENSE, INPUT);

  setupEspNow();
}

void loop() {
  updatePower();

  if (!powerOk) {
    applyRelay(false);
  }

  unsigned long now = millis();
  if (now - lastStatusSentMs > STATUS_INTERVAL_MS) {
    sendStatus();
  }

  if (lastControllerMsg != 0 && (now - lastControllerMsg) > CONTROLLER_TIMEOUT_MS) {
    applyRelay(false);
  }
}
