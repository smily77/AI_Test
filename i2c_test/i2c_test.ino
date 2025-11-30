// Required Libraries:
// 1. Wire Library (Built-in Arduino Library): For I2C communication.
// 2. LovyanGFX Library (https://github.com/lovyan03/LovyanGFX): For TFT display control.
//    Install via Arduino Library Manager: Search for "LovyanGFX".
// 3. CYD_Display_Config.h (User-provided configuration file): This file is crucial.
//    It is expected to define a custom LGFX class (e.g., named 'LGFX') that inherits from
//    lgfx::LGFX_Device and contains the specific configuration (resolution, pins, touch, etc.)
//    for the TFT display connected to the ESP32-S3. Ensure this header file is in your
//    sketch folder or a library path for compilation.

#include <Wire.h>             // Include the I2C communication library
#include <CYD_Display_Config.h> // Include the custom LovyanGFX display configuration file.

// IMPORTANT: The 'lcd' object (instance of the custom LGFX class) is now declared
// directly in the main sketch, as per user requirement. The 'CYD_Display_Config.h'
// file is expected to define the 'LGFX' class itself, which contains all the
// necessary display hardware configurations in its constructor.
LGFX lcd; // Declare the LovyanGFX display object globally.


// I2C Pins (CYD Standard - external I2C)
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif

// Variable to store the I2C address of the found PCF8574 module.
// Initialized to -1, indicating that no PCF8574 has been found yet.
int pcf8574Address = -1;

/**
 * @brief Reads the 8-bit input state from a PCF8574 I2C I/O expander.
 *        The PCF8574 acts as a quasi-bidirectional port, meaning reading from
 *        the device directly provides the current state of its input pins.
 * @param address The 7-bit I2C address of the PCF8574.
 * @return An 8-bit unsigned integer (byte) representing the state of the 8
 *         input pins (P0-P7). Each bit corresponds to an input pin.
 *         Returns 0xFF (all bits high) if the read operation fails (e.g.,
 *         the device does not respond or data is not available).
 */
uint8_t readPCF8574(uint8_t address) {
    Wire.requestFrom(address, (uint8_t)1); // Request 1 byte of data from the PCF8574
    if (Wire.available()) {               // Check if data was received
        return Wire.read();               // Read and return the received byte (input states)
    }
    return 0xFF; // If no data, return 0xFF (all inputs high by default/fail state)
}

/**
 * @brief Arduino setup function.
 *        This function runs once when the ESP32-S3 starts up.
 *        It initializes serial communication, the I2C bus with specified pins,
 *        and the TFT display. It then performs a scan of the I2C bus to
 *        discover connected devices and specifically identifies if a PCF8574
 *        is present, displaying results on both the TFT and Serial Monitor.
 */
