#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 1

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

int angle_measured = 0;  // Default measured angle
int angle_pred_springback = 0;  // Default predicted springback angle

// Button properties (round buttons at the top)
#define BTN_RADIUS 30
#define BTN_SPACING 40
int btn20_x = 60;  // X positions for the three buttons
int btn60_x = btn20_x + BTN_SPACING + BTN_RADIUS * 2;
int btn120_x = btn60_x + BTN_SPACING + BTN_RADIUS * 2;
#define BTN_Y 50  // Y position for all buttons
// Home button properties (square button at the upper left)
#define HOME_BTN_SIZE 30
#define HOME_BTN_X 0
#define HOME_BTN_Y 0

// Start button properties
#define START_BTN_RADIUS 50
#define START_BTN_Y 100
#define START_BTN_X (SCREEN_WIDTH / 2)

// Define screen states
enum ScreenState {
  HOME,
  ANGLE
};

ScreenState currentScreen = HOME;  // Track the current screen state

void drawCloseButton() {
    // Draw home button
    tft.fillRect(HOME_BTN_X, HOME_BTN_Y, HOME_BTN_SIZE, HOME_BTN_SIZE, TFT_RED);
    
    // Draw cross inside home button (rotated 45 degrees)
    int centerX = HOME_BTN_X + HOME_BTN_SIZE / 2;
    int centerY = HOME_BTN_Y + HOME_BTN_SIZE / 2;
    int lineLength = HOME_BTN_SIZE * 0.6; // Length of the cross lines
    int lineThickness = 4; // Thickness of the lines

    // Draw diagonal line from top-left to bottom-right
    tft.fillRect(centerX - lineThickness / 2, centerY - lineLength / 2, lineThickness, lineLength, TFT_WHITE);
    tft.fillRect(centerX - lineLength / 2, centerY - lineThickness / 2, lineLength, lineThickness, TFT_WHITE);
}

