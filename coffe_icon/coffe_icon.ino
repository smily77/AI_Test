#include <M5Unified.h> // Include the M5Unified library for M5Stack devices

// --- Required Libraries ---
// M5Unified: This library is essential for M5Stack devices.
// Install via Arduino Library Manager: Search for "M5Unified" and install.
// It handles display, button, power management, and other peripherals for M5Stack.

// --- Display Icon Parameters ---
// The M5Stack AtomS3 has a 128x128 pixel ST7789 LCD display.
const int ICON_DRAW_WIDTH = 60;   // Desired drawing width of the coffee cup icon
const int ICON_DRAW_HEIGHT = 60;  // Desired drawing height of the coffee cup icon
int icon_pos_x;                   // X position to draw the icon (calculated in setup)
int icon_pos_y;                   // Y position to draw the icon (calculated in setup)

// --- Colors ---
// M5Unified internally uses TFT_eSPI color definitions.
// These are 16-bit (RGB565) color values.
#define COLOR_GREEN_CUP   TFT_GREEN  // Green color for the cup
#define COLOR_RED_CUP     TFT_RED    // Red color for the cup
#define COLOR_BACKGROUND  TFT_BLACK  // Background color of the screen

// --- Global Variables ---
uint16_t currentCupColor = COLOR_GREEN_CUP; // Stores the current color of the cup
bool lastButtonState = false;               // Tracks the previous state of the built-in button

// --- Function Prototypes ---
// Declares the function used to draw the coffee cup.
void drawCoffeeCup(int x, int y, uint16_t cupColor);

// -------------------------------------------------------------------
// setup()
// This function runs once when the M5Stack AtomS3 starts up.
// -------------------------------------------------------------------
void setup() {
  // Initialize M5Unified library for the M5Stack AtomS3.
  // This configures the display, button, and other built-in peripherals.
  M5.begin();

  // Set the display rotation if needed. Default rotation (0) is usually fine.
  // For AtomS3, 0 is typically portrait (USB up), 1 is landscape (USB right).
  // M5.Lcd.setRotation(0);

  // Clear the entire screen to the specified background color.
  M5.Lcd.fillScreen(COLOR_BACKGROUND);

  // Calculate the centered position for the icon based on screen dimensions.
  // M5.Lcd.width() and M5.Lcd.height() return the current screen resolution.
  icon_pos_x = (M5.Lcd.width() - ICON_DRAW_WIDTH) / 2;
  icon_pos_y = (M5.Lcd.height() - ICON_DRAW_HEIGHT) / 2;

  // Draw the initial green coffee cup on the screen.
  drawCoffeeCup(icon_pos_x, icon_pos_y, currentCupColor);
}

// -------------------------------------------------------------------
// loop()
// This function runs repeatedly after setup().
// -------------------------------------------------------------------
void loop() {
  // Update M5Unified to read the current state of buttons, IMU, etc.
  // This must be called regularly for button state changes to be detected.
  M5.update();

  // Check if the built-in button (BtnA) of the AtomS3 is currently pressed.
  // M5.BtnA refers to the single programmable button on the AtomS3.
  bool buttonIsPressed = M5.BtnA.isPressed();

  // Only redraw the cup if the button state has changed.
  // This prevents unnecessary screen updates and reduces flickering.
  if (buttonIsPressed != lastButtonState) {
    // Button state has changed, determine the new color.
    if (buttonIsPressed) {
      currentCupColor = COLOR_RED_CUP;    // Button is pressed, set color to red.
    } else {
      currentCupColor = COLOR_GREEN_CUP;  // Button is released, set color to green.
    }

    // Redraw the coffee cup with the newly determined color.
    drawCoffeeCup(icon_pos_x, icon_pos_y, currentCupColor);

    // Update the last button state for the next comparison.
    lastButtonState = buttonIsPressed;
  }

  // A small delay to provide some debouncing for the button
  // and reduce CPU load, although M5.update() handles some debouncing.
  delay(50);
}

// -------------------------------------------------------------------
// drawCoffeeCup Function
// Draws a simple coffee cup icon on the M5Stack AtomS3 display using
// primitive drawing functions.
// x, y: Top-left coordinates of the icon's bounding box on the screen.
// cupColor: The 16-bit RGB565 color to use for drawing the cup.
// -------------------------------------------------------------------
void drawCoffeeCup(int x, int y, uint16_t cupColor) {
  // Clear the area where the icon will be drawn to ensure previous drawing is gone.
  // This is important when changing colors to avoid artifacts.
  M5.Lcd.fillRect(x, y, ICON_DRAW_WIDTH, ICON_DRAW_HEIGHT, COLOR_BACKGROUND);

  // --- Draw the cup body ---
  // A rounded rectangle for the main part of the cup.
  // Arguments: x-coord, y-coord, width, height, corner_radius, color
  M5.Lcd.fillRoundRect(x + 10, y + 15, 30, 35, 8, cupColor);

  // --- Draw the cup rim ---
  // A simple rectangle on top of the body to form the rim.
  // Arguments: x-coord, y-coord, width, height, color
  M5.Lcd.fillRect(x + 8, y + 10, 34, 6, cupColor);

  // --- Draw the cup handle ---
  // A simple rectangle for the handle.
  // Arguments: x-coord, y-coord, width, height, color
  M5.Lcd.fillRect(x + 40, y + 20, 10, 20, cupColor);

  // Optional: Add a small steam cloud for visual effect
  // This requires drawing multiple small circles or ovals.
  // M5.Lcd.fillCircle(x + 24, y + 5, 3, TFT_LIGHTGREY);
  // M5.Lcd.fillCircle(x + 28, y + 2, 2, TFT_LIGHTGREY);
}