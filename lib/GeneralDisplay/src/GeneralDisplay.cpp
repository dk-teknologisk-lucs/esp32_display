#include <GeneralDisplay.h>

TFT_eSPI tft = TFT_eSPI(); // Define the display object once here

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