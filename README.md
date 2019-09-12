# ESP32 FastLED BLE (Bluetooth Low Energy)
**Work in progress BLE port of https://github.com/jasoncoon/esp32-fastled-webserver**

This fork aims to document and improve functionality of the esp-fastled-ble project. 

## Features
### Currently Working:
* [x] DemoReel100 patterns
* [x] [Sam's multi-core support](https://github.com/samguyer/FastLED/blob/master/examples/DemoReelESP32/DemoReelESP32.ino)
* [x] Setting storage in EEPROM

### Currently Lacking:
* [x] Ability to adjust these settings via BLE:
   * [ ] power on/off
   * [ ] brightness
   * [ ] pattern
   * [ ] autoplay on/off
   * [ ] autoplay duration
   * [ ] speed
   * [ ] palette
   * [ ] auto palette cycling
   * [ ] palette duration
   * [ ] solid color
   * [ ] twinkle speed/density
   * [ ] fire cooling/sparking

### Currently working on:
* The current code will take BLE commands, but processes all writes the same regardless of the characteristic it's assigned to. Therefore, we must add logic that takes the current uuid variable value, and push/map it to an existing variable in the code. There is already an existing map as evidenced by field.getValue - we just need to "reverse" this.

## Requirements

### Hardware

#### ESP32 Development Board

[An ESP32 development board of your choice](https://www.google.com/search?q=esp32+development+board)

The code here is debugged on an ESP32-PICO kit.

#### Addressable LED strip

The code may be easily modified to run any configuration supported by FastLED; however, it will be debugged on a single WS2812B strip.

#### Other hardware:

* [3.3V to 5V Logic Level Shifter](http://www.digikey.com/product-detail/en/texas-instruments/SN74HCT245N/296-1612-5-ND/277258) (required if LEDs "glitch")
* [Octo Level Shifter FeatherWing](https://www.evilgeniuslabs.org/level-shifter-featherwing) (tidy level shifter PCB)

Recommended by [Adafruit NeoPixel "Best Practices"](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices) to help protect LEDs from current onrush:
* [1000µF Capacitor](http://www.digikey.com/product-detail/en/panasonic-electronic-components/ECA-1EM102/P5156-ND/245015)
* [300 to 500 Ohm resistor](https://www.digikey.com/product-detail/en/stackpole-electronics-inc/CF14JT470R/CF14JT470RCT-ND/1830342)

### Software

* [Arduino](https://www.arduino.cc/en/main/software)
* [ESP32 Arduino Libraries & Tools](https://github.com/espressif/arduino-esp32)
* [Arduino ESP32 filesystem uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin)

#### Libraries

* [samguyer/FastLED](https://github.com/samguyer/FastLED)
* [ESP32 Arduino Libraries & Tools](https://github.com/espressif/arduino-esp32)
