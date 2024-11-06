#include "MotorControl.h"
#include <VisualDisplay.h>

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


// Add value to motor distance (but make checks to ensure it is within valid range)
void addMotorDistance(float &motorDistance, float value) {
    motorDistance += value;
    // Ensure motor distance is within valid range
    // if (motorDistance < 0) {
    //     motorDistance = 0;
    // }
}

// Check if the motor control button was pressed
bool getMotorDistanceUpdate(int touchX, int touchY, float &motorDistance, float &currentMotorPosition, float savedPositions[]) {
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