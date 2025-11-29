#include <Wire.h> // Required for I2C communication with BNO055
#include <Adafruit_Sensor.h> // Dependency for Adafruit BNO055 library
#include <Adafruit_BNO055.h> // Library for BNO055 IMU
// #include <CYD_Display_Config.h> // This header (provided by the user) includes LGFX and configures it for the ESP32-S3 CYD board.
// Make sure this file is accessible in your project or installed with your board's core.
// If not using a specific board definition like CYD_Display_Config, you'd typically include <LGFX.h> and manually configure LGFX.
// For the ESP32-S3 CYD board, CYD_Display_Config.h is usually part of its support package.
#include <CYD_Display_Config.h>

// --- Library Installation Instructions ---
// For Adafruit BNO055 and Adafruit Unified Sensor:
// 1. Open your Arduino IDE.
// 2. Go to Sketch > Include Library > Manage Libraries...
// 3. Search for "Adafruit BNO055" and install it.
// 4. "Adafruit Unified Sensor" is a dependency and should be installed automatically or separately if prompted.
//
// For CYD_Display_Config.h:
// This file is typically provided by the specific ESP32-S3 CYD board support package or examples.
// Ensure your board is correctly set up in the Arduino IDE (e.g., ESP32S3 Dev Module, and select the specific CYD board if available).
// If you encounter an error, you might need to manually ensure this header is in your sketch folder or the libraries directory.

// --- BNO055 Configuration ---
#define BNO055_I2C_ADDRESS    0x29  // I2C address of the BNO055, as specified by the user
#define BNO055_SDA_PIN        1     // SDA pin for I2C communication
#define BNO055_SCL_PIN        2     // SCL pin for I2C communication

// Create BNO055 sensor object
// The last parameter (55) is the sensor ID, can be any unique integer.
Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO055_I2C_ADDRESS);

// Create LGFX display object.
// The CYD_Display_Config.h file takes care of the specific display initialization.
LGFX lcd;

// --- 3D Graphics Structures ---

// Define a simple 3D vector structure for vertices
struct Vector3f {
    float x, y, z;
};

// Define a 4x4 matrix structure for transformations
struct Matrix4f {
    float m[4][4];

    // Constructor to initialize the matrix as an identity matrix
    Matrix4f() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = (i == j) ? 1.0f : 0.0f; // Set diagonal to 1, others to 0
            }
        }
    }
};

// --- Cube Definition ---
// Vertices of a unit cube centered at the origin
// The order defines the corners for drawing, useful for visual debugging.
//    7-----6
//   /|    /|
//  4-----5 |
//  | 3---|-2
//  |/    |/
//  0-----1
Vector3f cubeVertices[] = {
  {-0.5, -0.5, -0.5}, // 0: Bottom-left-back
  { 0.5, -0.5, -0.5}, // 1: Bottom-right-back
  { 0.5,  0.5, -0.5}, // 2: Top-right-back
  {-0.5,  0.5, -0.5}, // 3: Top-left-back

  {-0.5, -0.5,  0.5}, // 4: Bottom-left-front
  { 0.5, -0.5,  0.5}, // 5: Bottom-right-front
  { 0.5,  0.5,  0.5}, // 6: Top-right-front
  {-0.5,  0.5,  0.5}  // 7: Top-left-front
};

// Edges of the cube (pairs of vertex indices)
const int numEdges = 12;
int cubeEdges[numEdges][2] = {
  {0,1}, {1,2}, {2,3}, {3,0}, // Back face edges
  {4,5}, {5,6}, {6,7}, {7,4}, // Front face edges
  {0,4}, {1,5}, {2,6}, {3,7}  // Connecting edges (front to back)
};

// --- 3D Math Functions ---

// Function to perform matrix multiplication: C = A * B
Matrix4f multiply(const Matrix4f& A, const Matrix4f& B) {
    Matrix4f C; // Result matrix, initialized as identity
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            C.m[i][j] = 0.0f; // Initialize element to 0 before summation
            for (int k = 0; k < 4; ++k) {
                C.m[i][j] += A.m[i][k] * B.m[k][j]; // Sum products
            }
        }
    }
    return C;
}

