// Required Libraries:
// Adafruit NeoPixel Library:
//   Go to Sketch > Include Library > Manage Libraries...
//   Search for "Adafruit NeoPixel" and install it.

#include <Adafruit_NeoPixel.h>

// Default pin definition for the internal NeoPixel on ESP32-S3-DevKitC-1.
// For the ESP32-S3-DevKitC-1, the addressable RGB LED is commonly connected to GPIO 48.
// This pin will be used by the main loop() function.
// The setup() function will first iterate through various pins to help you identify the correct one.
#define NEOPIXEL_DEFAULT_PIN 48 

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
// This global Adafruit_NeoPixel object is instantiated with the NEOPIXEL_DEFAULT_PIN.
// Its 'begin()' method will be called after the pin testing phase in setup().
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_DEFAULT_PIN, NEO_GRB + NEO_KHZ800);

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
  Serial.println("ESP32-S3 Internal NeoPixel Pin Tester Started!");
  Serial.println("Monitoring GPIOs from " + String(NEOPIXEL_DEFAULT_PIN) + " downwards to 0.");
  Serial.println("Look for the NeoPixel to light up. Note the corresponding GPIO in Serial Monitor.");
  Serial.println("--------------------------------------------------------------------------------");

  // Loop through potential NeoPixel pins to help identify the correct one.
  // This will test pins from NEOPIXEL_DEFAULT_PIN (e.g., 48) down to 0.
  // When the correct pin is tested, the NeoPixel should light up.
  // Note: Using GPIOs like 0, 1, 2, etc., might interfere with boot-mode or USB serial on some ESP32 variants.
  // It's recommended to pick a higher GPIO number for actual use if possible.
  for (int testPin = NEOPIXEL_DEFAULT_PIN; testPin >= 0; testPin--) {
    Serial.print("Testing GPIO: ");
    Serial.println(testPin);

    // Create a temporary NeoPixel object for testing the current pin.
    // The NEO_GRB + NEO_KHZ800 parameters are assumed standard for most NeoPixels.
    Adafruit_NeoPixel testPixels(NUM_PIXELS, testPin, NEO_GRB + NEO_KHZ800);

    // Initialize the temporary NeoPixel object for the current pin.
    testPixels.begin();

    // Set the pixel to a distinct color (e.g., white) for a short period to make it visible.
    testPixels.setPixelColor(0, testPixels.Color(255, 255, 255)); // White color
    testPixels.show();
    delay(1000); // Keep lit for 1 second for observation

    // Turn off the pixel before moving to the next pin to clearly distinguish pins.
    testPixels.clear();
    testPixels.show();
    delay(200); // Short delay while off

    // The 'testPixels' object goes out of scope at the end of this loop iteration.
    // This effectively stops control of the current 'testPin' by this temporary object.
  }

  Serial.println("--------------------------------------------------------------------------------");
  Serial.println("Pin test complete. The main program will now run using the default pin (" + String(NEOPIXEL_DEFAULT_PIN) + ").");
  Serial.println("If you found your NeoPixel on a different pin during the test,");
  Serial.println("please update the 'NEOPIXEL_DEFAULT_PIN' definition at the top of the sketch to that pin number, then re-upload.");
  Serial.println("Starting ESP32-S3 Internal NeoPixel Color Cycle with default pin!");

  // Initialize the global NeoPixel library with the default pin for the main loop.
  // This is for the 'pixels' object that will be used by the loop() function.
  pixels.begin();
  // Clear all pixel data (turn them off) to ensure a clean start.
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