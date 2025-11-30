# CYD_with_BMP280

## Project Description

Wir haben ein CYD (ESP32). Mit #include <CYD_Display_Config.h> wird die LGFX Klasse für den Schirm und Touchscreen initialisiert und definiert, unter Verwendung der der LovyanGFX (Das zugehörige Objekt musst du im Code generieren (z.B. LGFX lcd;). Die I2C Pins sind auch CYD_Display_Config.h als extSDA und extSCL definiert. Sollten sie nicht definiert sein legst du sie als extSDA 22 und extSCL 27 im Code fest.

Angeschlossen ist auch ein BMP280 (0x77) und ein AHT20 (0x38) am I2C BUS. 
Mache eine Anzeige für Temperatur, Luftfeuchtigkeit und Luftdruck. Verwende darfür möglichst grosse, schöne Fonts. Zeichne auch den jeweils eine Graphik zu den drei Werten, die den Verlauf über ide letzten 24 Stunden wieder gibt