void setup() {
    // Initialize Serial communication FIRST for debugging output.
    // A check is added to wait for the Serial Monitor to be connected.
    Serial.begin(115200);
    while (!Serial) {
        // On ESP32, Serial can be ready before the monitor connects. Wait a bit.
        // A small delay or a timeout can be used, but a blocking wait is typical for initial debug.
        delay(10); 
    }
    Serial.println("ESP32-S3 I2C Scanner and PCF8574 Monitor");
    Serial.println("--------------------------------------");

    // Initialize the I2C bus with the specified SDA and SCL pins.
    // This should also happen before display init if the display uses I2C for touch.
    Wire.begin(extSDA, extSCL);
    Serial.printf("I2C initialized on SDA: %d, SCL: %d\n", extSDA, extSCL);

    // Initialize the TFT display using the configuration from CYD_Display_Config.h.
    // The 'lcd' object is now instantiated globally as 'LGFX lcd;'
    Serial.println("Initializing TFT display...");
    if (!lcd.init()) {
        // If lcd.init() fails, it's likely a misconfiguration in CYD_Display_Config.h
        // or hardware issue. The program cannot proceed without a working display.
        Serial.println("ERROR: TFT display initialization failed! Check CYD_Display_Config.h and wiring.");
        Serial.println("Halting program.");
        while (true) { /* Halt */ }
    }
    Serial.println("TFT display initialized successfully.");

    // Set display rotation (adjust as needed: 0=portrait, 1=landscape, 2=inverted portrait, 3=inverted landscape)
    lcd.setRotation(1);
    // Set display brightness (0-255, where 255 is maximum brightness)
    lcd.setBrightness(255);

    // Clear the entire display to black
    lcd.fillScreen(TFT_BLACK);
    // Set text font, size, and color for display output
    lcd.setTextFont(2);        // Use font 2 (often a fixed-width, readable font)
    lcd.setTextSize(1);        // Normal text size
    lcd.setTextColor(TFT_WHITE); // Set text color to white

    // Display an initial message on the TFT screen and Serial Monitor
    lcd.setCursor(0, 0);       // Set cursor to the top-left corner of the display
    lcd.println("Starting I2C Scan...");
    Serial.println("Starting I2C Scan...");

    byte error, address;
    int nDevices = 0; // Counter to keep track of the number of found I2C devices

    // Move the cursor down on the display to make space for the device list
    lcd.setCursor(0, lcd.getCursorY() + 20);
    lcd.println("I2C devices found:");
    Serial.println("I2C devices found:");

    // Loop through all possible 7-bit I2C addresses (from 1 to 126)
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address); // Start a transmission to the current address
        error = Wire.endTransmission();  // End transmission and check for an ACK/NACK response

        if (error == 0) { // If error is 0, an ACK was received, meaning a device is present
            String msg = "  I2C device at 0x";
            if (address < 16) {
                msg += "0"; // Add a leading zero for single-digit hexadecimal addresses
            }
            msg += String(address, HEX); // Append the address in hexadecimal format
            msg += " (Dec: ";
            msg += String(address);      // Append the address in decimal format
            msg += ")";

            lcd.println(msg);    // Display the device's address on the TFT
            Serial.println(msg); // Print the device's address to the Serial Monitor

            // Check if the found device is a potential PCF8574.
            // PCF8574 devices typically use addresses from 0x20 to 0x27.
            if (address >= 0x20 && address <= 0x27) {
                // Attempt to read from the device to confirm its responsiveness.
                // A successful read confirms it's likely a PCF8574 or similar I/O expander.
                Wire.requestFrom(address, (uint8_t)1);
                if (Wire.available()) {
                    Wire.read();             // Read one byte (discarded, just to confirm availability)
                    pcf8574Address = address; // Store the address of the first detected PCF8574
                    lcd.println("    (Possible PCF8574 detected!)");
                    Serial.println("    (Possible PCF8574 detected!)");
                    // If multiple PCF8574s are present, only the first one found will be monitored
                    // by this program. To monitor multiple, this logic would need to be expanded.
                }
            }
            nDevices++; // Increment the counter for found I2C devices
        } else if (error == 4) { // Other error during transmission
            String msg = "  Unknown error at address 0x";
            if (address < 16) {
                msg += "0";
            }
            msg += String(address, HEX);
            lcd.println(msg);
            Serial.println(msg);
        }
    }

    // Display a summary of the I2C scan results
    if (nDevices == 0) {
        lcd.println("No I2C devices found.");
        Serial.println("No I2C devices found.");
    } else {
        lcd.println("Scan complete.");
        Serial.println("Scan complete.");
    }

    // If a PCF8574 was successfully found during the scan, prepare to monitor its inputs.
    if (pcf8574Address != -1) {
        // Move the cursor down and change text color for the PCF8574 status message
        lcd.setCursor(0, lcd.getCursorY() + 20);
        lcd.setTextColor(TFT_GREEN); // Set text color to green for this message
        lcd.printf("Monitoring PCF8574 at 0x%02X:\n", pcf8574Address);
        Serial.printf("Monitoring PCF8574 at 0x%02X:\n", pcf8574Address);
        lcd.setTextColor(TFT_WHITE); // Reset text color back to white
    } else {
        // Inform the user if no PCF8574 was found
        lcd.setCursor(0, lcd.getCursorY() + 20);
        lcd.println("No PCF8574 found, skipping input monitoring.");
        Serial.println("No PCF8574 found, skipping input monitoring.");
    }
}

/**
 * @brief Arduino loop function.
 *        This function runs repeatedly after the setup() function completes.
 *        If a PCF8574 was detected during setup, this loop continuously reads
 *        its 8 input pins and displays their current state (0 or 1 for each pin)
 *        on the TFT screen and prints to the Serial Monitor.
 */
void loop() {
    // Only proceed with PCF8574 monitoring if one was found in setup()
    if (pcf8574Address != -1) {
        // Read the current 8-bit input state from the detected PCF8574
        uint8_t inputState = readPCF8574(pcf8574Address);

        // Define a fixed vertical position on the screen for displaying input states
        // This ensures the input display doesn't scroll with other text and is always visible.
        int displayY = lcd.height() - 50; // Place the display area 50 pixels from the bottom

        // Clear the specific rectangular area on the screen where the input state will be shown.
        // This prevents previous text from ghosting or overlapping with new readings.
        // Clears from (0, displayY) to (width, displayY + 40) with black color.
        lcd.fillRect(0, displayY, lcd.width(), 40, TFT_BLACK);

        // Set the cursor to the defined display position and set text color for input status
        lcd.setCursor(0, displayY);
        lcd.setTextColor(TFT_CYAN); // Set text color to cyan for input status display
        lcd.printf("Inputs (P7-P0): "); // Display a label for the input pins

        // Iterate from P7 down to P0 to display each bit's state (0 or 1)
        for (int i = 7; i >= 0; i--) {
            // Use bitwise operations to extract the state of each individual pin
            lcd.printf("%d ", (inputState >> i) & 0x01); // Print 0 if low, 1 if high
        }
        lcd.println(); // Move to the next line (though it will be cleared in the next loop)

        // Also print the PCF8574 input state to the Serial Monitor in binary format
        char buffer[10]; // 8 bits + space + null terminator
        for (int i = 7; i >= 0; i--) {
            buffer[7-i] = ((inputState >> i) & 0x01) ? '1' : '0';
        }
        buffer[8] = '\0';
        Serial.printf("PCF8574 Inputs: %s\n", buffer);
    }

    // Introduce a small delay to prevent rapid screen updates, which can cause flickering,
    // and to avoid excessively polling the I2C bus.
    delay(200); // Wait for 200 milliseconds
}