// Function to create a rotation matrix around the X-axis
Matrix4f rotateX(float angle_rad) {
    Matrix4f M;
    float c = cos(angle_rad);
    float s = sin(angle_rad);
    M.m[1][1] = c;   M.m[1][2] = -s;
    M.m[2][1] = s;   M.m[2][2] = c;
    return M;
}

// Function to create a rotation matrix around the Y-axis
Matrix4f rotateY(float angle_rad) {
    Matrix4f M;
    float c = cos(angle_rad);
    float s = sin(angle_rad);
    M.m[0][0] = c;   M.m[0][2] = s;
    M.m[2][0] = -s;  M.m[2][2] = c;
    return M;
}

// Function to create a rotation matrix around the Z-axis
Matrix4f rotateZ(float angle_rad) {
    Matrix4f M;
    float c = cos(angle_rad);
    float s = sin(angle_rad);
    M.m[0][0] = c;   M.m[0][1] = -s;
    M.m[1][0] = s;   M.m[1][1] = c;
    return M;
}

// Function to create a translation matrix
Matrix4f translate(float x, float y, float z) {
    Matrix4f M;
    M.m[0][3] = x;
    M.m[1][3] = y;
    M.m[2][3] = z;
    return M;
}

// Function to create a perspective projection matrix
// fov_rad: Field of view angle in radians
// aspect: Aspect ratio of the viewport (width / height)
// near_plane: Distance to the near clipping plane
// far_plane: Distance to the far clipping plane
Matrix4f perspective(float fov_rad, float aspect, float near_plane, float far_plane) {
    Matrix4f M; // Initialize as identity
    float tanHalfFov = tan(fov_rad / 2.0f);

    M.m[0][0] = 1.0f / (aspect * tanHalfFov);
    M.m[1][1] = 1.0f / tanHalfFov;
    M.m[2][2] = -(far_plane + near_plane) / (far_plane - near_plane);
    M.m[2][3] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
    M.m[3][2] = -1.0f;
    M.m[3][3] = 0.0f; // Clear this element which is 1 in identity for correct perspective transform
    return M;
}

// Function to transform a 3D vector by a 4x4 matrix
// Performs matrix-vector multiplication, assuming vector's W-component is 1.0.
// Then performs perspective division (dividing x,y,z by W)
Vector3f transform(const Matrix4f& M, const Vector3f& V) {
    Vector3f result;
    // Calculate W component for perspective division
    float w = M.m[3][0] * V.x + M.m[3][1] * V.y + M.m[3][2] * V.z + M.m[3][3];

    // Perform the matrix-vector multiplication
    result.x = M.m[0][0] * V.x + M.m[0][1] * V.y + M.m[0][2] * V.z + M.m[0][3];
    result.y = M.m[1][0] * V.x + M.m[1][1] * V.y + M.m[1][2] * V.z + M.m[1][3];
    result.z = M.m[2][0] * V.x + M.m[2][1] * V.y + M.m[2][2] * V.z + M.m[2][3];

    // Perspective division
    if (w != 0.0f) { // Avoid division by zero
        result.x /= w;
        result.y /= w;
        result.z /= w; // Z component is for depth, ranges from -1 to 1 after projection
    } else {
        // Handle cases where w is zero (e.g., points at infinity or behind camera)
        // For simplicity, we might just leave them unscaled or clamp.
        // A proper 3D renderer would use clipping here.
        result.x = result.y = result.z = 0.0f; // Or some indicator of invalid point
    }
    return result;
}


