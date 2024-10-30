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

// Define screen states
enum ScreenState {
  HOME,
  ANGLE,
  MOTOR
};
ScreenState currentScreen = HOME;  // Track the current screen state

enum RenderMode {
  FAST,
  ACCURATE
};
RenderMode currentRenderMode = FAST;

enum MeasureState {
  MEASURING,
  IDLE
};
MeasureState currentMeasureState = IDLE;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 1

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

int angleMeasured = 180;  // Default measured angle
int angle_pred_springback = 0;  // Default predicted springback angle

// Button properties (round buttons at the top)
#define BTN_RADIUS 25
#define BTN_SPACING 25
int btnmeasure_x = 40;  // X position for the reset button
int btn20_x = btnmeasure_x + BTN_RADIUS * 2 + BTN_SPACING;
int btn60_x = btn20_x + BTN_SPACING + BTN_RADIUS * 2;
int btn120_x = btn60_x + BTN_SPACING + BTN_RADIUS * 2;
#define BTN_Y 80  // Y position for all control buttons
// Home button properties (square button at the upper left)
#define HOME_BTN_SIZE 30
#define HOME_BTN_X 0
#define HOME_BTN_Y 0
// Render button properties (square button in the upper right)
#define RENDER_BTN_SIZE 30
#define RENDER_BTN_X SCREEN_WIDTH - RENDER_BTN_SIZE
#define RENDER_BTN_Y 0

// Start angle button properties
#define START_BTN_RADIUS 50
#define START_BTN_Y 100
#define START_BTN_X (SCREEN_WIDTH / 4)

#define NUM_DEGREES 361 // 0 to 180 degrees and -180 to 180 degrees (361 total)
#define ANGLE_LIST_OFFSET 180       // To offset the index for negative angles

int16_t sinTable[NUM_DEGREES];
int16_t cosTable[NUM_DEGREES];

// Start motor button properties
#define START_MOTOR_BTN_RADIUS 50
#define START_MOTOR_BTN_Y 100
#define START_MOTOR_BTN_X (SCREEN_WIDTH / 4 * 3)

// Motor control button properties
#define BTN_SQUARE_SIZE 30
#define BTN_SQUARE_SPACING BTN_SQUARE_SIZE / 2
#define BTN_RECT_WIDTH 100

#define BTN_MOTOR_RESET_Y SCREEN_HEIGHT / 2
#define BTN_MOTOR_NEG_Y BTN_MOTOR_RESET_Y + BTN_SQUARE_SIZE + BTN_SQUARE_SPACING / 2
#define BTN_MOTOR_POS_Y BTN_MOTOR_NEG_Y

#define BTN_X_CENTER SCREEN_WIDTH / 2 - BTN_SQUARE_SIZE / 2

#define BTN_NEG_01_X BTN_X_CENTER - BTN_SQUARE_SIZE - BTN_SQUARE_SPACING
#define BTN_NEG_1_X BTN_NEG_01_X - BTN_SQUARE_SIZE - BTN_SQUARE_SPACING
#define BTN_NEG_5_X BTN_NEG_1_X - BTN_SQUARE_SIZE - BTN_SQUARE_SPACING
#define BTN_POS_01_X BTN_X_CENTER + BTN_SQUARE_SIZE + BTN_SQUARE_SPACING
#define BTN_POS_1_X BTN_POS_01_X + BTN_SQUARE_SIZE + BTN_SQUARE_SPACING
#define BTN_POS_5_X BTN_POS_1_X + BTN_SQUARE_SIZE + BTN_SQUARE_SPACING

#define BTN_SAVED_POS_WIDTH 70
#define BTN_SAVED_POS_HEIGHT 30
#define BTN_SAVED_POS_Y SCREEN_HEIGHT - BTN_SAVED_POS_HEIGHT - 5
#define BTN_SAVED_POS2_X SCREEN_WIDTH / 2 - BTN_SAVED_POS_WIDTH - 10
#define BTN_SAVED_POS1_X BTN_SAVED_POS2_X - BTN_SAVED_POS_WIDTH - 10
#define BTN_SAVED_POS3_X SCREEN_WIDTH / 2 + 10
#define BTN_SAVED_POS4_X BTN_SAVED_POS3_X + BTN_SAVED_POS_WIDTH + 10
float savedPositions[4] = {10.00, 21.00, 13.00, 9.00};

