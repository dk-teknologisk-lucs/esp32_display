# Setup


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

For the first test use:
https://randomnerdtutorials.com/programming-esp32-cyd-cheap-yellow-display-vs-code/


In TFT_eSPI/user_setup.h uncomment:
```bash
#define TFT_RGB_ORDER TFT_BGR
#define TFT_INVERSION_ON
```
