// This sketch blinks the built-in LED on an Arduino Uno.
// The built-in LED is connected to digital pin 13 on most Arduino boards,
// and can be referenced using the LED_BUILTIN macro.

// No external libraries are required for this basic functionality.

// --- Pin Definitions ---
// LED_BUILTIN: The pin number of the built-in LED.
//              On Arduino Uno, this is digital pin 13.

// --- Global Variables ---
const int BLINK_DELAY_MS = 1000; // Delay in milliseconds for how long the LED is on/off

void setup() {
  // Initialize the digital pin LED_BUILTIN as an output.
  // This tells the Arduino that we will be sending signals (HIGH/LOW) to this pin.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Turn the LED on (HIGH is the voltage level).
  // Sending HIGH to the pin supplies voltage, turning the LED on.
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Wait for a second (1000 milliseconds).
  // This pauses the program execution for the specified duration.
  delay(BLINK_DELAY_MS);
  
  // Turn the LED off by making the voltage LOW.
  // Sending LOW to the pin removes voltage, turning the LED off.
  digitalWrite(LED_BUILTIN, LOW);
  
  // Wait for a second.
  // This ensures the LED stays off for the same duration it was on.
  delay(BLINK_DELAY_MS);
}