void setup() {
    // Initialize Serial communication for debugging messages
    Serial.begin(115200);
    while (!Serial); // Wait for Serial to be ready
    Serial.println("ESP32-S3 3D Cube Demo with BNO055");

    // Initialize the LGFX display
    lcd.init();
    lcd.setRotation(1); // Set display to landscape mode (adjust as needed for your specific CYD board orientation)
    lcd.setFont(&fonts::FreeSansBold12pt7b); // Set a font for text output
    lcd.setTextDatum(MC_DATUM); // Set text datum to middle-center for easy positioning
    lcd.fillScreen(TFT_BLACK); // Clear the display with black

    // Configure and start I2C communication for the BNO055
    Wire.setPins(BNO055_SDA_PIN, BNO055_SCL_PIN);
    Wire.begin();

    // Initialize the BNO055 sensor
    if (!bno.begin()) {
        Serial.println("Ooops, no BNO055 detected ... Check your wiring, I2C address (0x29) or pins!");
        lcd.setTextColor(TFT_RED);
        lcd.setCursor(lcd.width() / 2, lcd.height() / 2);
        lcd.print("BNO055 Error!");
        while (1); // Halt program execution if BNO055 is not found
    }
    bno.setExtCrystalUse(true); // Use external crystal for better timing and accuracy

    Serial.println("BNO055 initialized successfully.");
    delay(100); // Small delay to allow BNO055 to stabilize
}


