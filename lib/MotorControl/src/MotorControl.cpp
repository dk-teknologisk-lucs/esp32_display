# include "MotorControl.h"

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