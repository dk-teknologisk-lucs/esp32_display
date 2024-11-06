#ifndef VISUAL_DISPLAY_H
#define VISUAL_DISPLAY_H

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

extern TFT_eSPI tft; // Declare the display object as extern


enum ScreenState {
  HOME,
  ANGLE,
  MOTOR
};
enum RenderMode {
  FAST,
  ACCURATE
};
enum MeasureState {
  MEASURING,
  IDLE
};

// GENERAL display configuration
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 1


#define BTN_RADIUS 25
#define BTN_SPACING 25

// RETURN to home button properties (square button at the upper left)
#define HOME_BTN_SIZE 30
#define HOME_BTN_X 0
#define HOME_BTN_Y 0

// HOME SCREEN button properties
#define START_MOTOR_BTN_RADIUS 50
#define START_MOTOR_BTN_Y 100
#define START_MOTOR_BTN_X (SCREEN_WIDTH / 4 * 3)

#define START_BTN_RADIUS 50
#define START_BTN_Y 100
#define START_BTN_X (SCREEN_WIDTH / 4)

// ANGLE SCREEN button properties

#define NUM_DEGREES 361 // 0 to 180 degrees and -180 to 180 degrees (361 total)
#define ANGLE_LIST_OFFSET 180       // To offset the index for negative angles

// extern int16_t sinTable[NUM_DEGREES];
// extern int16_t cosTable[NUM_DEGREES];

#define BTN_MEASURE_X 40  // X position for the reset button
#define BTN_20_X BTN_MEASURE_X + BTN_RADIUS * 2 + BTN_SPACING
#define BTN_60_X BTN_20_X + BTN_SPACING + BTN_RADIUS * 2
#define BTN_120_X BTN_60_X + BTN_SPACING + BTN_RADIUS * 2
#define BTN_Y 80  // Y position for all control buttons

#define RENDER_BTN_SIZE 30
#define RENDER_BTN_X SCREEN_WIDTH - RENDER_BTN_SIZE
#define RENDER_BTN_Y 0


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

#define BTN_RESET_X SCREEN_WIDTH - BTN_RECT_WIDTH
#define BTN_RESET_Y 0

#define MOTOR_DISTANCE_Y SCREEN_HEIGHT / 2 - 10
#define MOTOR_DISTANCE_X SCREEN_WIDTH / 2

#define MOTOR_GO_SIZE BTN_SQUARE_SIZE * 1.5
#define MOTOR_GO_X MOTOR_DISTANCE_X - MOTOR_GO_SIZE / 2
#define MOTOR_GO_Y BTN_MOTOR_RESET_Y + 10


void drawCloseButton();

void drawHomeScreen();


void setupTrigTables();

void clearArcBoundingBox(int centerX, int centerY, int radius);

void drawMeasureButton(MeasureState currentMeasureState);

void drawRenderButton(RenderMode currentRenderMode);

void drawControlButtons();

void drawArc(int centerX, int centerY, int radius, int angle, uint16_t color);

void drawAngleLines(int centerX, int centerY, int radius, int angle, uint16_t color);

void drawAngleScreenButtons(RenderMode currentRenderMode, MeasureState currentMeasureState);

void drawAngleVisualization(int angle_measured, int angle_pred_springback, ScreenState currentScreen, RenderMode currentRenderMode, MeasureState currentMeasureState);




void drawMotorDistance(float &motorDistance, float &currentMotorPosition);

void drawMotorGoButton(bool moving);

void drawMotorControlButtons(float &motorDistance, float &currentMotorPosition, bool moving, float savedPositions[]);

void drawMotorScreen(float &motorDistance, float &currentMotorPosition, bool moving, float savedPositions[]);

#endif