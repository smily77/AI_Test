// Arduino C++ code to blink "SOS" Morse code using the built-in LED on an Arduino Uno.

// --- Pin Configuration ---
// Define the LED pin. Pin 13 is the built-in LED on most Arduino boards,
// including the Arduino Uno. It's connected to a resistor and ready to use.
#define LED_PIN 13

// --- Morse Code Timing Definitions ---
// These values define the speed and rhythm of the SOS signal.
// Adjust UNIT_TIME to make the SOS faster or slower.
// All other timing values are derived from UNIT_TIME according to Morse code standards.
const int UNIT_TIME = 100; // Milliseconds for a single 'dot' (dit) duration.

// Dot duration: 1 unit of time (e.g., 100ms)
const int DOT_DURATION = UNIT_TIME;
// Dash duration: 3 units of time (e.g., 300ms)
const int DASH_DURATION = 3 * UNIT_TIME;

// Pause between elements (dots or dashes within a single letter): 1 unit of time (e.g., 100ms)
const int PAUSE_BETWEEN_ELEMENTS = UNIT_TIME;
// Pause between letters: 3 units of time (e.g., 300ms)
const int PAUSE_BETWEEN_LETTERS = 3 * UNIT_TIME;
// Pause between words (or in this case, between repetitions of the entire "SOS" sequence): 7 units of time (e.g., 700ms)
const int PAUSE_BETWEEN_WORDS = 7 * UNIT_TIME;

// --- Helper Functions for Morse Code Elements ---

/**
 * @brief Blinks the LED for a short duration, representing a 'dot'.
 */
void blinkDot() {
  digitalWrite(LED_PIN, HIGH); // Turn the LED on
  delay(DOT_DURATION);         // Keep it on for the dot duration
  digitalWrite(LED_PIN, LOW);  // Turn the LED off
}

/**
 * @brief Blinks the LED for a longer duration, representing a 'dash'.
 */
void blinkDash() {
  digitalWrite(LED_PIN, HIGH); // Turn the LED on
  delay(DASH_DURATION);        // Keep it on for the dash duration
  digitalWrite(LED_PIN, LOW);  // Turn the LED off
}

/**
 * @brief Introduces a short pause between individual dots/dashes within a letter.
 */
void pauseBetweenElements() {
  delay(PAUSE_BETWEEN_ELEMENTS);
}

/**
 * @brief Introduces a medium pause between different letters in a word.
 */
void pauseBetweenLetters() {
  delay(PAUSE_BETWEEN_LETTERS);
}

/**
 * @brief Introduces a long pause, typically between words or full messages.
 */
void pauseBetweenWords() {
  delay(PAUSE_BETWEEN_WORDS);
}

// --- Specific Morse Code Letter Functions ---

/**
 * @brief Signals the Morse code for the letter 'S' (dot-dot-dot).
 */
void signalS() {
  blinkDot();
  pauseBetweenElements();
  blinkDot();
  pauseBetweenElements();
  blinkDot();
}

/**
 * @brief Signals the Morse code for the letter 'O' (dash-dash-dash).
 */
void signalO() {
  blinkDash();
  pauseBetweenElements();
  blinkDash();
  pauseBetweenElements();
  blinkDash();
}

// --- Standard Arduino Setup Function ---
// This function runs once when the Arduino starts or is reset.
void setup() {
  // Initialize the LED_PIN as an OUTPUT.
  // This tells the Arduino that we will be controlling the voltage on this pin.
  pinMode(LED_PIN, OUTPUT);
}

// --- Standard Arduino Loop Function ---
// This function runs repeatedly after setup() completes.
void loop() {
  // Signal the first 'S' of "SOS"
  signalS();
  pauseBetweenLetters(); // Pause between 'S' and 'O'

  // Signal the 'O' of "SOS"
  signalO();
  pauseBetweenLetters(); // Pause between 'O' and the second 'S'

  // Signal the second 'S' of "SOS"
  signalS();

  // After signaling "SOS", wait for a longer period before repeating the sequence.
  // This represents the space between repetitions of the "SOS" message.
  pauseBetweenWords();
}