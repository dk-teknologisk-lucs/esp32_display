#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>


#define dirPin 22 // Blue
#define stepPin 27 // Yellow

const int stepsPerRevolution = 800;  // change this to fit the number of steps per revolution
const int mmPerRev = 1;  // Distance moved per revolution in mm
const int rps = 4;  // Revolutions per second desired
const int motorDelay = 1000000 / (stepsPerRevolution * rps); // Delay in microseconds between steps

void moveMotor(float &motorDistance, float &currentMotorPosition);

void addMotorDistance(float &motorDistance, float value);

bool getMotorDistanceUpdate(int touchX, int touchY, float &motorDistance, float &currentMotorPosition, float savedPositions[]);

#endif