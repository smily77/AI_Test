# PV_Batt

## Project Description

Ein Prozessor ermittelt die Daten einer PV anlage (Er pollt sie -> Poller) und sendet sie über UDP als broadcast.
Mit folgender IP und part adr:
static const IPAddress MCAST_GRP(239, 12, 12, 12);
static const uint16_t  MCAST_PORT = 55221;
durch diese Anweisung:
    udpFrame.writeTo((uint8_t*)&lastF, sizeof(PvFrameV4), MCAST_GRP, MCAST_PORT);
Die definition des Datenframes ist diese:
// ------------------------- Frame V4 -------------------------
#define PV_MAGIC   0xBEEF
#define PV_VERSION 4

typedef struct __attribute__((packed)) {
  uint16_t magic;         // PV_MAGIC
  uint8_t  version;       // PV_VERSION (=4)
  uint32_t seq;           // laufende Nummer
  uint32_t ts;            // UNIX time (s)

  // Hauptwerte
  int32_t  pvW;
  int32_t  gridW;
  int32_t  battW;
  int32_t  loadW;

  int16_t  temp10;        // 0.1°C
  uint16_t socx10;        // 0.1%

  float    pvTodayKWh;
  float    gridExpToday;
  float    gridImpToday;
  float    loadTodayKWh;

  int32_t  eta20s;        // Sekunden bis 20% (oder -1 wenn unbekannt)

  // Zusatz (Skalen s. Namen)
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

  uint16_t crc;           // CRC-16 (Modbus) über alles bis vor 'crc'
} PvFrameV4;

Mache einen Code der den Frame empfängt und den Ladezustand der Batterie ( socx10;) auf dem Bildschirm des M5Stack Atom3S anzeigt. Neben der %-Angabe soll auch ein Batteriesymbol die aktelle Batterieladung 
visualisieren

Weitere prozeduren als beispiel zum analaysieren
Empfangs funktion:
static void beginListenFrames(){
  if (!udpFrame.listenMulticast(MCAST_GRP, MCAST_PORT, 1, TCPIP_ADAPTER_IF_STA)){
    Serial.println("[ERR] listenMulticast failed"); return;
  }
  udpFrame.onPacket([](AsyncUDPPacket p){
    if (p.length() < sizeof(PvFrameV4)) return;
    const PvFrameV4* f = (const PvFrameV4*)p.data();
    if (f->magic!=PV_MAGIC || f->version!=PV_VERSION) return;
    uint16_t check = crc16_modbus((const uint8_t*)p.data(), sizeof(PvFrameV4)-2);
    if (check != f->crc) return;
    if (f->seq <= lastSeq) return;

    lastSeq = f->seq; lastRxMs = millis();
    lastF = *f; haveFrame=true;

    // Lokal integrieren, damit Anzeige auf Clients Werte hat
    integrateTick(lastF.pvW, lastF.gridW, lastF.battW);
    handleDayMonthRollover();

    drawIfFrame();
  });
}

static inline uint16_t crc16_modbus(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) crc = (crc >> 1) ^ 0xA001;
      else         crc >>= 1;
    }
  }
  return crc;
}
