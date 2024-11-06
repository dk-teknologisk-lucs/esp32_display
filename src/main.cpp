#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <VisualDisplay.h>
#include <MotorControl.h>
#include <AngleLogic.h>

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

ScreenState currentScreen = HOME;  // Track the current screen state
RenderMode currentRenderMode = FAST;
MeasureState currentMeasureState = IDLE;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 1

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

int angleMeasured = 180;  // Default measured angle
int angle_pred_springback = 0;  // Default predicted springback angle

// Motor control properties
float savedPositions[4] = {10.00, 21.00, 13.00, 9.00};
bool moving = false;
float motorDistance = 0.00;
float currentMotorPosition = 0.00;


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

// Check if a round button was pressed and return the corresponding angle
int checkFixAngleButtonPress(int touchX, int touchY) {
    // Check if the press is within any button's circle
    if (sqrt(sq(touchX - BTN_20_X) + sq(touchY - BTN_Y)) <= BTN_RADIUS) {
        return 20;
    } else if (sqrt(sq(touchX - BTN_60_X) + sq(touchY - BTN_Y)) <= BTN_RADIUS) {
        return 60;
    } else if (sqrt(sq(touchX - BTN_120_X) + sq(touchY - BTN_Y)) <= BTN_RADIUS) {
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
  return sqrt(sq(touchX - BTN_MEASURE_X) + sq(touchY - BTN_Y)) <= BTN_RADIUS;
}

// Check if the move button was pressed
bool checkMoveButtonPress(int touchX, int touchY) {
    return touchX >= MOTOR_GO_X && touchX <= MOTOR_GO_X + MOTOR_GO_SIZE &&
           touchY >= MOTOR_GO_Y && touchY <= MOTOR_GO_Y + MOTOR_GO_SIZE;
}


// Function to handle the home screen
void handleHomeScreen(int touchX, int touchY) {
    if (checkStartButtonPress(touchX, touchY)) {
        drawAngleVisualization(angleMeasured, angle_pred_springback, currentScreen, currentRenderMode, currentMeasureState);
        currentScreen = ANGLE;  // Switch to angle screen
    }
    if (checkStartMotorButtonPress(touchX, touchY)) {
        drawMotorScreen(motorDistance, currentMotorPosition, moving, savedPositions);
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
      drawAngleVisualization(angleMeasured, angle_pred_springback, currentScreen, currentRenderMode, currentMeasureState);
  }

  // Check if Measure button was pressed
  if (checkMeasureButtonPress(touchX, touchY)) {
      currentMeasureState = (currentMeasureState == IDLE) ? MEASURING : IDLE;
      drawMeasureButton(currentMeasureState);
  }

  if (currentMeasureState == MEASURING) {
      angle_pred_springback = calculateSpringbackAngle(angleMeasured);
      drawAngleVisualization(angleMeasured, angle_pred_springback, currentScreen, currentRenderMode, currentMeasureState);
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

    bool motorDistanceUpdated = getMotorDistanceUpdate(touchX, touchY, motorDistance, currentMotorPosition, savedPositions);
    

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