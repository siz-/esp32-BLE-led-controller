# esp32-BLE-led-controller

A small script that utilizes the esp32 for controlling chinese aftermarket LED strips that uses the BLE app/LEDLAMP app with hardware buttons.

Dependencies: Arduino IDE with esp32 installed and EasyButton library. BLE Service and characteristic IDs of your device (use fx BlueLight on Android). 

Thanks to kquinsland and arduino12 for their work on deciphering the inner workings of the controllers:

https://github.com/kquinsland/JACKYLED-BLE-RGB-LED-Strip-controller

https://github.com/arduino12/ble_rgb_led_strip_controller
