// This code is designed for the ESP32-S3 board.
// It initializes two serial interfaces:
// 1. Serial (UART0): Used for communication with the Serial Monitor via USB-CDC.
// 2. Serial1 (UART1): Configured to receive data on GPIO8 at 9600 baud.
//
// The program will read any incoming data on Serial1 (GPIO8) and immediately
// print it to the Serial Monitor (Serial).
//
// No external libraries are required for this functionality as HardwareSerial
// is built into the ESP32 core.

// Define the GPIO pin used for receiving serial data.
// For ESP32-S3, GPIO8 is used as the RX pin for the secondary serial interface.
const int RX_PIN_SERIAL1 = 8;

void setup() {
  // Initialize the primary Serial port (UART0) for communication with the Serial Monitor.
  // This is typically done over the USB-CDC interface on ESP32-S3.
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
    // Wait for the Serial port to connect, especially useful for boards with native USB.
    // Timeout added to prevent indefinite blocking if no Serial Monitor is open.
  }
  Serial.println("ESP32-S3 Serial Passthrough - Pin 8 RX");
  Serial.println("------------------------------------");
  Serial.print("Listening for serial data on GPIO");
  Serial.print(RX_PIN_SERIAL1);
  Serial.println(" at 9600 baud...");

  // Initialize the secondary Serial port (Serial1, which uses UART1 on ESP32).
  // We configure it to receive data on RX_PIN_SERIAL1 (GPIO8) at 9600 baud.
  // The -1 for the TX pin indicates that we are not using a TX pin for Serial1
  // for this specific application (only receiving).
  Serial1.begin(9600, SERIAL_8N1, RX_PIN_SERIAL1, -1);
}

void loop() {
  // Check if there is any data available to read from Serial1 (GPIO8).
  if (Serial1.available()) {
    // Read a byte from Serial1.
    char receivedChar = Serial1.read();

    // Print the received byte to the primary Serial port (Serial Monitor).
    Serial.print(receivedChar);
  }
}