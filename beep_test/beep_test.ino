// ESP32-S3 Piezo Speaker Test

// This code demonstrates how to make a piezo speaker beep on an ESP32-S3 board.
// The piezo speaker is connected to GPIO Pin 21.

// No external libraries are required for basic tone generation on ESP32 boards,
// as the 'tone()' function is built into the ESP32 Arduino core.

// Define the pin where the piezo speaker is connected.
const int PIEZO_PIN = 21;

void setup() {
  // Initialize the serial communication for debugging purposes (optional).
  // Serial.begin(115200);
  // Serial.println("ESP32-S3 Piezo Beep Test Started");

  // On ESP32, the 'tone()' function automatically configures the pin as an output
  // using its internal LEDC peripheral, so pinMode() is not strictly necessary
  // for this specific application. However, if you were controlling the pin
  // manually with digitalWrite, you would use:
  // pinMode(PIEZO_PIN, OUTPUT);
}

void loop() {
  // Generate a tone on the piezo speaker.
  // tone(pin, frequency) starts an continuous tone.
  // For ESP32, frequencies typically range from 20 Hz to 20 kHz.
  // We'll use 1000 Hz for a clear beep.
  tone(PIEZO_PIN, 1000);

  // Keep the tone playing for 100 milliseconds.
  delay(1000);

  // Stop the tone.
  noTone(PIEZO_PIN);

  // Pause for 500 milliseconds before the next beep to create a rhythm.
  delay(500);

  // This loop will continuously make the piezo speaker beep with a short pause in between.
}