# ESP Display

- Board: ESP32-2432S028
- ESP type: ESP-WROOM-32

# Setup using VSCode
For the first test use: [this tutorial for the display](https://randomnerdtutorials.com/programming-esp32-cyd-cheap-yellow-display-vs-code/) and [this tutorial for VSCode setup](https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/)

Use the following:
- Board: upesy_wroom
- Framework: Arduino
- Update libdeps (is already added to the platformio.ini file)
- Replace the generated `.pio/libdeps/uepsy_wroom/TFT_eSPI/user_setup.h` with the `setup_files_backup/User_Setup.h` file (`#define TFT_RGB_ORDER TFT_BGR` and `#define TFT_INVERSION_ON` uncommented)

# USB permission from computer
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


# TODO:

- [ ] Implement SquareLine UI or LVGL to imporve UI
    - https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/discussions/102
    - https://randomnerdtutorials.com/lvgl-cheap-yellow-display-esp32-2432s028r/
- [ ] Implement inputs from outside (to adjust the angle)
- [ ] Implement outputs (to control motors)
