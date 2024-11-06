#ifndef GENERAL_DISPLAY_H
#define GENERAL_DISPLAY_H

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Display configuration
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 1

// Home button properties (square button at the upper left)
#define HOME_BTN_SIZE 30
#define HOME_BTN_X 0
#define HOME_BTN_Y 0

extern TFT_eSPI tft; // Declare the display object as extern

void drawCloseButton();

#endif