#define BTN_RESET_X SCREEN_WIDTH - BTN_RECT_WIDTH
#define BTN_RESET_Y 0

#define MOTOR_DISTANCE_Y SCREEN_HEIGHT / 2 - 10
#define MOTOR_DISTANCE_X SCREEN_WIDTH / 2

#define MOTOR_GO_SIZE BTN_SQUARE_SIZE * 1.5
#define MOTOR_GO_X MOTOR_DISTANCE_X - MOTOR_GO_SIZE / 2
#define MOTOR_GO_Y BTN_MOTOR_RESET_Y + 10
bool moving = false;
float motorDistance = 0.00;
#define dirPin 22 // Blue
#define stepPin 27 // Yellow
const int stepsPerRevolution = 800;  // change this to fit the number of steps per revolution
const int mmPerRev = 1;  // Distance moved per revolution in mm
const int rps = 4;  // Revolutions per second desired
const int motorDelay = 1000000 / (stepsPerRevolution * rps); // Delay in microseconds between steps
float currentMotorPosition = 0.00;


void setupTrigTables() {  // Fill the sine and cosine lookup tables for the visualization of angles
    for (int i = -180; i <= 180; i++) {
        int index = i + ANGLE_LIST_OFFSET; // Offset index to handle negative angles
        sinTable[index] = (int16_t)(sin(i * DEG_TO_RAD) * 1000); // Scale by 1000
        cosTable[index] = (int16_t)(cos(i * DEG_TO_RAD) * 1000); // Scale by 1000
    }
}

