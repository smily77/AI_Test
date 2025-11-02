/*
  Pong Game - Klassisches 2-Spieler Pong

  Hardware:
  - CYD Display (320x240)
  - 2x Analog-Regler (Potentiometer):
    * GPIO 34: Linker Spieler
    * GPIO 35: Rechter Spieler

  Steuerung:
  - Linker Regler: Bewegt linken Schläger hoch/runter
  - Rechter Regler: Bewegt rechten Schläger hoch/runter

  Spielregeln:
  - Ball prallt an Schlägern und Wänden ab
  - Trifft Ball links/rechts raus: Punkt für Gegner
  - Spiel läuft bis Reset
*/

#include <CYD_Display_Config.h>

// Analog Input Pins
#define POT_LEFT  34    // Linker Regler
#define POT_RIGHT 35    // Rechter Regler

// Display Objekt
LGFX lcd;

// Spielfeld-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Farben
#define COLOR_BG      0x0000  // Schwarz
#define COLOR_FG      0xFFFF  // Weiß
#define COLOR_PADDLE  0x07FF  // Cyan
#define COLOR_BALL    0xFFE0  // Gelb
#define COLOR_LINE    0x4208  // Dunkelgrau

// Schläger (Paddle) Struktur
struct Paddle {
  int x, y;
  int w, h;
  uint16_t color;
};

// Ball Struktur
struct Ball {
  float x, y;      // Position (float für smooth movement)
  float vx, vy;    // Geschwindigkeit
  int size;
  uint16_t color;
};

// Game Objekte
Paddle paddleLeft;
Paddle paddleRight;
Ball ball;

// Score
int scoreLeft = 0;
int scoreRight = 0;

// Alte Positionen für effizientes Löschen
int oldPaddleLeftY = 0;
int oldPaddleRightY = 0;
float oldBallX = 0;
float oldBallY = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== PONG GAME ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Analog Inputs konfigurieren
  pinMode(POT_LEFT, INPUT);
  pinMode(POT_RIGHT, INPUT);

  // Spiel initialisieren
  initGame();

  // Spielfeld zeichnen
  drawField();
  drawScore();

  Serial.println("Spiel gestartet!");
  Serial.println("Linker Regler: GPIO 34");
  Serial.println("Rechter Regler: GPIO 35");
}

void loop() {
  // Schläger-Positionen von Analog-Inputs lesen
  updatePaddles();

  // Ball bewegen
  updateBall();

  // Kollisionen prüfen
  checkCollisions();

  // Alles zeichnen
  drawGame();

  // Frame-Rate ~60 FPS
  delay(16);
}

// ===== INITIALISIERUNG =====

void initGame() {
  // Linker Schläger
  paddleLeft.x = 10;
  paddleLeft.y = SCREEN_HEIGHT / 2 - 30;
  paddleLeft.w = 5;
  paddleLeft.h = 60;
  paddleLeft.color = COLOR_PADDLE;

  // Rechter Schläger
  paddleRight.x = SCREEN_WIDTH - 15;
  paddleRight.y = SCREEN_HEIGHT / 2 - 30;
  paddleRight.w = 5;
  paddleRight.h = 60;
  paddleRight.color = COLOR_PADDLE;

  // Ball in der Mitte
  resetBall();

  // Score
  scoreLeft = 0;
  scoreRight = 0;

  // Alte Positionen initialisieren
  oldPaddleLeftY = paddleLeft.y;
  oldPaddleRightY = paddleRight.y;
  oldBallX = ball.x;
  oldBallY = ball.y;
}

void resetBall() {
  ball.x = SCREEN_WIDTH / 2;
  ball.y = SCREEN_HEIGHT / 2;
  ball.size = 6;
  ball.color = COLOR_BALL;

  // Zufällige Start-Richtung
  ball.vx = (random(0, 2) == 0) ? -3.5 : 3.5;
  ball.vy = random(-2, 3);

  delay(500);  // Kurze Pause nach Reset
}

// ===== SPIELFELD ZEICHNEN =====

void drawField() {
  // Mittellinie (gestrichelt)
  int centerX = SCREEN_WIDTH / 2;
  for (int y = 0; y < SCREEN_HEIGHT; y += 10) {
    lcd.fillRect(centerX - 1, y, 2, 5, COLOR_LINE);
  }
}

void drawScore() {
  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_FG, COLOR_BG);

  // Linker Score
  lcd.setCursor(SCREEN_WIDTH / 4 - 20, 10);
  lcd.printf("%d", scoreLeft);

  // Rechter Score
  lcd.setCursor(SCREEN_WIDTH * 3 / 4 - 20, 10);
  lcd.printf("%d", scoreRight);
}

