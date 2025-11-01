// Arduino Sketch for printing "Test3" to the serial monitor.
//
// Board: Arduino Uno
//
// This sketch initializes the serial communication and then
// continuously prints the word "Test3" to the serial monitor,
// followed by a newline, with a one-second delay between prints.

void setup() {
  // Initialize serial communication at 9600 bits per second.
  // This rate must match the rate set in the Serial Monitor in the Arduino IDE.
  Serial.begin(9600);
}

void loop() {
  // Print the string "Test3" to the serial monitor.
  // 'println' adds a newline character after the string,
  // moving the cursor to the next line for subsequent prints.
  Serial.println("Test3");

  // Wait for 1000 milliseconds (1 second) before repeating the loop.
  // This prevents flooding the serial monitor with too many messages too quickly,
  // making the output more readable.
  delay(1000);
}