void drawCloseButton() {
    // Draw close button
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

void drawRenderButton(RenderMode currentRenderMode) {
    // Draw render button
    tft.fillRect(RENDER_BTN_X, RENDER_BTN_Y, RENDER_BTN_SIZE, RENDER_BTN_SIZE, TFT_BLACK);

    // Draw text inside the render button (F for fast, A for accurate)
    tft.setTextColor(TFT_WHITE);
    if (currentRenderMode == FAST) {
        tft.drawCentreString("F", RENDER_BTN_X + RENDER_BTN_SIZE / 2, RENDER_BTN_Y + RENDER_BTN_SIZE / 2, FONT_SIZE);
    } else {
        tft.drawCentreString("A", RENDER_BTN_X + RENDER_BTN_SIZE / 2, RENDER_BTN_Y + RENDER_BTN_SIZE / 2, FONT_SIZE);
    }
    // Write render mode text to the left of the button
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("Render Mode:", RENDER_BTN_X - 40, RENDER_BTN_Y + RENDER_BTN_SIZE / 2, FONT_SIZE);
}

void drawControlButtons() {
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
}

void drawMeasureButton(MeasureState currentMeasureState) {
    // Draw measure button
    tft.setTextColor(TFT_WHITE);
    if (currentMeasureState == IDLE) {
        tft.fillCircle(btnmeasure_x, BTN_Y, BTN_RADIUS, TFT_DARKGREY);
        tft.drawCentreString("IDLE", btnmeasure_x, BTN_Y - 10, FONT_SIZE);
    } else {
        tft.fillCircle(btnmeasure_x, BTN_Y, BTN_RADIUS, TFT_DARKGREEN);
        tft.drawCentreString("MEASURE", btnmeasure_x, BTN_Y - 10, FONT_SIZE);
    }
  }

void drawMotorDistance(float &motorDistance, float &currentMotorPosition) {
    // Clear old distance value
    tft.fillRect(0, SCREEN_HEIGHT / 2 - 80, SCREEN_WIDTH, 70, TFT_WHITE);

    // Set text color and size
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(FONT_SIZE * 2.5);

    // Draw current motor position
    tft.drawCentreString("Cur Position: " + String(currentMotorPosition) + " mm", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 70, FONT_SIZE);

    // Draw motor distance displayed in the center of the screen
    tft.drawCentreString("Des Position: " + String(motorDistance) + " mm", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30, FONT_SIZE);
}

// Draw the button to move the motor. Takes as input the color it should have
void drawMotorGoButton(bool moving) {
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(FONT_SIZE * 1.5);

    // Draw the button with the specified color
    if (moving){
        tft.fillRect(MOTOR_GO_X, MOTOR_GO_Y, MOTOR_GO_SIZE, MOTOR_GO_SIZE, TFT_RED);
        tft.drawCentreString("MOVING", MOTOR_GO_X + BTN_SQUARE_SIZE / 2 + 5, MOTOR_GO_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);
    } else {
        tft.fillRect(MOTOR_GO_X, MOTOR_GO_Y, MOTOR_GO_SIZE, MOTOR_GO_SIZE, TFT_GREEN);
        tft.drawCentreString("MOVE", MOTOR_GO_X + BTN_SQUARE_SIZE / 2 + 10, MOTOR_GO_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);
    }

}

void drawMotorControlButtons() {
    // Draw motor control buttons - should be square buttons with -20, -10, -5, reset, +5, +10, +20

    tft.setTextColor(TFT_WHITE);

    // Draw -20, -10, and -5 button (square buttons)
    tft.fillRect(BTN_NEG_5_X, BTN_MOTOR_NEG_Y, BTN_SQUARE_SIZE, BTN_SQUARE_SIZE, TFT_DARKGREY);
    tft.drawCentreString("-5.0", BTN_NEG_5_X + BTN_SQUARE_SIZE / 2, BTN_MOTOR_NEG_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);
    tft.fillRect(BTN_NEG_1_X, BTN_MOTOR_NEG_Y, BTN_SQUARE_SIZE, BTN_SQUARE_SIZE, TFT_DARKGREY);
    tft.drawCentreString("-1.0", BTN_NEG_1_X + BTN_SQUARE_SIZE / 2, BTN_MOTOR_NEG_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);
    tft.fillRect(BTN_NEG_01_X, BTN_MOTOR_NEG_Y, BTN_SQUARE_SIZE, BTN_SQUARE_SIZE, TFT_DARKGREY);
    tft.drawCentreString("-0.1", BTN_NEG_01_X + BTN_SQUARE_SIZE / 2, BTN_MOTOR_NEG_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);

    // Draw reset button (rectangular button)
    tft.fillRect(BTN_RESET_X, BTN_RESET_Y, BTN_RECT_WIDTH, BTN_SQUARE_SIZE, TFT_BLUE);
    tft.drawCentreString("RESET CUR POS", BTN_RESET_X + BTN_RECT_WIDTH / 2, BTN_RESET_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);

    // Draw +5, +10, and +20 button (square buttons)
    tft.fillRect(BTN_POS_01_X, BTN_MOTOR_POS_Y, BTN_SQUARE_SIZE, BTN_SQUARE_SIZE, TFT_DARKGREY);
    tft.drawCentreString("+0.1", BTN_POS_01_X + BTN_SQUARE_SIZE / 2, BTN_MOTOR_POS_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);
    tft.fillRect(BTN_POS_1_X, BTN_MOTOR_POS_Y, BTN_SQUARE_SIZE, BTN_SQUARE_SIZE, TFT_DARKGREY);
    tft.drawCentreString("+1.0", BTN_POS_1_X + BTN_SQUARE_SIZE / 2, BTN_MOTOR_POS_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);
    tft.fillRect(BTN_POS_5_X, BTN_MOTOR_POS_Y, BTN_SQUARE_SIZE, BTN_SQUARE_SIZE, TFT_DARKGREY);
    tft.drawCentreString("+5.0", BTN_POS_5_X + BTN_SQUARE_SIZE / 2, BTN_MOTOR_POS_Y + BTN_SQUARE_SIZE / 2, FONT_SIZE);

    // Define the line height based on the font size
    int lineHeight = FONT_SIZE * 10; // Adjust as needed

    // Draw buttons and display saved positions
    tft.fillRect(BTN_SAVED_POS1_X, BTN_SAVED_POS_Y, BTN_SAVED_POS_WIDTH, BTN_SAVED_POS_HEIGHT, TFT_DARKGREEN);
    tft.drawCentreString("POS 1", BTN_SAVED_POS1_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 - lineHeight / 2, FONT_SIZE);
    tft.drawCentreString(String(savedPositions[0], 2) + " mm", BTN_SAVED_POS1_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 + lineHeight / 2, FONT_SIZE);

    tft.fillRect(BTN_SAVED_POS2_X, BTN_SAVED_POS_Y, BTN_SAVED_POS_WIDTH, BTN_SAVED_POS_HEIGHT, TFT_ORANGE);
    tft.drawCentreString("POS 2", BTN_SAVED_POS2_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 - lineHeight / 2, FONT_SIZE);
    tft.drawCentreString(String(savedPositions[1], 2) + " mm", BTN_SAVED_POS2_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 + lineHeight / 2, FONT_SIZE);

    tft.fillRect(BTN_SAVED_POS3_X, BTN_SAVED_POS_Y, BTN_SAVED_POS_WIDTH, BTN_SAVED_POS_HEIGHT, TFT_DARKCYAN);
    tft.drawCentreString("POS 3", BTN_SAVED_POS3_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 - lineHeight / 2, FONT_SIZE);
    tft.drawCentreString(String(savedPositions[2], 2) + " mm", BTN_SAVED_POS3_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 + lineHeight / 2, FONT_SIZE);

    tft.fillRect(BTN_SAVED_POS4_X, BTN_SAVED_POS_Y, BTN_SAVED_POS_WIDTH, BTN_SAVED_POS_HEIGHT, TFT_PURPLE);
    tft.drawCentreString("POS 4", BTN_SAVED_POS4_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 - lineHeight / 2, FONT_SIZE);
    tft.drawCentreString(String(savedPositions[3], 2) + " mm", BTN_SAVED_POS4_X + BTN_SAVED_POS_WIDTH / 2, BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT / 2 + lineHeight / 2, FONT_SIZE);


    // Draw motor distance displayed in the center top part of the screen
    drawMotorDistance(motorDistance, currentMotorPosition);

    drawMotorGoButton(moving = false);
}

// Function to draw round buttons
void drawButtons(RenderMode currentRenderMode, MeasureState currentMeasureState) {
    // Draw control buttons
    drawControlButtons();

    // Draw home button
    drawCloseButton();

    // Draw render button
    drawRenderButton(currentRenderMode);

    // Draw measurement button
    drawMeasureButton(currentMeasureState);
}

// Function to draw the home screen
void drawHomeScreen() {
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(FONT_SIZE);
  tft.drawCentreString("Home Screen", SCREEN_WIDTH / 2, 30, FONT_SIZE);

  // Draw Motor button
  tft.fillCircle(START_MOTOR_BTN_X, START_MOTOR_BTN_Y, START_MOTOR_BTN_RADIUS, TFT_GREEN);
  tft.drawCentreString("Start Motor", START_MOTOR_BTN_X, START_MOTOR_BTN_Y - 10, FONT_SIZE);

  // Draw Start button
  tft.fillCircle(START_BTN_X, START_BTN_Y, START_BTN_RADIUS, TFT_GREEN);
  tft.drawCentreString("Start Angle", START_BTN_X, START_BTN_Y - 10, FONT_SIZE);
}

// Function to draw the motor screen
void drawMotorScreen() {
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(FONT_SIZE);
    tft.drawCentreString("Motor Screen", SCREEN_WIDTH / 2, 30, FONT_SIZE);
    
    // Draw Home button
    drawCloseButton();

    // Draw motor control buttons
    drawMotorControlButtons();

}

// // Function to draw filled arcs (part-circles)
void drawArc(int centerX, int centerY, int radius, int angle, uint16_t color) {
    // Normalize the angle to be between 0 and 180
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    int multiplierValue = 1;

    // Determine a suitable multiplier value to increase drawing speed (but reduce resolution)
    if (angle < 10) {
        multiplierValue = 1;  // For very small angles, use the finest granularity
    } else if (angle < 20) {
        multiplierValue = 2;  // Small angles, moderate granularity
    } else {
        multiplierValue = 4;  // For angles greater than or equal to 20, the lowest granularity
    }
    // Round down the angle to the nearest multiple of the multiplier value
    int visAngle = (angle / multiplierValue) * multiplierValue;

    // Calculate start and end angles based on the provided angle
    int startAngle = -visAngle / 2;  // Negative angle for clockwise drawing
    int endAngle = visAngle / 2;

    // Draw the arc as filled triangles
    for (int i = startAngle; i < endAngle; i+=multiplierValue) {
        // Ensure indices are within valid range for the lookup table
        int index1 = i + ANGLE_LIST_OFFSET;  // Normalize to positive index
        int index2 = (i + multiplierValue) + ANGLE_LIST_OFFSET;  // Next angle

        int16_t sinVal1 = sinTable[index1];
        int16_t cosVal1 = cosTable[index1];
        int16_t sinVal2 = sinTable[index2 ];
        int16_t cosVal2 = cosTable[index2];

        int16_t x_1_add = (radius * sinVal1) / 1000;
        int16_t y_1_add = (radius * cosVal1) / 1000;
        int16_t x_2_add = (radius * sinVal2) / 1000;
        int16_t y_2_add = (radius * cosVal2) / 1000;

        // Use lookup table values
        int x1 = centerX + x_1_add;
        int y1 = centerY - y_1_add;
        int x2 = centerX + x_2_add;
        int y2 = centerY - y_2_add;

        // Draw filled triangle from center to arc edges
        tft.fillTriangle(centerX, centerY, x1, y1, x2, y2, color);
    }
}

void drawAngleLines(int centerX, int centerY, int radius, int angle, uint16_t color) {
    // Draw lines for the angles (only for the start and end angles)
    int startAngle = -angle / 2;
    int endAngle = angle / 2;

    // Calculate the start and end points of the lines
    int x1 = centerX + (radius * sinTable[startAngle + ANGLE_LIST_OFFSET]) / 1000;
    int y1 = centerY - (radius * cosTable[startAngle + ANGLE_LIST_OFFSET]) / 1000;
    int x2 = centerX + (radius * sinTable[endAngle + ANGLE_LIST_OFFSET]) / 1000;
    int y2 = centerY - (radius * cosTable[endAngle + ANGLE_LIST_OFFSET]) / 1000;

    // Draw the lines
    tft.drawLine(centerX, centerY, x1, y1, color);
    tft.drawLine(centerX, centerY, x2, y2, color);
}



// Function to clear the rectangular area around the arc
void clearArcBoundingBox(int centerX, int centerY, int radius) {
    // The bounding box will be a square of size 2 * radius centered at (centerX, centerY)
    int x = centerX - radius;
    int y = centerY - radius;

    // Clear the bounding box by filling it with the background color (e.g., TFT_WHITE) (and 5 pixel extra on all sides)
    tft.fillRect(x, y, 2 * radius + 10, 2 * radius + 10, TFT_WHITE);
}


// Function to visualize the angles with everything outside them in green
void drawAngleVisualization(int angle_measured, int angle_pred_springback, ScreenState currentScreen, RenderMode currentRenderMode) {

    // Set the center at the bottom middle of the screen
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT;
    int radius = 120;  // Radius for the angle visualization

    if (currentScreen == ANGLE) {
        // Clear the rectangular area around the arc
        clearArcBoundingBox(centerX, centerY, radius);
    } else if (currentScreen == HOME) {
        // Clear the full screen and draw the buttons
        tft.fillScreen(TFT_WHITE);
        drawButtons(currentRenderMode, currentMeasureState);
    }

  if (currentRenderMode == ACCURATE) {
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
  else if (currentRenderMode == FAST) {
    // Draw only lines for the angles (no filled arcs)
    drawAngleLines(centerX, centerY, radius, angle_measured, TFT_RED);
    drawAngleLines(centerX, centerY, radius, angle_pred_springback, TFT_BLUE);

    // Display the current measured angle at the bottom of the screen
    String angleText = "Measured Angle = " + String(angle_measured) + "deg";
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString(angleText, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, FONT_SIZE);
    }
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
    } else {
        return -1;  // No button pressed
    }
}

// Check if the render button was pressed
bool checkRenderButtonPress(int touchX, int touchY) {
    return touchX >= RENDER_BTN_X && touchX <= RENDER_BTN_X + RENDER_BTN_SIZE &&
           touchY >= RENDER_BTN_Y && touchY <= RENDER_BTN_Y + RENDER_BTN_SIZE;
}
// Check if the start button was pressed
bool checkStartButtonPress(int touchX, int touchY) {
  return sqrt(sq(touchX - START_BTN_X) + sq(touchY - START_BTN_Y)) <= START_BTN_RADIUS;
}
// Check if the start motor button was pressed
bool checkStartMotorButtonPress(int touchX, int touchY) {
  return sqrt(sq(touchX - START_MOTOR_BTN_X) + sq(touchY - START_MOTOR_BTN_Y)) <= START_MOTOR_BTN_RADIUS;
}
// Check if the home button was pressed
bool checkHomeButtonPress(int touchX, int touchY) {
  return sqrt(sq(touchX - HOME_BTN_X) + sq(touchY - HOME_BTN_Y)) <= HOME_BTN_SIZE;
}
// Check if the measure button was pressed
bool checkMeasureButtonPress(int touchX, int touchY) {
  return sqrt(sq(touchX - btnmeasure_x) + sq(touchY - BTN_Y)) <= BTN_RADIUS;
}

// Check if the move button was pressed
bool checkMoveButtonPress(int touchX, int touchY) {
    return touchX >= MOTOR_GO_X && touchX <= MOTOR_GO_X + MOTOR_GO_SIZE &&
           touchY >= MOTOR_GO_Y && touchY <= MOTOR_GO_Y + MOTOR_GO_SIZE;
}

// Add value to motor distance (but make checks to ensure it is within valid range)
void addMotorDistance(float &motorDistance, float value) {
    motorDistance += value;
    // Ensure motor distance is within valid range
    if (motorDistance < 0) {
        motorDistance = 0;
    }
}

// Check if the motor control button was pressed
bool getMotorDistanceUpdate(int touchX, int touchY, float &motorDistance, float &currentMotorPosition) {
    printf("TouchX: %d, TouchY: %d\n", touchX, touchY);
    // Check if the press is within any button's square
    if (touchX >= BTN_NEG_5_X && touchX <= (BTN_NEG_5_X + BTN_SQUARE_SIZE) &&
        touchY >= BTN_MOTOR_NEG_Y && touchY <= (BTN_MOTOR_NEG_Y + BTN_SQUARE_SIZE)) {
        addMotorDistance(motorDistance, -5);
        return true;
    } else if (touchX >= BTN_NEG_1_X && touchX <= (BTN_NEG_1_X + BTN_SQUARE_SIZE) &&
               touchY >= BTN_MOTOR_NEG_Y && touchY <= (BTN_MOTOR_NEG_Y + BTN_SQUARE_SIZE)) {
        addMotorDistance(motorDistance, -1);
        return true;
    } else if (touchX >= BTN_NEG_01_X && touchX <= (BTN_NEG_01_X + BTN_SQUARE_SIZE) &&
               touchY >= BTN_MOTOR_NEG_Y && touchY <= (BTN_MOTOR_NEG_Y + BTN_SQUARE_SIZE)) {
        addMotorDistance(motorDistance, -0.1);
        return true;
    } else if (touchX >= BTN_POS_01_X && touchX <= (BTN_POS_01_X + BTN_SQUARE_SIZE) &&
               touchY >= BTN_MOTOR_POS_Y && touchY <= (BTN_MOTOR_POS_Y + BTN_SQUARE_SIZE)) {
        addMotorDistance(motorDistance, 0.1);
        return true;
    } else if (touchX >= BTN_POS_1_X && touchX <= (BTN_POS_1_X + BTN_SQUARE_SIZE) &&
               touchY >= BTN_MOTOR_POS_Y && touchY <= (BTN_MOTOR_POS_Y + BTN_SQUARE_SIZE)) {
        addMotorDistance(motorDistance, 1);
        return true;
    } else if (touchX >= BTN_POS_5_X && touchX <= (BTN_POS_5_X + BTN_SQUARE_SIZE) &&
               touchY >= BTN_MOTOR_POS_Y && touchY <= (BTN_MOTOR_POS_Y + BTN_SQUARE_SIZE)) {
        addMotorDistance(motorDistance, 5);
        return true;
    } else if (touchX >= BTN_SAVED_POS1_X && touchX <= (BTN_SAVED_POS1_X + BTN_SAVED_POS_WIDTH) &&
               touchY >= BTN_SAVED_POS_Y && touchY <= (BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT)) {
        motorDistance = savedPositions[0];
        return true;
    } else if (touchX >= BTN_SAVED_POS2_X && touchX <= (BTN_SAVED_POS2_X + BTN_SAVED_POS_WIDTH) &&
               touchY >= BTN_SAVED_POS_Y && touchY <= (BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT)) {
        motorDistance = savedPositions[1];
        return true;
    } else if (touchX >= BTN_SAVED_POS3_X && touchX <= (BTN_SAVED_POS3_X + BTN_SAVED_POS_WIDTH) &&
               touchY >= BTN_SAVED_POS_Y && touchY <= (BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT)) {
        motorDistance = savedPositions[2];
        return true;
    } else if (touchX >= BTN_SAVED_POS4_X && touchX <= (BTN_SAVED_POS4_X + BTN_SAVED_POS_WIDTH) &&
               touchY >= BTN_SAVED_POS_Y && touchY <= (BTN_SAVED_POS_Y + BTN_SAVED_POS_HEIGHT)) {
        motorDistance = savedPositions[3];
        return true;
    } else if (touchX >= BTN_RESET_X && touchX <= (BTN_RESET_X + BTN_RECT_WIDTH) &&
               touchY >= BTN_RESET_Y && touchY <= (BTN_RESET_Y + BTN_SQUARE_SIZE)) {
        currentMotorPosition = 0;
        return true;
    } else {
        return false;  // No button pressed
    }
}

void moveMotor(float &motorDistance, float &currentMotorPosition) {
    // Move the motor to the specified distance
    // For simplicity, we will just print the motor distance
    printf("Current motor position: %.2f mm\n", currentMotorPosition);
    printf("Moving motor to distance: %.2f mm\n", motorDistance);

    // Calculate the number of steps to move the motor
    float distToMove = (motorDistance - currentMotorPosition);
    // If negative, move in the opposite direction
    if (distToMove < 0) {
        digitalWrite(dirPin, LOW);  // TODO: Maybe flip the direction depending on setup
    } else {
        digitalWrite(dirPin, HIGH); // TODO: Maybe flip the direction depending on setup
    }
    int stepsToMove = abs(distToMove) * stepsPerRevolution / mmPerRev;

    for (int i = 0; i <= stepsToMove; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorDelay);
    }
    // Update the current motor position
    currentMotorPosition = motorDistance;
}