// Function to draw round buttons
void drawButtons() {
    // Draw button for 20 degrees
    tft.fillCircle(btn20_x, BTN_Y, BTN_RADIUS, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("20deg", btn20_x, BTN_Y - 10, FONT_SIZE);

    // Draw button for 60 degrees
    tft.fillCircle(btn60_x, BTN_Y, BTN_RADIUS, TFT_BLUE);
    tft.drawCentreString("60deg", btn60_x, BTN_Y - 10, FONT_SIZE);

    // Draw button for 120 degrees
    tft.fillCircle(btn120_x, BTN_Y, BTN_RADIUS, TFT_BLUE);
    tft.drawCentreString("120deg", btn120_x, BTN_Y - 10, FONT_SIZE);

    // Draw home button
    drawCloseButton();
}

// Function to draw the home screen
void drawHomeScreen() {
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(FONT_SIZE);
  tft.drawCentreString("Home Screen", SCREEN_WIDTH / 2, 30, FONT_SIZE);

  // Draw Start button
  tft.fillCircle(START_BTN_X, START_BTN_Y, START_BTN_RADIUS, TFT_GREEN);
  tft.drawCentreString("Start Angle", START_BTN_X, START_BTN_Y - 10, FONT_SIZE);
}

// Function to draw filled arcs (part-circles)
void drawArc(int centerX, int centerY, int radius, int angle, uint16_t color) {
    int startAngle = -angle / 2;
    int endAngle = angle / 2;

    // Draw the arc as filled triangles
    for (int i = startAngle; i < endAngle; i++) {
        float radians1 = i * DEG_TO_RAD;
        float radians2 = (i + 1) * DEG_TO_RAD;  // Next angle for the triangle

        int x1 = centerX + radius * sin(radians1);
        int y1 = centerY - radius * cos(radians1);
        int x2 = centerX + radius * sin(radians2);
        int y2 = centerY - radius * cos(radians2);

        // Draw filled triangle from center to arc edges
        tft.fillTriangle(centerX, centerY, x1, y1, x2, y2, color);
    }
}

// Function to visualize the angles with everything outside them in green
void drawAngleVisualization(int angle_measured, int angle_pred_springback) {
    // Clear the screen, but leave the buttons intact
    tft.fillScreen(TFT_WHITE);
    drawButtons();


    // Set the center at the bottom middle of the screen
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT;

    int radius = 150;  // Radius for the angle visualization

    // Draw the entire region in green (outside the angles)
    tft.fillCircle(centerX, centerY, radius, TFT_GREEN);

    // Visualize the predicted angle for springback as a filled arc in blue
    drawArc(centerX, centerY, radius, angle_pred_springback, TFT_BLUE);

    // Visualize the measured angle as a filled arc in red
    drawArc(centerX, centerY, radius, angle_measured, TFT_RED);

    // Display the current measured angle at the bottom of the screen
    String angleText = "Measured Angle = " + String(angle_measured) + "deg";
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString(angleText, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, FONT_SIZE);

    // Display the current predicted angle for springback
    String predAngleText = "Predicted Springback Angle = " + String(angle_pred_springback) + "deg";
    tft.drawCentreString(predAngleText, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, FONT_SIZE);
}

// Check if a round button was pressed and return the corresponding angle
int checkButtonPress(int touchX, int touchY) {
    // Check if the press is within any button's circle
    if (sqrt(sq(touchX - btn20_x) + sq(touchY - BTN_Y)) <= BTN_RADIUS) {
        return 20;
    } else if (sqrt(sq(touchX - btn60_x) + sq(touchY - BTN_Y)) <= BTN_RADIUS) {
        return 60;
    } else if (sqrt(sq(touchX - btn120_x) + sq(touchY - BTN_Y)) <= BTN_RADIUS) {
        return 120;
    }
    return -1;  // No button pressed
}

// Check if the start button was pressed
bool checkStartButtonPress(int touchX, int touchY) {
  return sqrt(sq(touchX - START_BTN_X) + sq(touchY - START_BTN_Y)) <= START_BTN_RADIUS;
}

// Check if the home button was pressed
bool checkHomeButtonPress(int touchX, int touchY) {
  return sqrt(sq(touchX - HOME_BTN_X) + sq(touchY - HOME_BTN_Y)) <= HOME_BTN_SIZE;
}

// Calculate springback angle based on the measured angle and the material properties
int calculateSpringbackAngle(int measuredAngle) {
    // Springback angle is calculated based on the measured angle and the material properties
    // For simplicity, we will assume that the springback angle is 10% of the measured angle
    return measuredAngle + (180 - measuredAngle) * 0.1;
}

void setup() {
  Serial.begin(115200);

  // Initialize the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);  // Set touchscreen rotation to landscape mode

  // Initialize the TFT display
  tft.init();
  tft.setRotation(1);  // Set TFT rotation to landscape mode

  // Draw the home screen
  drawHomeScreen();
}

void loop() {
  // Check if the touchscreen was touched
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get touchscreen points
    TS_Point p = touchscreen.getPoint();

    // Map touchscreen points to screen coordinates
    int touchX = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int touchY = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);

    // Handle screen navigation based on the current screen state
    switch (currentScreen) {
      case HOME:
        if (checkStartButtonPress(touchX, touchY)) {
          currentScreen = ANGLE;  // Switch to angle screen
          drawAngleVisualization(angle_measured, angle_pred_springback);
        }
        break;
      case ANGLE:
        // Check if a round button was pressed
        int newAngle = checkButtonPress(touchX, touchY);
        if (newAngle != -1) {
          angle_measured = newAngle;
          angle_pred_springback = calculateSpringbackAngle(angle_measured);
          drawAngleVisualization(angle_measured, angle_pred_springback);
        }

        // Check if Back button is pressed
        if (checkHomeButtonPress(touchX, touchY)) {
          currentScreen = HOME;  // Switch back to home screen
          drawHomeScreen();
        }
        break;
    }

    delay(200);  // Small delay to debounce touchscreen press
  }
}