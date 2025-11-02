// Required Libraries:
// 1. LovyanGFX (install via Arduino Library Manager, search for "LovyanGFX" by lovyan03)
// 2. CYD_Display_Config.h (This is a custom configuration file specific to CYD boards.
//    It's usually provided as part of a board support package or needs to be placed
//    in your sketch folder or a library path if not part of a standard library.)
//    Please ensure this header file is available to your project.

#include <LovyanGFX.hpp> // Include the core LovyanGFX library
#include <CYD_Display_Config.h> // Include the CYD board specific configuration file

// Create a global LGFX object.
// The CYD_Display_Config.h file is expected to define a class named LGFX
// which inherits from lgfx::LGFX_Device and configures it for the CYD 2.8 board.
static LGFX tft;

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  Serial.println("Starting CYD 2.8 ESP32 gradient sketch with LovyanGFX...");

  // Initialize the TFT screen using LovyanGFX.
  // tft.begin() automatically applies the configuration defined in CYD_Display_Config.h,
  // including SPI pins, frequency, DMA, and backlight settings.
  tft.begin();

  // Set screen rotation: 0 = Portrait, 1 = Landscape, 2 = Reverse Portrait, 3 = Reverse Landscape
  // For this sketch, landscape (width 320, height 240) is chosen to make a horizontal gradient clear.
  tft.setRotation(1);

  // LovyanGFX typically handles backlight control internally if configured in CYD_Display_Config.h.
  // No explicit pinMode/digitalWrite is usually needed here.

  tft.fillScreen(tft.color565(0, 0, 0)); // Clear the screen to black using LovyanGFX's color conversion

  drawBlueGradient(); // Draw the blue gradient once in setup
}

void loop() {
  // The gradient is drawn once in setup().
  // The loop() function can be used for other tasks or remain empty if no animation is desired.
  delay(100); // Small delay to prevent watchdog timer issues if nothing else is running
}

// Function to draw a smooth blue gradient across the entire screen
void drawBlueGradient() {
  Serial.println("Drawing blue gradient...");

  // Define start and end colors for the gradient in RGB (0-255)
  // These will be converted to 16-bit RGB565 by tft.color565()

  // Start color: Very Dark Blue
  byte startR = 0;
  byte startG = 0;
  byte startB = 30; // A subtle dark blue

  // End color: Very Light Blue (almost cyan/sky blue)
  byte endR = 100;
  byte endG = 200;
  byte endB = 255; // A bright, light blue

  int screenWidth = tft.width();   // Get the actual screen width based on rotation
  int screenHeight = tft.height(); // Get the actual screen height based on rotation

  // Iterate through each column of the screen
  for (int x = 0; x < screenWidth; x++) {
    // Calculate the interpolated R, G, B components for the current column
    // The map() function works with 'long' types, so cast accordingly
    byte currentR = map(x, 0, screenWidth - 1, startR, endR);
    byte currentG = map(x, 0, screenWidth - 1, startG, endG);
    byte currentB = map(x, 0, screenWidth - 1, startB, endB);

    // Create the 16-bit color (RGB565 format) from the calculated R, G, B values
    // LovyanGFX also provides color565() for direct RGB565 conversion.
    uint16_t gradientColor = tft.color565(currentR, currentG, currentB);

    // Draw a vertical line for the current column with the calculated color
    // This efficiently draws the gradient from left to right
    tft.drawFastVLine(x, 0, screenHeight, gradientColor);
  }

  Serial.println("Gradient drawing complete.");
}