// Calculate springback angle based on the measured angle and the material properties
int calculateSpringbackAngle(int measuredAngle) {
    // Springback angle is calculated based on the measured angle and the material properties
    // For simplicity, we will assume that the springback angle is 10% of the measured angle
    return measuredAngle + (180 - measuredAngle) * 0.1;
}

// Function to retrieve touch coordinates
bool getTouchCoordinates(int &touchX, int &touchY) {
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();
        // Map touchscreen points to screen coordinates
        touchX = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
        touchY = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
        return true;  // Touch detected
    }
    return false;  // No touch detected
}

bool getMeasuredAngle(int &angleMeasured) {
  // FOR I/O INPUT:
  // Check if the angle has changed
  // int newAngle = analogRead(A0);  // Read the angle from the analog pin
  // Make sure angle is within 0 to 180 degrees
  // if (newAngle < 0) {
  //     newAngle = 0;
  // } else if (newAngle > 180) {
  //     newAngle = 180;
  // }

  // FOR SIMULATION:
    static int lastAngle = 180;  // Start at 180 degrees
    static int angleDirection = -1;  // Start by decreasing the angle (from 180 to 0)
    // Update angle based on direction
    int newAngle = lastAngle + angleDirection;
    // Reverse direction when limits are reached (180 or 0 degrees)
    if (newAngle > 160) {
        newAngle = 160;
        angleDirection = -1;  // Start decreasing
    } else if (newAngle < 40) {
        newAngle = 40;
        angleDirection = 1;  // Start increasing
    }


    // Check if the angle has changed
    if (newAngle != angleMeasured) {
        angleMeasured = newAngle;
        lastAngle = newAngle;  // Update lastAngle
        return true;  // Angle updated
    }
    return false;  // No angle update
}

