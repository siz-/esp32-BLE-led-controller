A script that utilizes the esp32 for controlling chinese LED aRGB controllers that use the BLE/LEDLAMP app. 
Code has been edited and optimized for 3 rotary encoders:

Rotary Encoder 1: aRGB strip control (single press to change mode)
  - Mode 1: Custom animation and color change (23)
  - Mode 2: Preinstalled animations (209)
  - Long press: shutdown in mode 1, reset animation in mode 2
    
Rotary Encoder 2: RGB strip control
  - Mode 1: color change (10)
  - Mode 2 global color change (10)
  - Long press: shutdown. Goes to sleep for 10 seconds if encoder 1 = mode 2 and encoder 2 = mode 2 and encoder 3 =  mode 3, so user can connect to controller via phone
    
Rotary Encoder 3: brightness control
  - Mode 1: Global control
  - Mode 2: aRGB control
  - Mode 3: RGB control
  - Long press: shutdown in mode 1. In mode 2 and 3, each zone shuts off independently of each other.

The script saves state. Now jumps between colors when switching, it resets LEDs at startup to avoid garbage data and more.

Dependencies: Arduino IDE with esp32 installed, EasyButton, NimBLE and ESP32 Encoder library. BLE device address, BLE Service and characteristic IDs of your device (use BlueLight on Android).

Inspiration:
https://github.com/kquinsland/JACKYLED-BLE-RGB-LED-Strip-controller
https://github.com/arduino12/ble_rgb_led_strip_controller
