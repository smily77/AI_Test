// Arduino C++ code for "Blink with LED" functionality
// Board: Arduino Uno

// This code blinks the built-in LED on the Arduino Uno board.
// The built-in LED is typically connected to digital pin 13 on most Arduino boards,
// and it's conveniently accessible via the `LED_BUILTIN` constant.

// Required Libraries:
// No external libraries are needed for this basic functionality.
// All necessary functions are part of the standard Arduino core.

// --- Pin Definitions ---
// Define the pin connected to the LED.
// LED_BUILTIN is a convenient constant that refers to the on-board LED.
// On Arduino Uno, this is digital pin 13.
const int ledPin = 13; // The pin connected to the built-in LED

// --- Setup Function ---
// The setup() function runs once when the sketch starts or after a reset.
void setup() {
  // Initialize the digital pin `ledPin` as an output.
  // This prepares the pin to send voltage (HIGH) or stop sending voltage (LOW).
  pinMode(ledPin, OUTPUT);
}

// --- Loop Function ---
// The loop() function runs repeatedly forever after setup() completes.
void loop() {
  // Turn the LED on (HIGH is the voltage level).
  // This sends 5V to the `ledPin`, lighting up the LED.
  digitalWrite(ledPin, HIGH);
  
  // Wait for a second (1000 milliseconds).
  // This pause keeps the LED on for the specified duration.
  delay(1000); 
  
  // Turn the LED off (LOW is the voltage level).
  // This sends 0V to the `ledPin`, turning off the LED.
  digitalWrite(ledPin, LOW);
  
  // Wait for another second.
  // This pause keeps the LED off for the specified duration, completing one blink cycle.
  delay(1000); 
}