// Function to handle the home screen
void handleHomeScreen(int touchX, int touchY) {
    if (checkStartButtonPress(touchX, touchY)) {
        drawAngleVisualization(angleMeasured, angle_pred_springback, currentScreen, currentRenderMode);
        currentScreen = ANGLE;  // Switch to angle screen
    }
    if (checkStartMotorButtonPress(touchX, touchY)) {
        drawMotorScreen();
        currentScreen = MOTOR;  // Switch to motor screen
    }
}

// Function to handle the angle screen
void handleAngleScreen(int touchX, int touchY, int angleMeasured) {
  // Check if Back button is pressed
  if (checkHomeButtonPress(touchX, touchY)) {
      currentScreen = HOME;  // Switch back to home screen
      drawHomeScreen();
      return;
  }

  // Check if render button was pressed
  if (checkRenderButtonPress(touchX, touchY)) {
      currentRenderMode = (currentRenderMode == FAST) ? ACCURATE : FAST;
      drawRenderButton(currentRenderMode);
      drawAngleVisualization(angleMeasured, angle_pred_springback, currentScreen, currentRenderMode);
  }

  // Check if Measure button was pressed
  if (checkMeasureButtonPress(touchX, touchY)) {
      currentMeasureState = (currentMeasureState == IDLE) ? MEASURING : IDLE;
      drawMeasureButton(currentMeasureState);
  }

  if (currentMeasureState == MEASURING) {
      angle_pred_springback = calculateSpringbackAngle(angleMeasured);
      drawAngleVisualization(angleMeasured, angle_pred_springback, currentScreen, currentRenderMode);
  } else {
    return;  
  }
}

