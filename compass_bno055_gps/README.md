# Compass_BNO055_GPS

## Project Description

Wir haben einen ESP32-S3 Dieser hat einen TFT. Mit #include <CYD_Display_Config.h> wird die LGFX Klasse für den Schirm und Touchscreen initialisiert und definiert, unter Verwendung der der LovyanGFX (Das zugehörige Objekt musst du im Code generieren (z.B. LGFX lcd;) Angeschlossen auch ein Quectel_L80-R GPS der am Pin 8 mit 9600 Baud empfangen wird.  Zusätzlich ist noch ein BNO055 über I2C angeschlossen Adresse 0x29, SDA Pin 1, SCL Pin 2. (Achtung der BNO ist kopfüber eingebaut : Z-Achse zeig nach unten)
Ich möchte, dass du auf dem Display eine Kompass-Windrosen mit zwei Zeigern anzeigst die die Richtung angeben. Einer basierend auf dem BNO055 und einer basierend auf dem GPS. (Ich möchte sie so vergleichen können). Zeige GPS-Fix Status mit aktuell empfangenen Satelliten und BNO Kalibrierungsstatus an, verwende Landscape