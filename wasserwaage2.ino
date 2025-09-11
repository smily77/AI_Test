#include <M5StickC.h>

// --- Configuration Constants ---
const int DOT_RADIUS = 3;             // Radius of the dot in pixels
const int DOT_COLOR = WHITE;          // Color of the dot (e.g., WHITE, BLACK, RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA)
const int BACKGROUND_COLOR = BLACK;   // Background color of the display area
const int TOP_TEXT_HEIGHT = 40;       // Height in pixels reserved for text at the top of the screen

// Range for accelerometer values in 'g' units.
// These values define the sensitivity and the mapping boundaries.
// Using a slightly wider range than -1.0 to 1.0 (e.g., -1.2 to 1.2)
// can help prevent the dot from sticking at the edges due to minor overshoots or noise.
const float ACCEL_RANGE_MIN = -1.2;
const float ACCEL_RANGE_MAX = 1.2;

// --- Global Variables ---
// Variables to store accelerometer data (in 'g' units)
float accX = 0;
float accY = 0;
float accZ = 0;

// --- Helper Function ---
/**
 * @brief Maps a floating-point value from one range to another.
 *        The input value is constrained to the input range before mapping.
 * @param x The input value to map.
 * @param in_min The lower bound of the input range.
 * @param in_max The upper bound of the input range.
 * @param out_min The lower bound of the output range.
 * @param out_max The upper bound of the output range.
 * @return The mapped value, constrained to the output range.
 */
float map_float(float x, float in_min, float in_max, float out_min, float out_max) {
    // Constrain the input value to prevent mapping outside the intended range
    if (x < in_min) x = in_min;
    if (x > in_max) x = in_max;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// --- Arduino Setup Function ---
void setup() {
    // Initialize M5StickC. This function handles the setup of the display, IMU,
    // power management, and other internal components.
    M5.begin();

    // Set screen rotation. M5StickC's default resolution is 80x160.
    // Rotation 0: Portrait mode (80x160, USB port downwards)
    // The screen dimensions will be M5.Lcd.width()=80, M5.Lcd.height()=160.
    M5.Lcd.setRotation(0);

    // Clear the entire screen and set the background color
    M5.Lcd.fillScreen(BACKGROUND_COLOR);

    // Set text properties for initial display
    M5.Lcd.setTextFont(2); // Font size 2 (smaller than default)
    M5.Lcd.setTextColor(WHITE); // White text color

    // Display initial instructions or title
    M5.Lcd.setCursor(5, 5); // Position cursor at (x=5, y=5)
    M5.Lcd.print("IMU Dot Tracker");
    M5.Lcd.setCursor(5, 25); // Position cursor below the title
    M5.Lcd.print("Tilt M5StickC");

    // IMPORTANT: Uncomment the line below to enable serial output for debugging accelerometer values.
    // This is crucial to diagnose if the IMU is providing data.
    Serial.begin(115200);
}

// --- Arduino Loop Function ---
void loop() {
    // Update M5Stack services like IMU data, button states, etc.
    M5.update();

    // Read accelerometer data from the built-in IMU (MPU6886) in 'g' units.
    // When the M5StickC is held with the screen facing you and the USB port down:
    // - X-axis points to the right. (Rotation around this axis is Roll, which affects horizontal movement)
    // - Y-axis points out of the screen, towards you. (When upright, gravity acts predominantly along this axis in the negative direction)
    // - Z-axis points upwards, towards the top edge. (Rotation around this axis is Pitch, which affects vertical movement)
    M5.IMU.getAccelData(&accX, &accY, &accZ);

    // --- Debugging: Check raw sensor data and mapped coordinates via Serial Monitor ---
    // If the dot is not moving, check if these values are changing as you tilt the M5StickC.
    // If they remain static (e.g., 0.00), the IMU might not be correctly initialized or working.
    Serial.printf("accX: %.2f, accY: %.2f, accZ: %.2f | ", accX, accY, accZ);

    // --- Map Accelerometer Data to Screen Coordinates ---
    // The screen has a width of M5.Lcd.width() (80 pixels) and height of M5.Lcd.height() (160 pixels).
    // We reserve the top TOP_TEXT_HEIGHT pixels for text, so the dot drawing area starts at y=TOP_TEXT_HEIGHT.
    // The effective drawing area for the dot is M5.Lcd.width() x (M5.Lcd.height() - TOP_TEXT_HEIGHT).

    // Horizontal position (dotX) is controlled by `accX` (roll motion):
    // - If `accX` is positive (tilted left, left edge down), `dotX` should be towards the left (small X).
    // - If `accX` is negative (tilted right, right edge down), `dotX` should be towards the right (large X).
    // - If `accX` is 0 (flat), `dotX` should be at the horizontal center.
    // The output range is reversed (width-1 to 0) to match the desired dot movement with accX polarity.
    int dotX = (int)map_float(accX, ACCEL_RANGE_MIN, ACCEL_RANGE_MAX, M5.Lcd.width() - 1, 0);

    // Vertical position (dotY) is controlled by `accZ` (pitch motion in this orientation):
    // - When held upright (USB down, screen facing you), `accZ` is near 0.
    // - If `accZ` is negative (tilted forward / screen-top down), `dotY` should be towards the top of the drawing area (small Y).
    // - If `accZ` is positive (tilted backward / screen-bottom down), `dotY` should be towards the bottom of the drawing area (large Y).
    // The output range is 0 to (height-text_height)-1 to match the desired dot movement with accZ polarity.
    int dotY_mapped_to_area = (int)map_float(accZ, ACCEL_RANGE_MIN, ACCEL_RANGE_MAX, 0, (M5.Lcd.height() - TOP_TEXT_HEIGHT) - 1);

    // --- Debugging: Print mapped coordinates ---
    Serial.printf("Mapped dotX: %d, Mapped dotY_area: %d (Total Y: %d)\n", dotX, dotY_mapped_to_area, dotY_mapped_to_area + TOP_TEXT_HEIGHT);

    // --- Drawing ---
    // Clear the previous dot from the drawing area.
    // This redraws a rectangle from the starting Y of the drawing area to the bottom of the screen.
    M5.Lcd.fillRect(0, TOP_TEXT_HEIGHT, M5.Lcd.width(), M5.Lcd.height() - TOP_TEXT_HEIGHT, BACKGROUND_COLOR);

    // Draw the dot at the new calculated position.
    // We add TOP_TEXT_HEIGHT to dotY_mapped_to_area to place it correctly within the screen coordinates,
    // below the reserved text area.
    M5.Lcd.fillCircle(dotX, dotY_mapped_to_area + TOP_TEXT_HEIGHT, DOT_RADIUS, DOT_COLOR);

    // Introduce a small delay to avoid excessive updates and make the movement smooth.
    delay(20);
}