// Function to handle the motor screen
void handleMotorScreen(int touchX, int touchY, float &motorDistance) {

    // Check if Back button is pressed
    if (checkHomeButtonPress(touchX, touchY)) {
        currentScreen = HOME;  // Switch back to home screen
        drawHomeScreen();
        return;
    }

    // check if the move button was pressed
    if (checkMoveButtonPress(touchX, touchY)) {
        moving = !moving;
        drawMotorGoButton(moving);
        moveMotor(motorDistance, currentMotorPosition);
        moving = !moving;
        drawMotorGoButton(moving);
    }

    // Check if any of the motor control buttons were pressed
    drawMotorDistance(motorDistance, currentMotorPosition);

}


// Setup function - runs once
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

  // Fill the sine and cosine lookup tables
  setupTrigTables();

  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

// Main loop - runs continuously after setup
void loop() {
    int touchX, touchY;
    bool touchDetected = getTouchCoordinates(touchX, touchY);

    int angleMeasured;
    bool angleUpdated = getMeasuredAngle(angleMeasured);

    bool motorDistanceUpdated = getMotorDistanceUpdate(touchX, touchY, motorDistance, currentMotorPosition);
    

    // Process screen state whether touch is detected or from other inputs
    switch (currentScreen) {
        case HOME:
            if (touchDetected) {
                handleHomeScreen(touchX, touchY);
            }
            break;

        case ANGLE:
            if (touchDetected || angleUpdated) {
                handleAngleScreen(touchX, touchY, angleMeasured);
            }
            break;
        
        case MOTOR:
            if (touchDetected) {
                printf("Motor Distance: %f\n", motorDistance);
                handleMotorScreen(touchX, touchY, motorDistance);
            }
            break;
    }

    delay(100);  // Small delay to debounce touchscreen press or slow down updates
}