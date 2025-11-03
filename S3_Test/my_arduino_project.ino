// Required Libraries:
// Adafruit NeoPixel Library:
//   Go to Sketch > Include Library > Manage Libraries...
//   Search for "Adafruit NeoPixel" and install it.

#include <Adafruit_NeoPixel.h>

// Pin definition for the internal NeoPixel on ESP32-S3-DevKitC-1.
// For the ESP32-S3-DevKitC-1, the addressable RGB LED is commonly connected to GPIO 48.
#define PIN_NEOPIXEL 48
// Number of pixels in the strip. The ESP32-S3-DevKitC-1 has one internal RGB LED pixel.
#define NUM_PIXELS 1

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (GPIO 48 for ESP32-S3-DevKitC-1)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_GRB    Pixels are wired for GRB bitstream (most WS2812B/NeoPixels are GRB)
//   NEO_RGB    Pixels are wired for RGB bitstream
//   NEO_BRG    Pixels are wired for BRG bitstream
//   NEO_KHZ800 800 KHz bitstream (most WS2812B/NeoPixels are 800KHz)
//   NEO_KHZ400 400 KHz (older WS2811 pixels)
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Delay between color changes in milliseconds. Adjust for faster or slower transitions.
#define DELAY_MS 20

// Global variable to keep track of the current hue for the rainbow effect.
static uint8_t currentHue = 0;

// Helper function to convert a 0-255 value to a NeoPixel 24-bit color value (RGB).
// This function generates a "rainbow wheel" effect.
// Input a value 0 to 255 to get a color value.
// The colors transition from Red -> Green -> Blue -> Red.
uint32_t Wheel(byte WheelPos) {
  // Invert the position to get a standard rainbow flow.
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) { // Red -> Green phase
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) { // Green -> Blue phase
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  // Blue -> Red phase
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setup() {
  // Initialize Serial communication for debugging purposes.
  // This can be helpful to see messages in the Serial Monitor.
  Serial.begin(115200);
  Serial.println("ESP32-S3 Internal NeoPixel Color Cycle Started!");

  // Initialize the NeoPixel library. This prepares the GPIO pin.
  pixels.begin();
  // Clear all pixel data (turn them off).
  pixels.clear();
  // Push the cleared state to the pixel to ensure it's off initially.
  pixels.show();
}

void loop() {
  // Set the color of the first (and only) pixel (index 0) using the Wheel function.
  // The 'currentHue' value cycles from 0 to 255, creating a continuous rainbow effect.
  pixels.setPixelColor(0, Wheel(currentHue));

  // Update the pixel with the new color data. This sends the color to the LED.
  pixels.show();

  // Increment the hue for the next color in the sequence.
  currentHue++;
  // If the hue value exceeds 255, wrap it back to 0 to restart the color cycle.
  if (currentHue > 255) {
    currentHue = 0;
  }

  // Wait for a short period before changing to the next color.
  // This controls the speed of the color transition.
  delay(DELAY_MS);
}