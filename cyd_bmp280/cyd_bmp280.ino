#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// ---------------------------------------------------------------------------------
// BIBLIOTHEK-INSTALLATIONSHINWEISE:
// ---------------------------------------------------------------------------------
// 1. TFT_eSPI Bibliothek:
//    - Öffnen Sie die Arduino IDE -> Sketch -> Bibliotheken einbinden -> Bibliotheken verwalten...
//    - Suchen Sie nach "TFT_eSPI" und installieren Sie die Bibliothek von Bodmer.
//    - WICHTIG: Nach der Installation MÜSSEN Sie die Datei User_Setup.h
//      im Ordner der TFT_eSPI Bibliothek konfigurieren.
//      - Navigieren Sie zu <Arduino Sketchbook Ordner>/libraries/TFT_eSPI/
//      - Öffnen Sie User_Setup.h
//      - Finden und entkommentieren Sie die Zeile, die zu Ihrem ESP32 CYD Board passt,
//        z.B. `#include <User_Setups/Setup12_ILI9341_CYD.h>` oder ähnlich für ILI9341
//        oder ST7789 basierte CYDs.
//      - Kommentieren Sie alle anderen `#include <User_Setups/...>` Zeilen aus.
//      - Speichern Sie die Datei User_Setup.h.
//
// 2. Adafruit BMP280 Bibliothek:
//    - Öffnen Sie die Arduino IDE -> Sketch -> Bibliotheken einbinden -> Bibliotheken verwalten...
//    - Suchen Sie nach "Adafruit BMP280" und installieren Sie diese.
//
// 3. Adafruit Unified Sensor Bibliothek:
//    - Dies ist eine Abhängigkeit für Adafruit BMP280. Falls nicht automatisch installiert,
//      suchen Sie nach "Adafruit Unified Sensor" und installieren Sie diese.
//
// ---------------------------------------------------------------------------------

// Objekt für das TFT_eSPI Display erstellen
TFT_eSPI tft = TFT_eSPI();

// Objekt für den BMP280 Sensor erstellen
Adafruit_BMP280 bmp;

// I2C-Adresse des BMP280 Sensors (Standard ist 0x76, einige Module könnten 0x77 haben)
// Möglicherweise müssen Sie diese ändern, wenn Ihr Sensor eine andere Adresse verwendet.
#define BMP_ADDRESS 0x76

void setup() {
  // Serielle Kommunikation für Debugging initialisieren
  Serial.begin(115200);
  while (!Serial); // Warten, bis die serielle Verbindung hergestellt ist (nützlich für einige Boards)

  Serial.println("ESP32 CYD & BMP280 - Temperatur- und Luftdruckanzeige");

  // --- Initialisierung des TFT Displays ---
  tft.init(); // TFT_eSPI Display initialisieren

  // NEU: Helligkeit des Displays einstellen. Der ESP32 CYD verwendet PWM für die Hintergrundbeleuchtung.
  // Der Wert 255 setzt die maximale Helligkeit. Passen Sie ihn bei Bedarf an.
  // Diese Funktion nutzt den in User_Setup.h definierten TFT_BL Pin.
  tft.setBrightness(255);

  // Display-Rotation einstellen (0 bis 3). Bei Bedarf an Ihr Setup anpassen.
  // Rotation 1 oder 3 funktioniert normalerweise gut für Querformat auf dem CYD.
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK); // Gesamten Bildschirm mit Schwarz löschen
  tft.setTextFont(4); // Schriftgröße einstellen (4 ist eine gute Größe für die allgemeine Anzeige)
                      // Für weitere Schriftoptionen (1-7 oder benutzerdefiniert) siehe TFT_eSPI-Dokumentation.
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Textfarbe auf Weiß mit schwarzem Hintergrund einstellen

  tft.setCursor(0, 0); // Cursor in die obere linke Ecke setzen
  tft.println("Initialisiere BMP280...");
  Serial.println("Initialisiere BMP280...");

  // --- Initialisierung des BMP280 Sensors ---
  // I2C-Bus initialisieren. Für ESP32 sind die Standard-I2C-Pins GPIO 21 (SDA) und GPIO 22 (SCL).
  Wire.begin();

  // Versuch, den BMP280 Sensor zu initialisieren
  if (!bmp.begin(BMP_ADDRESS)) {
    Serial.println(F("Fehler: BMP280 Sensor nicht gefunden! Verkabelung oder I2C-Adresse pruefen."));
    tft.fillScreen(TFT_RED); // Fehler mit rotem Bildschirm anzeigen
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.println("BMP280 Fehler!");
    tft.println("Pruefen Sie");
    tft.println("Verkabelung!");
    tft.println("Adresse!");
    while (1); // Programm anhalten, wenn Sensor nicht gefunden wird
  }

  // BMP280 Sensoreinstellungen konfigurieren (optional, Standardeinstellungen sind oft ausreichend)
  // Diese Einstellungen balancieren Stromverbrauch, Rauschen und Reaktionszeit.
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Betriebsmodus: Normal (nimmt kontinuierlich Messungen vor)
                  Adafruit_BMP280::SAMPLING_X2,     // Temperatur-Oversampling: x2
                  Adafruit_BMP280::SAMPLING_X16,    // Druck-Oversampling: x16 (höhere Auflösung)
                  Adafruit_BMP280::FILTER_X16,      // IIR-Filter: x16 (reduziert Rauschen)
                  Adafruit_BMP280::STANDBY_MS_500); // Standby-Zeit: 500ms zwischen Messungen

  tft.fillScreen(TFT_BLACK); // Bildschirm nach den Initialisierungsmeldungen löschen
  Serial.println("BMP280 erfolgreich initialisiert!");
}

void loop() {
  // --- Sensorwerte lesen ---
  // Temperatur in Celsius lesen
  float temperature_c = bmp.readTemperature();
  // Druck in Pascal (Pa) lesen und in Hektopascal (hPa) umrechnen
  // 1 hPa = 100 Pa
  float pressure_hPa = bmp.readPressure() / 100.0F;

  // --- Werte zur seriellen Überwachung für Debugging ausgeben ---
  Serial.print(F("Temperatur = "));
  Serial.print(temperature_c);
  Serial.println(F(" *C"));

  Serial.print(F("Luftdruck = "));
  Serial.print(pressure_hPa);
  Serial.println(F(" hPa"));
  Serial.println();

  // --- Werte auf dem TFT-Bildschirm anzeigen ---
  // Cursor für die erste Zeile oben links setzen
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Sicherstellen, dass der Text weiß auf schwarzem Hintergrund ist

  // Temperatur anzeigen
  tft.print(F("Temperatur: "));
  tft.print(temperature_c, 2); // Temperatur mit 2 Nachkommastellen anzeigen
  tft.println(F(" C"));

  // Luftdruck in der nächsten Zeile anzeigen
  // Um den Inhalt der vorherigen Zeile vollständig zu löschen, müsste man mit Leerzeichen überschreiben
  // oder tft.fillRect verwenden, wenn sich die Werte in der Länge erheblich ändern.
  // Für die Einfachheit hier funktioniert tft.print dann tft.println im Allgemeinen gut für feste Längen.
  tft.print(F("Luftdruck:  ")); // Leerzeichen hinzugefügt, um sich mit "Temperatur" auszurichten
  tft.print(pressure_hPa, 2);  // Druck mit 2 Nachkommastellen anzeigen
  tft.println(F(" hPa"));

  // Eine Verzögerung vor der nächsten Messung hinzufügen
  delay(2000); // Alle 2 Sekunden aktualisieren
}