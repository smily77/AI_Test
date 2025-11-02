// Required Library: TFT_eSPI
// This library can be installed via the Arduino Library Manager.
// Search for "TFT_eSPI" and install the version by Bodmer.
//
// Board configuration for the "CYD 2.8 ESP32 DEV board" is included directly in this sketch.
// This is achieved by defining USER_SETUP_LOADED before including TFT_eSPI.h,
// and then providing all necessary TFT_eSPI configuration #defines.

#define USER_SETUP_LOADED // This ensures that TFT_eSPI uses our custom setup below

// Define the display driver for ILI9341
#define ILI9341_DRIVER

// Define the SPI pins for the CYD 2.8 ESP32 board
// These pins are typical for the ESP32 connected to an ILI9341 display
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15  // Chip Select for the TFT
#define TFT_DC   2   // Data/Command pin
#define TFT_RST  4   // Reset pin (can be tied to Arduino RESET pin if not needed)

// Backlight pin (may vary, 21 is common for CYD boards, 22 or 23 are also possible)
#define TFT_BL   21  // Backlight control pin

// Define the screen dimensions
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Optional: Define touch screen pins if a touch screen is present and you want to use it
// #define TOUCH_CS 33 // Touch Chip Select (e.g., for XPT2046)

// End of TFT_eSPI custom setup
#include <TFT_eSPI.h> // Include the graphics library

TFT_eSPI tft = TFT_eSPI(); // Create a TFT_eSPI object

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  Serial.println("Starting CYD 2.8 ESP32 gradient sketch...");

  tft.init();         // Initialize the TFT screen
  tft.setRotation(1); // Set screen rotation: 0 = Portrait, 1 = Landscape, 2 = Reverse Portrait, 3 = Reverse Landscape
                      // For this sketch, landscape (width 320, height 240) is chosen to make a horizontal gradient clear.

  // Optional: Enable backlight. If TFT_BL is defined, set it to output and HIGH.
  // Note: Some boards might control backlight differently (e.g., via PWM or inverted logic).
  if (TFT_BL != -1) { // Check if backlight pin is defined
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Turn on backlight (HIGH is usually ON)
  }

  tft.fillScreen(TFT_BLACK); // Clear the screen to black

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

  int screenWidth = tft.width(); // Get the actual screen width based on rotation
  int screenHeight = tft.height(); // Get the actual screen height based on rotation

  // Iterate through each column of the screen
  for (int x = 0; x < screenWidth; x++) {
    // Calculate the interpolated R, G, B components for the current column
    // The map() function works with 'long' types, so cast accordingly
    byte currentR = map(x, 0, screenWidth - 1, startR, endR);
    byte currentG = map(x, 0, screenWidth - 1, startG, endG);
    byte currentB = map(x, 0, screenWidth - 1, startB, endB);

    // Create the 16-bit color (RGB565 format) from the calculated R, G, B values
    uint16_t gradientColor = tft.color565(currentR, currentG, currentB);

    // Draw a vertical line for the current column with the calculated color
    // This efficiently draws the gradient from left to right
    tft.drawFastVLine(x, 0, screenHeight, gradientColor);
  }

  Serial.println("Gradient drawing complete.");
}