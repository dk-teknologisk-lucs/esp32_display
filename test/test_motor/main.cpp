#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Stepper.h>
#include "AccelStepper.h"

// Define stepper motor connections:
// OBS: Pin 21 used for the backlight as well.
#define dirPin 35 // Blue
#define stepPin 22 // Yellow
#define motorInterfaceType 1 // Using a driver

const int stepsPerRevolution = 800;  // change this to fit the number of steps per revolution
const int mmPerRev = 1;  // Distance moved per revolution in mm

void setup() {
  Serial.begin(115200);

  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Set the spinning direction CW/CCW:
  digitalWrite(dirPin, HIGH);
}

void loop() {
  for (int i = 0; i <= stepsPerRevolution; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(500);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(500);
  }
  delay(2000);
}