// ===== UPDATE FUNKTIONEN =====

void updatePaddles() {
  // Alte Positionen speichern
  oldPaddleLeftY = paddleLeft.y;
  oldPaddleRightY = paddleRight.y;

  // Analog-Werte lesen (0-4095)
  int potLeftValue = analogRead(POT_LEFT);
  int potRightValue = analogRead(POT_RIGHT);

  // In Y-Position umrechnen (0 bis SCREEN_HEIGHT - paddle height)
  paddleLeft.y = map(potLeftValue, 1000, 0, 0, SCREEN_HEIGHT - paddleLeft.h);
  paddleRight.y = map(potRightValue, 1000, 0, 0, SCREEN_HEIGHT - paddleRight.h);

  // Grenzen prüfen
  paddleLeft.y = constrain(paddleLeft.y, 0, SCREEN_HEIGHT - paddleLeft.h);
  paddleRight.y = constrain(paddleRight.y, 0, SCREEN_HEIGHT - paddleRight.h);
}

void updateBall() {
  // Alte Position speichern
  oldBallX = ball.x;
  oldBallY = ball.y;

  // Ball bewegen
  ball.x += ball.vx;
  ball.y += ball.vy;

  // Kollision mit oberer/unterer Wand
  if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - ball.size) {
    ball.vy = -ball.vy;
    ball.y = constrain(ball.y, 0, SCREEN_HEIGHT - ball.size);
  }
}

void checkCollisions() {
  // Kollision mit linkem Schläger
  if (ball.x <= paddleLeft.x + paddleLeft.w &&
      ball.x >= paddleLeft.x &&
      ball.y + ball.size >= paddleLeft.y &&
      ball.y <= paddleLeft.y + paddleLeft.h) {

    ball.vx = -ball.vx;
    ball.x = paddleLeft.x + paddleLeft.w;

    // Winkel abhängig von Treffpunkt
    float hitPos = (ball.y + ball.size/2) - (paddleLeft.y + paddleLeft.h/2);
    ball.vy += hitPos * 0.05;

    Serial.println("Hit: Linker Schläger");
  }

  // Kollision mit rechtem Schläger
  if (ball.x + ball.size >= paddleRight.x &&
      ball.x + ball.size <= paddleRight.x + paddleRight.w &&
      ball.y + ball.size >= paddleRight.y &&
      ball.y <= paddleRight.y + paddleRight.h) {

    ball.vx = -ball.vx;
    ball.x = paddleRight.x - ball.size;

    // Winkel abhängig von Treffpunkt
    float hitPos = (ball.y + ball.size/2) - (paddleRight.y + paddleRight.h/2);
    ball.vy += hitPos * 0.05;

    Serial.println("Hit: Rechter Schläger");
  }

  // Ball links raus - Punkt für Rechts
  if (ball.x < 0) {
    scoreRight++;
    Serial.printf("PUNKT! Rechts: %d - Links: %d\n", scoreRight, scoreLeft);
    drawScore();
    resetBall();
  }

  // Ball rechts raus - Punkt für Links
  if (ball.x > SCREEN_WIDTH) {
    scoreLeft++;
    Serial.printf("PUNKT! Links: %d - Rechts: %d\n", scoreLeft, scoreRight);
    drawScore();
    resetBall();
  }
}

// ===== ZEICHNEN =====

void drawGame() {
  // Linken Schläger löschen (alte Position) und neu zeichnen
  if (oldPaddleLeftY != paddleLeft.y) {
    lcd.fillRect(paddleLeft.x, oldPaddleLeftY, paddleLeft.w, paddleLeft.h, COLOR_BG);
    lcd.fillRect(paddleLeft.x, paddleLeft.y, paddleLeft.w, paddleLeft.h, paddleLeft.color);
  }

  // Rechten Schläger löschen (alte Position) und neu zeichnen
  if (oldPaddleRightY != paddleRight.y) {
    lcd.fillRect(paddleRight.x, oldPaddleRightY, paddleRight.w, paddleRight.h, COLOR_BG);
    lcd.fillRect(paddleRight.x, paddleRight.y, paddleRight.w, paddleRight.h, paddleRight.color);
  }

  // Ball löschen (alte Position) und neu zeichnen
  lcd.fillRect(oldBallX, oldBallY, ball.size, ball.size, COLOR_BG);
  lcd.fillRect(ball.x, ball.y, ball.size, ball.size, ball.color);
}
