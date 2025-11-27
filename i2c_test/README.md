# I2C Test

## Project Description

Ich habe ein ESP32-S3 Board mit angeschlossenem TFT Display.  Die komplette Definition für die LovyanGFX geschieht über den Befehl #include <CYD_Display_Config.h> Dabei wird für die LovyanGFX die LGFX Klasse für den Bildschim inkl. Touch definiert. (Womit auch die Auflösung des angeschlossenen Bildschirms abgefragt werden kann).
Das Programm ist ein Test Programm: An den Pin's 1 (SDA) und 2 (SCL) ist der  I2C Bus. Die Erste Funktion des Programmes ist die Adresse aller angeschlossenen I2C Geräte auf dem Bildschirm an zu zeigen. 
Die Zweite Funktion besteht darin, falls am I2C - BUS ein pcf8574 angeschlossen ist den Zustand aller 8 Eingänge laufend anzuzeigen