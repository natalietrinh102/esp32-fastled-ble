/*
  ESP32 FastLED BLE - BLE functionality completed at this fork: https://github.com/natalietrinh102/esp32-fastled-ble
  Copyright (C) 2019 Natalie Trinh

  Orginal work at: 
   ESP32 FastLED BLE: https://github.com/jasoncoon/esp32-fastled-ble
   Copyright (C) 2018 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <FastLED.h>
#include <EEPROM.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001008)
#warning "Requires FastLED 3.1.8 or later; check github for latest code."
#endif


const int led = 5;

// Autoplay is the "demo mode"
uint8_t autoplay = 0; // Set to 0 to disable
uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t currentPatternIndex = 0; // Index number of which pattern is current

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

uint8_t power = 1;
uint8_t brightness = 8;

uint8_t speed = 30;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 50;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 120;

CRGB solidColor = CRGB::Blue;

uint8_t cyclePalettes = 0;
uint8_t paletteDuration = 10;
uint8_t currentPaletteIndex = 0;
unsigned long paletteTimeout = 0;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN    4 // Useful variable, however, not really used in the addLED code below to allow for parallel strips
//#define CLK_PIN   5 // Used only on SPI based strips (APA102 and clones)
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
#define NUM_STRIPS 1
#define NUM_LEDS_PER_STRIP 12
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
CRGB leds[NUM_LEDS];

#define MILLI_AMPS         1000 // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  120

// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0

// Includes some of the FastLED demos
#include "patterns.h"

// Storage for EEPROM
#include "field.h"
#include "fields.h"

// Bluetooth code
#include "ble.h"

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

/** show() for ESP32
    Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/

void FastLEDshowESP32()
{
  if (userTaskHandle == 0) {
    // -- Store the handle of the current task, so that the show task can
    //    notify it when it's done
    userTaskHandle = xTaskGetCurrentTaskHandle();

    // -- Trigger the show task
    xTaskNotifyGive(FastLEDshowTaskHandle);

    // -- Wait to be notified that it's done
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
    ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    userTaskHandle = 0;
  }
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/

void FastLEDshowTask(void *pvParameters)
{
  // -- Run forever...
  for (;;) {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // -- Do the show (synchronously)
    // Not sure why FastLEDshowESP32() isn't used, but there must be a reason...
    FastLED.show();

    // -- Notify the calling task
    xTaskNotifyGive(userTaskHandle);
  }
}

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  //  delay(3000); // 3 second delay for recovery
  Serial.begin(115200);

  loadFieldsFromEEPROM(fields, fieldCount);

  setupBLE();

  // three-wire LEDs (WS2811, WS2812, NeoPixel)
  //  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // four-wire LEDs (APA102, DotStar)
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Parallel output is possible but disabled here
  FastLED.addLeds<LED_TYPE, 4, COLOR_ORDER>(leds, 0, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, 12, COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);


  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);

  // set master brightness control
  FastLED.setBrightness(brightness);

  int core = xPortGetCoreID();
  Serial.print("Main code running on core ");
  Serial.println(core);

  // -- Create the FastLED show task
  xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

  autoPlayTimeout = millis() + (autoplayDuration * 1000);
}

void loop()
{
  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  else {
    // Call the current pattern function once, updating the 'leds' array
    patterns[currentPatternIndex].pattern();

    // do some periodic updates
    EVERY_N_MILLISECONDS(40) {
      // slowly blend the current palette to the next
      nblendPaletteTowardPalette(currentPalette, targetPalette, 8);
      gHue++;  // slowly cycle the "base color" through the rainbow
    }
    
// Autoplay "demo" mode. Not important.
    if (autoplay == 1 && (millis() > autoPlayTimeout)) {
      nextPattern();
      autoPlayTimeout = millis() + (autoplayDuration * 1000);
    }

    if (cyclePalettes == 1 && (millis() > paletteTimeout)) {
      nextPalette();
      paletteTimeout = millis() + (paletteDuration * 1000);
    }
  }

  // send the 'leds' array out to the actual LED strip
  FastLEDshowESP32();

  // FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  currentPatternIndex = (currentPatternIndex + 1) % patternCount;
}

void nextPalette()
{
  currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
  targetPalette = palettes[currentPaletteIndex];
}
