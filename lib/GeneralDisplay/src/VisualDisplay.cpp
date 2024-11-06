#include <VisualDisplay.h>

TFT_eSPI tft = TFT_eSPI(); // Define the display object once here

int16_t sinTable[NUM_DEGREES];
int16_t cosTable[NUM_DEGREES];

//// GENERAL DISPLAYS ////
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

//// HOME SCREEN ////

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

//// ANGLE SCREEN ////
void drawMeasureButton(MeasureState currentMeasureState) {
    // Draw measure button
    tft.setTextColor(TFT_WHITE);
    if (currentMeasureState == IDLE) {
        tft.fillCircle(BTN_MEASURE_X, BTN_Y, BTN_RADIUS, TFT_DARKGREY);
        tft.drawCentreString("IDLE", BTN_MEASURE_X, BTN_Y - 10, FONT_SIZE);
    } else {
        tft.fillCircle(BTN_MEASURE_X, BTN_Y, BTN_RADIUS, TFT_DARKGREEN);
        tft.drawCentreString("MEASURE", BTN_MEASURE_X, BTN_Y - 10, FONT_SIZE);
    }
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
    tft.fillCircle(BTN_20_X, BTN_Y, BTN_RADIUS, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("20deg", BTN_20_X, BTN_Y - 10, FONT_SIZE);

    // Draw button for 60 degrees
    tft.fillCircle(BTN_60_X, BTN_Y, BTN_RADIUS, TFT_BLUE);
    tft.drawCentreString("60deg", BTN_60_X, BTN_Y - 10, FONT_SIZE);

    // Draw button for 120 degrees
    tft.fillCircle(BTN_120_X, BTN_Y, BTN_RADIUS, TFT_BLUE);
    tft.drawCentreString("120deg", BTN_120_X, BTN_Y - 10, FONT_SIZE);
}

// Function to generate a list of angles for the visualization (for fast rendering)
void setupTrigTables() {  // Fill the sine and cosine lookup tables for the visualization of angles
    for (int i = -180; i <= 180; i++) {
        int index = i + ANGLE_LIST_OFFSET; // Offset index to handle negative angles
        sinTable[index] = (int16_t)(sin(i * DEG_TO_RAD) * 1000); // Scale by 1000
        cosTable[index] = (int16_t)(cos(i * DEG_TO_RAD) * 1000); // Scale by 1000
    }
}

// Function to clear the rectangular area around the arc
void clearArcBoundingBox(int centerX, int centerY, int radius) {
    // The bounding box will be a square of size 2 * radius centered at (centerX, centerY)
    int x = centerX - radius;
    int y = centerY - radius;

    // Clear the bounding box by filling it with the background color (e.g., TFT_WHITE) (and 5 pixel extra on all sides)
    tft.fillRect(x, y, 2 * radius + 10, 2 * radius + 10, TFT_WHITE);
}

// Function to draw filled arcs (part-circles)
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
        int16_t sinVal2 = sinTable[index2];
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

// Draw the angles as lines (without filling the arcs)
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

// Draw buttons on Angle screen
void drawAngleScreenButtons(RenderMode currentRenderMode, MeasureState currentMeasureState) {
    // Draw control buttons
    drawControlButtons();

    // Draw home button
    drawCloseButton();

    // Draw render button
    drawRenderButton(currentRenderMode);

    // Draw measurement button
    drawMeasureButton(currentMeasureState);
}



// Function to visualize the angles with everything outside them in green
void drawAngleVisualization(int angle_measured, int angle_pred_springback, ScreenState currentScreen, RenderMode currentRenderMode, MeasureState currentMeasureState) {

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
        drawAngleScreenButtons(currentRenderMode, currentMeasureState);
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







//// MOTOR SCREEN ////

// Draw the motor current distance and setpoint distance as text
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

// Draw the button to move the motor which changes color based on the motor state (moving or not)
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

void drawMotorControlButtons(float &motorDistance, float &currentMotorPosition, bool moving, float savedPositions[]) {
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

// Function to draw the motor screen
void drawMotorScreen(float &motorDistance, float &currentMotorPosition, bool moving, float savedPositions[]) {
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(FONT_SIZE);
    tft.drawCentreString("Motor Screen", SCREEN_WIDTH / 2, 30, FONT_SIZE);
    
    // Draw Home button
    drawCloseButton();

    // Draw motor control buttons
    drawMotorControlButtons(motorDistance, currentMotorPosition, moving, savedPositions);
}