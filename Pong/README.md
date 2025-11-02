# 12_Pong - Klassisches Pong-Spiel

Klassisches 2-Spieler Pong-Spiel für CYD mit **Analog-Regler Steuerung**.

## Hardware-Anforderungen

### Standard CYD Display
- ESP32-2432S028R (320x240)
- ILI9341 Display

### Spezielle Hardware: 2x Analog-Regler
- **GPIO 34**: Linker Spieler (Potentiometer)
- **GPIO 35**: Rechter Spieler (Potentiometer)

> **Hinweis**: Dieses Beispiel benötigt ein **spezielles CYD-Board** mit zwei analogen Reglern (Potentiometern).

## Spielprinzip

### Klassisches Pong
- 2 Spieler (links vs. rechts)
- Jeder steuert einen Schläger (Paddle) hoch/runter
- Ball prallt an Schlägern und Wänden ab
- Trifft Ball links/rechts außerhalb: Punkt für Gegner
- Score-Anzeige oben

### Steuerung
```
Linker Spieler  : Analog-Regler GPIO 34
Rechter Spieler : Analog-Regler GPIO 35

Regler drehen → Schläger bewegt sich vertikal
```

## Features

### Gameplay
- ⚡ **Schnelles Gameplay**: ~60 FPS
- 🎯 **Präzise Steuerung**: Analog-Input für smooth movement
- 📊 **Score-System**: Zählt Punkte für beide Spieler
- 🎱 **Ball-Physik**:
  - Konstante Geschwindigkeit
  - Prallt an Wänden ab
  - Winkel abhängig von Treffpunkt am Schläger
  - Zufällige Start-Richtung

### Visuals
- 🎨 **Retro-Design**: Klassisches Schwarz/Weiß Pong-Look
- 💙 **Cyan Schläger**: Gut sichtbare Paddle
- 💛 **Gelber Ball**: Leicht zu verfolgen
- ⚪ **Mittellinie**: Gestrichelte Linie (klassisch)
- 📈 **Score-Anzeige**: Groß und lesbar

### Technisch
- **Effizientes Rendering**: Nur veränderte Bereiche neu zeichnen
- **Float-Positionen**: Smooth ball movement
- **Collision Detection**: Paddle- und Wand-Kollision
- **Serial Debug**: Tore und Treffer werden ausgegeben

## Code-Struktur

### Game Objects
```cpp
struct Paddle {
  int x, y;      // Position
  int w, h;      // Größe
  uint16_t color;
};

struct Ball {
  float x, y;    // Position (float für smooth movement)
  float vx, vy;  // Geschwindigkeit
  int size;
  uint16_t color;
};
```

### Haupt-Funktionen
```cpp
void initGame()        // Spiel initialisieren
void resetBall()       // Ball nach Tor zurücksetzen
void updatePaddles()   // Analog-Inputs lesen
void updateBall()      // Ball-Position aktualisieren
void checkCollisions() // Kollisionen prüfen
void drawGame()        // Alles zeichnen
```

### Game Loop
```cpp
void loop() {
  updatePaddles();     // Schläger von Analog-Inputs
  updateBall();        // Ball bewegen
  checkCollisions();   // Treffer & Tore
  drawGame();          // Rendern
  delay(16);           // ~60 FPS
}
```

## Spielfeld-Layout

```
    Score Links        Score Rechts
        3                  5

┌──────────────────────────────────┐
│          :                       │
│  █       :                    █  │  ← Schläger
│  █       :                    █  │    (Paddels)
│  █       :                    █  │
│  █       :        ■           █  │  ← Ball
│  █       :                    █  │
│  █       :                    █  │
│          :                       │
└──────────────────────────────────┘
  ↑                               ↑
  GPIO 34                    GPIO 35
  (Linker Regler)      (Rechter Regler)
```

### Dimensionen
- **Display**: 320 x 240 Pixel
- **Schläger**: 5 x 60 Pixel
- **Ball**: 6 x 6 Pixel
- **Ball-Speed**: 3.5 px/frame horizontal, ±2 px/frame vertikal

## Spielregeln

### Punktevergabe
1. Ball trifft **linke Seite** raus → **+1 für rechten Spieler**
2. Ball trifft **rechte Seite** raus → **+1 für linken Spieler**

### Ball-Verhalten
- **Wand-Kollision**: Ball prallt ab (oben/unten)
- **Schläger-Kollision**: Ball prallt ab, Winkel abhängig von Treffpunkt
- **Nach Tor**: Ball resettet in Bildschirm-Mitte
- **Start-Richtung**: Zufällig links oder rechts

