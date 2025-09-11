// Required library: M5AtomS3
// This library can be installed via the Arduino Library Manager.
// Search for "M5AtomS3" and install the official M5Stack library.

#include <M5AtomS3.h> // Include the M5AtomS3 library for display and core functionalities

void setup() {
  // Initialize the M5AtomS3.
  // This command initializes the core components, including the display.
  M5.begin();

  // Seed the random number generator using an unconnected analog pin.
  // This helps ensure that the flickering pattern is unpredictable each time the board starts.
  randomSeed(analogRead(0));

  // Set the screen rotation if desired.
  // 0-3 are common values. Adjust as needed for your preferred orientation.
  M5.Lcd.setRotation(0);

  // Initially fill the screen with black to simulate the lamp being off before it starts flickering.
  M5.Lcd.fillScreen(TFT_BLACK);
}

void loop() {
  // --- Simulate the 'on' state (yellow light) ---
  M5.Lcd.fillScreen(TFT_YELLOW); // Turn the screen full yellow

  // Random delay for how long the light stays 'on'.
  // This simulates the varying duration of the lamp being lit, from brief flashes to slightly longer periods.
  // Values: 50ms (quick flash) to 200ms (brief steady light).
  delay(random(50, 200));

  // --- Simulate the 'off' or 'dim' state ---
  // A lamp with a loose contact might not always go completely off; it might dim.
  // We introduce a chance for it to either go fully black or to a very dim yellow.
  if (random(0, 100) < 80) { // Approximately 80% chance to go fully off (black)
    M5.Lcd.fillScreen(TFT_BLACK);
  } else { // Approximately 20% chance to go to a very dim, brownish yellow
    // M5.Lcd.color565(R, G, B) takes 8-bit RGB values and converts them to 16-bit 565 format.
    // (60, 60, 0) represents a dark yellow/brown, simulating a very dim or sputtering light.
    M5.Lcd.fillScreen(M5.Lcd.color565(60, 60, 0));
  }

  // Random delay for how long the light stays 'off' or 'dim'.
  // This delay is highly varied to mimic the unpredictable nature of a loose contact.
  // Values: 10ms (almost imperceptible flicker) to 500ms (half-second off period).
  delay(random(10, 500));

  // --- Occasional extended 'off' period ---
  // To further enhance the realism of a loose contact, occasionally the lamp might stay off for longer.
  // There's a small chance (5%) for a longer 'off' duration, simulating a temporary disconnect.
  if (random(0, 100) < 5) {
    M5.Lcd.fillScreen(TFT_BLACK); // Ensure it's black for the extended off period
    delay(random(1000, 3000)); // Stay off for 1 to 3 seconds
  }
}