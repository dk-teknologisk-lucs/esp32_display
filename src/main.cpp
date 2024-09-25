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
#define FONT_SIZE 2

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

// Function to draw round buttons
void drawButtons() {
  // Draw button for 30 degrees
  tft.fillCircle(btn20_x, BTN_Y, BTN_RADIUS, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("20deg", btn20_x, BTN_Y - 10, FONT_SIZE);

  // Draw button for 10 degrees
  tft.fillCircle(btn60_x, BTN_Y, BTN_RADIUS, TFT_BLUE);
  tft.drawCentreString("60deg", btn60_x, BTN_Y - 10, FONT_SIZE);

  // Draw button for 45 degrees
  tft.fillCircle(btn120_x, BTN_Y, BTN_RADIUS, TFT_BLUE);
  tft.drawCentreString("120deg", btn120_x, BTN_Y - 10, FONT_SIZE);
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

  // Visualize the predicted angle for springback as a symmetrical blue sector around the vertical axis (upwards)
  int halfPredAngle = angle_pred_springback / 2;
  for (int i = -halfPredAngle; i <= halfPredAngle; i++) {
    float radians = i * DEG_TO_RAD;

    int x = centerX + radius * sin(radians);
    int y = centerY - radius * cos(radians);

    tft.drawLine(centerX, centerY, x, y, TFT_BLUE);
  }

  // Visualize the measured angle as a symmetrical red sector around the vertical axis (upwards)
  int halfMeasuredAngle = angle_measured / 2;
  for (int i = -halfMeasuredAngle; i <= halfMeasuredAngle; i++) {
    float radians = i * DEG_TO_RAD;

    int x = centerX + radius * sin(radians);
    int y = centerY - radius * cos(radians);

    tft.drawLine(centerX, centerY, x, y, TFT_RED);
  }

  // Display the current measured angle at the bottom of the screen
  String angleText = "Measured Angle = " + String(angle_measured) + "°";
  tft.setTextColor(TFT_BLACK);
  tft.drawCentreString(angleText, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, FONT_SIZE);

  // Display the current predicted angle for springback
  String predAngleText = "Predicted Springback Angle = " + String(angle_pred_springback) + "°";
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

// Calculate springback angle based on the measured angle and the material properties
int calculateSpringbackAngle(int measuredAngle) {
  // Springback angle is calculated based on the measured angle and the material properties
  // For simplicity, we will assume that the springback angle is 10% of the measured angle
  return measuredAngle + (180-measuredAngle) * 0.1;
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

  // Clear the screen and draw the buttons
  tft.fillScreen(TFT_WHITE);
  drawButtons();

  // Draw the initial angle visualization
  drawAngleVisualization(angle_measured, angle_pred_springback);
}

void loop() {
  // Check if the touchscreen was touched
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get touchscreen points
    TS_Point p = touchscreen.getPoint();

    // Map touchscreen points to screen coordinates
    int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);

    // Check if a round button was pressed
    int newAngle = checkButtonPress(x, y);
    if (newAngle != -1) {
      angle_measured = newAngle;
      angle_pred_springback = calculateSpringbackAngle(angle_measured);
      drawAngleVisualization(angle_measured, angle_pred_springback);  // Redraw the angle visualization with the new angle
    }

    delay(200);  // Small delay to debounce touchscreen press
  }
}