### Schläger-Bewegung
- **Analog-Input**: 0-4095 (12-bit ADC)
- **Mapping**: Analog-Wert → Y-Position (0 bis 180)
- **Smooth**: Direkte Analog-Steuerung ohne Verzögerung

## Hardware-Setup

### Potentiometer anschließen

#### Linker Regler (GPIO 34)
```
Potentiometer:
  [VCC] ──── 3.3V
  [SIG] ──── GPIO 34
  [GND] ──── GND
```

#### Rechter Regler (GPIO 35)
```
Potentiometer:
  [VCC] ──── 3.3V
  [SIG] ──── GPIO 35
  [GND] ──── GND
```

> **Wichtig**: Verwende 3.3V, **NICHT** 5V!

### GPIO-Eigenschaften
- GPIO 34 & 35 sind **Input-Only** Pins
- Haben **keinen** internen Pull-Up/Pull-Down
- Perfekt für Analog-Inputs
- 12-bit ADC (0-4095)

## Installation

1. **CYD_Display_Config.h** im Root-Verzeichnis vorhanden?
2. **LovyanGFX** Library installiert?
3. **Potentiometer** an GPIO 34 & 35 angeschlossen?
4. **Sketch hochladen**
5. **Spielen!** 🎮

## Spieltipps

### Für Anfänger
- Paddle in der **Mitte** halten
- Ball mit **Paddle-Mitte** treffen
- Nicht zu schnell bewegen

### Für Profis
- Winkel mit **Paddle-Rand** ändern
- Gegner in **Ecke** drängen
- **Spin-Shots** mit Paddle-Kante

## Anpassungen

### Ball schneller machen
```cpp
// In resetBall():
ball.vx = (random(0, 2) == 0) ? -4.5 : 4.5;  // statt -3.5/3.5
```

### Schläger größer machen
```cpp
// In initGame():
paddleLeft.h = 80;   // statt 60
paddleRight.h = 80;  // statt 60
```

### Farben ändern
```cpp
#define COLOR_PADDLE  0xF800  // Rot statt Cyan
#define COLOR_BALL    0x07E0  // Grün statt Gelb
```

### Gewinn-Limit
```cpp
// In checkCollisions() nach scoreLeft++ bzw. scoreRight++:
if (scoreLeft >= 10) {
  // Linker Spieler gewinnt!
  lcd.fillScreen(COLOR_BG);
  lcd.setCursor(80, 120);
  lcd.print("LINKS GEWINNT!");
  while(1);  // Spiel stoppt
}
```

## Troubleshooting

### Schläger bewegt sich nicht
- ✅ Potentiometer richtig angeschlossen?
- ✅ GPIO 34 & 35 verwendet?
- ✅ 3.3V (nicht 5V)?
- ✅ Seriellen Monitor öffnen - Analog-Werte ausgeben lassen

### Schläger zittert
- Potentiometer eventuell defekt
- Versuche Software-Filter:
  ```cpp
  int filtered = (lastValue * 3 + newValue) / 4;
  ```

### Ball zu langsam/schnell
- Ändere `ball.vx` und `ball.vy` in `resetBall()`
- Oder ändere `delay(16)` in `loop()`

### Display flackert
- Normal bei direktem Pixel-Drawing
- Für weniger Flackern: Double-Buffering implementieren

## Erweiterungsideen

### Easy
- [ ] **Gewinn-Limit**: Spiel endet bei 10 Punkten
- [ ] **Pause-Funktion**: Touch auf Mittellinie pausiert
- [ ] **Ball-Farbe** wechselt bei jedem Treffer

### Medium
- [ ] **Schwierigkeits-Level**: Ball wird schneller
- [ ] **Sound-Effekte**: Beep bei Treffer
- [ ] **Power-Ups**: Ball wird größer/kleiner

### Advanced
- [ ] **KI-Gegner**: Computer steuert rechten Schläger
- [ ] **Turnier-Modus**: Best of 5
- [ ] **Multiplayer-Levels**: Hindernisse im Spielfeld

## Credits

Basiert auf dem klassischen **Pong** (1972, Atari).

Implementiert für CYD mit analoger Steuerung.

## Viel Spaß beim Spielen! 🎮

```
     _____  ____  _   _  _____
    |  __ \/ __ \| \ | |/ ____|
    | |__) | |  | |  \| | |  __
    |  ___/| |  | | . ` | | |_ |
    | |    | |__| | |\  | |__| |
    |_|     \____/|_| \_|\_____|
```
