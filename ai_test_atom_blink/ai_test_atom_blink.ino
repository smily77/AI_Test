/*M5Stack Atom Blinking LED Example (Internal Pixel)
This code makes a specific pixel (at position 2,2) on the M5Stack Atom's
internal 5x5 RGB LED matrix blink with a 2-second ON and 2-second OFF cycle.
This version uses the FastLED library directly for LED control, as requested.
Board: M5Stack Atom (ESP32-PICO-D4)
Pin Configuration:
No external connections are needed, this uses the built-in 5x5 RGB LED matrix.
The pixel at (2,2) corresponds to index 12 in the 0-indexed, row-major sequence.
(e.g., (0,0) is index 0, (0,1) is index 1, (1,0) is index 5, (2,2) is index 12).
*/

#include <M5Atom.h>   // Required library for M5Stack Atom's built-in features, including Serial and other core components.
#include <FastLED.h> // Required library for controlling addressable LEDs like WS2812B

// --- FastLED Configuration for M5Stack Atom Internal LED ---
#define NUM_LEDS 25       // 5x5 matrix has 25 LEDs
#define LED_DATA_PIN 27   // M5Stack Atom internal LED is typically connected to GPIO27
#define LED_TYPE WS2812B // Type of LED chip (e.g., WS2812B, NEOPIXEL)
#define COLOR_ORDER GRB   // Color order for the LEDs (Green, Red, Blue is common for WS2812B)

// Define the array of leds. This is where we'll store the color of each LED.
CRGB leds[NUM_LEDS];

// Define the index of the target pixel on the 5x5 matrix.
// For a 5x5 matrix, (row, col) to index is: index = row * 5 + col
// So, for (2,2): index = 2 * 5 + 2 = 12.
const int targetPixelIndex = 12;

// Define the blink durations in milliseconds.
const long onTime = 2000;  // LED ON duration (milliseconds)
const long offTime = 2000; // LED OFF duration (milliseconds)

void setup() {
  // Initialize the M5Atom board. This call initializes Serial communication
  // and other core components. We will manually initialize FastLED for the LEDs.
  M5.begin();

  // A small delay after M5.begin() can help stabilize internal peripherals.
  delay(50);

  // --- FastLED Initialization ---
  // Add the LED strip to FastLED.
  // This tells FastLED what type of LEDs you're using, which pin they're on,
  // their color order, and how many there are.
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  // Set the global brightness for the entire LED strip.
  // Value can be from 0 (off) to 255 (max brightness).
  FastLED.setBrightness(50);

  // Clear all LEDs in the 'leds' array to black (off) and show it on the physical display.
  FastLED.clear(); // Set all pixels in the 'leds' array buffer to black
  FastLED.show();  // Push the cleared buffer to the physical display

  Serial.println("M5Stack Atom Internal Pixel Blinking started using FastLED...");
  Serial.print("Targeting internal LED matrix pixel at index: ");
  Serial.println(targetPixelIndex);
}

void loop() {
  // Turn the target pixel ON by setting its color in the 'leds' array buffer.
  // CRGB::White is a predefined FastLED color. You can also use CRGB(R, G, B) or 0xRRGGBB.
  leds[targetPixelIndex] = CRGB::White; // Set pixel 12 to White
  // After updating LED data in the array, you must call FastLED.show() to refresh the physical display.
  FastLED.show();
  Serial.print("Pixel ");
  Serial.print(targetPixelIndex);
  Serial.print(" ON for ");
  Serial.print(onTime);
  Serial.println(" ms");

  // Wait for the specified ON duration.
  delay(onTime);

  // Turn the target pixel OFF by setting its color to black in the 'leds' array buffer.
  leds[targetPixelIndex] = CRGB::Black; // Set pixel 12 to Black (OFF)
  // After updating LED data in the array, you must call FastLED.show() to refresh the physical display.
  FastLED.show();
  Serial.print("Pixel ");
  Serial.print(targetPixelIndex);
  Serial.print(" OFF for ");
  Serial.print(offTime);
  Serial.println(" ms");

  // Wait for the specified OFF duration.
  delay(offTime);
}