# ESP32 Screen

This repo contains code that uses an ESP32 Screen to show the angle and controls a stepper motor.
It has a main menu to choose between the two operational modes and in each you have different controls and visuals.


# Hardware and wiring
## ESP Display Hardware:
- Board: ESP32-2432S028
- ESP type: ESP-WROOM-32
- Stepper: 17HS4401 (somehow the version we have in hand has an angle step of 0.45° = 360/0.45 = 800 steps/rev)
- Stepper driver: [MicrostepDriver, TB6600](https://arduinotech.dk/shop/tb6600-stepper-motor-driver-controller-4a-942v-ttl-16-micro-step-cnc-1-axis-new-upgraded-version)

## Wiring

### ESP32-screen - Motor Driver:
- Screen: Use CN1 port (the other will not work, as GPIO35 is only input and 21 is used for backlight screen)
    - Red: Not used (3.3V)
    - Yellow: Step (GPIO 27)
    - Blue: Dir (GPIO 22)
    - Black: GND
- Motor Driver
    - EN: active_low (Should be GND)

### Motor Driver - Stepper Motor:
Blue & Yellow: phase 1
Red & Green: phase 2


# Development:

## Custom libs

Som custom libraries has been made. Even though they are not necessarily reusable they are used to maintain a fairly short main.cpp file.
These are placed in the /lib folder.

## Setup using VSCode
For the first test use: [this tutorial for the display](https://randomnerdtutorials.com/programming-esp32-cyd-cheap-yellow-display-vs-code/) and [this tutorial for VSCode setup](https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/)

Use the following:
- Board: upesy_wroom
- Framework: Arduino
- Update libdeps (is already added to the platformio.ini file)
- Replace the generated `.pio/libdeps/uepsy_wroom/TFT_eSPI/user_setup.h` with the `setup_files_backup/User_Setup.h` file (`#define TFT_RGB_ORDER TFT_BGR` and `#define TFT_INVERSION_ON` uncommented)

## USB permission from computer
1. Test if board is recognised
    ```bash
    ls /dev/ttyUSB*
    ```
1. Make sure user is in dialout group
    ```bash
    groups
    sudo usermod -aG dialout $USER
    ```
    Log out and in to make the change


## TODOs

Need:
- [x] Displays how the angle continously is simulated and shown on the screen
- [x] Can control a stepper motor
- [x] Connecs inputs and stepper motor with presaved distances, simple calibration and regular control (back and forth)

Nice:
- [ ] Get angle inputs from another ESP
- [ ] Save and load configs (like the presaved motor-distances) to a SD card
- [ ] Implement inputs from outside (to adjust the angle)
- [ ] Stepper motor control with ramping up acceleration (maybe make it work with one of the stepper libraries)
- [ ] Implement SquareLine UI or LVGL to imporve UI
    - https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/discussions/102
    - https://randomnerdtutorials.com/lvgl-cheap-yellow-display-esp32-2432s028r/