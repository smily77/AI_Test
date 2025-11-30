#include <Wire.h> // Required for I2C communication with BMP280
#include <SPI.h>  // TFT_eSPI library uses SPI for communication with the display controller
#include <TFT_eSPI.h> // Hardware-specific library for the ESP32 CYD display
#include <Adafruit_Sensor.h> // Required for Adafruit Unified Sensor framework
#include <Adafruit_BMP280.h> // Library for the BMP280 pressure and temperature sensor

// ---------------------------------------------------------------------------------
// BIBLIOTHEK-INSTALLATIONSHINWEISE:
// ---------------------------------------------------------------------------------
// 1. TFT_eSPI Bibliothek:
//    - �ffnen Sie die Arduino IDE -> Sketch -> Bibliotheken einbinden -> Bibliotheken verwalten...
//    - Suchen Sie nach "TFT_eSPI" und installieren Sie die Bibliothek von Bodmer.
//    - WICHTIG: Nach der Installation M�SSEN Sie die Datei User_Setup.h
//      im Ordner der TFT_eSPI Bibliothek konfigurieren.
//      - Navigieren Sie zu <Arduino Sketchbook Ordner>/libraries/TFT_eSPI/
//      - �ffnen Sie User_Setup.h
//      - Finden und entkommentieren Sie die Zeile, die zu Ihrem ESP32 CYD Board passt,
//        z.B. `#include <User_Setups/Setup12_ILI9341_CYD.h>` oder �hnlich f�r ILI9341
//        oder ST7789 basierte CYDs.
//      - Kommentieren Sie alle anderen `#include <User_Setups/...>` Zeilen aus.
//      - Speichern Sie die Datei User_Setup.h.
//
// 2. Adafruit BMP280 Bibliothek:
//    - �ffnen Sie die Arduino IDE -> Sketch -> Bibliotheken einbinden -> Bibliotheken verwalten...
//    - Suchen Sie nach "Adafruit BMP280" und installieren Sie diese.
//
// 3. Adafruit Unified Sensor Bibliothek:
//    - Dies ist eine Abh�ngigkeit f�r Adafruit BMP280. Falls nicht automatisch installiert,
//      suchen Sie nach "Adafruit Unified Sensor" und installieren Sie diese.
//
// ---------------------------------------------------------------------------------

// Objekt f�r das TFT_eSPI Display erstellen
TFT_eSPI tft = TFT_eSPI();

// Objekt f�r den BMP280 Sensor erstellen
Adafruit_BMP280 bmp;

// I2C-Adresse des BMP280 Sensors (Standard ist 0x76, einige Module k�nnten 0x77 haben)
// M�glicherweise m�ssen Sie diese �ndern, wenn Ihr Sensor eine andere Adresse verwendet.
#define BMP_ADDRESS 0x76

void setup() {
  // Serielle Kommunikation f�r Debugging initialisieren
  Serial.begin(115200);
  while (!Serial); // Warten, bis die serielle Verbindung hergestellt ist (n�tzlich f�r einige Boards)
  pinMode(21,OUTPUT);
digitalWrite(21,HIGH);
  Serial.println("ESP32 CYD & BMP280 - Temperatur- und Luftdruckanzeige");

  // --- Initialisierung des TFT Displays ---
  tft.init(); // TFT_eSPI Display initialisieren
  // Display-Rotation einstellen (0 bis 3). Bei Bedarf an Ihr Setup anpassen.
  // Rotation 1 oder 3 funktioniert normalerweise gut f�r Querformat auf dem CYD.
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK); // Gesamten Bildschirm mit Schwarz l�schen
  tft.setTextFont(4); // Schriftgr��e einstellen (4 ist eine gute Gr��e f�r die allgemeine Anzeige)
                      // F�r weitere Schriftoptionen (1-7 oder benutzerdefiniert) siehe TFT_eSPI-Dokumentation.
  tft.setTextColor(0xFFFFFF, TFT_BLACK); // Textfarbe auf Wei� mit schwarzem Hintergrund einstellen

  tft.setCursor(0, 0); // Cursor in die obere linke Ecke setzen
  tft.println("Initialisiere BMP280...");
  Serial.println("Initialisiere BMP280...");

  // --- Initialisierung des BMP280 Sensors ---
  // I2C-Bus initialisieren. F�r ESP32 sind die Standard-I2C-Pins GPIO 21 (SDA) und GPIO 22 (SCL).
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
                  Adafruit_BMP280::SAMPLING_X16,    // Druck-Oversampling: x16 (h�here Aufl�sung)
                  Adafruit_BMP280::FILTER_X16,      // IIR-Filter: x16 (reduziert Rauschen)
                  Adafruit_BMP280::STANDBY_MS_500); // Standby-Zeit: 500ms zwischen Messungen

  tft.fillScreen(TFT_BLACK); // Bildschirm nach den Initialisierungsmeldungen l�schen
  Serial.println("BMP280 erfolgreich initialisiert!");
}

void loop() {
  // --- Sensorwerte lesen ---
  // Temperatur in Celsius lesen
  float temperature_c = bmp.readTemperature();
  // Druck in Pascal (Pa) lesen und in Hektopascal (hPa) umrechnen
  // 1 hPa = 100 Pa
  float pressure_hPa = bmp.readPressure() / 100.0F;

  // --- Werte zur seriellen �berwachung f�r Debugging ausgeben ---
  Serial.print(F("Temperatur = "));
  Serial.print(temperature_c);
  Serial.println(F(" *C"));

  Serial.print(F("Luftdruck = "));
  Serial.print(pressure_hPa);
  Serial.println(F(" hPa"));
  Serial.println();

  // --- Werte auf dem TFT-Bildschirm anzeigen ---
  // Cursor f�r die erste Zeile oben links setzen
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Sicherstellen, dass der Text wei� auf schwarzem Hintergrund ist

  // Temperatur anzeigen
  tft.print(F("Temperatur: "));
  tft.print(temperature_c, 2); // Temperatur mit 2 Nachkommastellen anzeigen
  tft.println(F(" C"));

  // Luftdruck in der n�chsten Zeile anzeigen
  // Um den Inhalt der vorherigen Zeile vollst�ndig zu l�schen, m�sste man mit Leerzeichen �berschreiben
  // oder tft.fillRect verwenden, wenn sich die Werte in der L�nge erheblich �ndern.
  // F�r die Einfachheit hier funktioniert tft.print dann tft.println im Allgemeinen gut f�r feste L�ngen.
  tft.print(F("Luftdruck:  ")); // Leerzeichen hinzugef�gt, um sich mit "Temperatur" auszurichten
  tft.print(pressure_hPa, 2);  // Druck mit 2 Nachkommastellen anzeigen
  tft.println(F(" hPa"));

  // Eine Verz�gerung vor der n�chsten Messung hinzuf�gen
  delay(2000); // Alle 2 Sekunden aktualisieren
}