void loop() {
    // Clear the entire screen at the start of each frame
    lcd.fillScreen(TFT_BLACK);

    // --- Read BNO055 Orientation Data ---
    sensors_event_t event; // Create an event object to store sensor data
    bno.getEvent(&event); // Read the latest sensor data

    // Adafruit BNO055 reports Euler angles (heading, roll, pitch) in degrees:
    // event.orientation.x = Heading (Yaw)
    // event.orientation.y = Roll
    // event.orientation.z = Pitch
    // The user notes "Z-Achse zeigt nach unten", meaning the BNO055's Z-axis is inverted
    // relative to a standard mounting. This might cause pitch/roll to be inverted
    // or swapped depending on the BNO's internal coordinate system and Adafruit library's interpretation.
    // We will use standard YXZ Euler rotation (Yaw around Y, Pitch around X, Roll around Z) for the camera.
    // The view matrix is the inverse of the camera's world transformation.
    // So we apply negative angles in reverse order of rotation (Z-X-Y).
    
    // Convert BNO055 angles from degrees to radians
    float yaw_bno_rad   = radians(event.orientation.x); // Heading (rotation around Y-axis for camera)
    float roll_bno_rad  = radians(event.orientation.y); // Roll (rotation around X-axis for camera)
    float pitch_bno_rad = radians(event.orientation.z); // Pitch (rotation around Z-axis for camera)

    // --- IMPORTANT: Adjusting for "Z-Achse zeigt nach unten" and desired camera behavior ---
    // If rotating the board right/left (Yaw) works correctly, then yaw_bno_rad is likely okay.
    // If tilting the board forward (positive pitch) makes the cube move *down* instead of *up* (i.e., camera looks *up*),
    // then you might need to negate `pitch_bno_rad`. E.g., `pitch_bno_rad = -pitch_bno_rad;`
    // If rolling the board left (positive roll) makes the cube rotate clockwise instead of counter-clockwise,
    // you might need to negate `roll_bno_rad`. E.g., `roll_bno_rad = -roll_bno_rad;`
    // The current setup assumes standard IMU Euler angle output, which might require sign inversion based on physical mounting.
    
    // --- Construct the Camera View Matrix ---
    // The view matrix transforms world coordinates into the camera's coordinate system.
    // It's the inverse of the camera's transformation in the world.
    // Assuming the camera's rotation is YXZ (Yaw around Y, Pitch around X, Roll around Z)
    // The inverse transformation applies rotations in reverse order with negated angles (Z-X-Y).
    Matrix4f viewMatrix = rotateZ(-roll_bno_rad);  // Apply inverse roll around camera's Z-axis
    viewMatrix = multiply(viewMatrix, rotateX(-pitch_bno_rad)); // Apply inverse pitch around camera's X-axis
    viewMatrix = multiply(viewMatrix, rotateY(-yaw_bno_rad));   // Apply inverse yaw around camera's Y-axis

    // --- Define Projection Matrix ---
    float fov_angle_deg = 60.0f; // Field of view in degrees (common for perspective)
    float fov_rad = radians(fov_angle_deg); // Convert to radians
    float aspect_ratio = (float)lcd.width() / (float)lcd.height(); // Aspect ratio of the display
    float near_clip_plane = 0.1f; // Objects closer than this are clipped
    float far_clip_plane = 100.0f; // Objects farther than this are clipped
    Matrix4f projMatrix = perspective(fov_rad, aspect_ratio, near_clip_plane, far_clip_plane);

    // --- Define Model Matrix for the Cube ---
    // The cube is stationary in world space. We translate it a fixed distance in front of the camera.
    // If the camera is at origin and looking along +Z, the cube should be at (+Z_distance).
    Matrix4f modelMatrix = translate(0.0f, 0.0f, 3.0f); // Cube is 3 units in front of the camera

    // --- Combine all transformation matrices ---
    // Order: Projection * View * Model (transforms from Object space -> World space -> Camera space -> Clip space)
    Matrix4f mvpMatrix = multiply(projMatrix, viewMatrix);
    mvpMatrix = multiply(mvpMatrix, modelMatrix);

    // --- Project Cube Vertices to Screen Coordinates ---
    Vector3f projectedVertices[8]; // Array to store 2D screen coordinates of each vertex
    for (int i = 0; i < 8; ++i) {
        // Transform each 3D vertex using the combined MVP matrix
        projectedVertices[i] = transform(mvpMatrix, cubeVertices[i]);

        // After transformation and perspective division, x and y are in Normalized Device Coordinates (NDC)
        // ranging from -1.0 to 1.0. Map these to screen pixel coordinates.
        // LGFX (0,0) is top-left, X increases to the right, Y increases downwards.
        // NDC Y (-1 to 1) needs to be inverted for screen Y (0 to height).
        projectedVertices[i].x = (projectedVertices[i].x + 1.0f) * lcd.width() / 2.0f;
        projectedVertices[i].y = (1.0f - projectedVertices[i].y) * lcd.height() / 2.0f;
    }

    // --- Draw the Cube Edges ---
    lcd.drawRect(0, 0, lcd.width(), lcd.height(), TFT_BLUE); // Draw a blue frame around the display
    for (int i = 0; i < numEdges; ++i) {
        int v0_idx = cubeEdges[i][0]; // Index of the first vertex of the edge
        int v1_idx = cubeEdges[i][1]; // Index of the second vertex of the edge

        // Basic clipping: Only draw edges if both vertices are roughly within the viewing frustum (z between -1 and 1).
        // This avoids drawing lines that span across the camera's Z=0 plane (behind the camera).
        if (projectedVertices[v0_idx].z < 1.0f && projectedVertices[v1_idx].z < 1.0f &&
            projectedVertices[v0_idx].z > -1.0f && projectedVertices[v1_idx].z > -1.0f) {
            lcd.drawLine(
                (int)projectedVertices[v0_idx].x, (int)projectedVertices[v0_idx].y,
                (int)projectedVertices[v1_idx].x, (int)projectedVertices[v1_idx].y,
                TFT_WHITE // Draw lines in white
            );
        }
    }

    // --- Display Debug Information (Euler Angles) ---
    // Set text datum to top-left for easy debug text alignment
    lcd.setTextDatum(TL_DATUM);
    lcd.setTextColor(TFT_CYAN); // Set text color to cyan
    lcd.setCursor(5, 5); // Position for Yaw
    lcd.printf("Yaw: %.1f", event.orientation.x);
    lcd.setCursor(5, 20); // Position for Roll
    lcd.printf("Roll: %.1f", event.orientation.y);
    lcd.setCursor(5, 35); // Position for Pitch
    lcd.printf("Pitch: %.1f", event.orientation.z);

    // Small delay to prevent the ESP32's watchdog timer from resetting the board
    // and to limit the drawing frame rate.
    delay(10);
}