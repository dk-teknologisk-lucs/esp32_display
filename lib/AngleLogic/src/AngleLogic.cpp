#include "AngleLogic.h"

// Calculate springback angle based on the measured angle and the material properties
int calculateSpringbackAngle(int measuredAngle) {
    // Springback angle is calculated based on the measured angle and the material properties
    // For simplicity, we will assume that the springback angle is 10% of the measured angle
    return measuredAngle + (180 - measuredAngle) * 0.1;
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