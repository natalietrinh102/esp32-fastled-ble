/*
  ESP32 FastLED BLE - BLE functionality completed at this fork: https://github.com/natalietrinh102/esp32-fastled-ble
  Copyright (C) 2019 Natalie Trinh

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

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "9a9c83cc-845e-4883-96c3-1f021b99c34c"


class FieldCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        std::string uuid = pCharacteristic->getUUID().toString();

        //Convert the C++11 strings to C-strings
        String targetUUID = uuid.c_str();
        String targetValue = rxValue.c_str();
        
        // This line of code can be removed to just use targetUUID later, but it'll stay here for now for reference
        Serial.print("Received write for characteristic UUID: ");
        for (int i = 0; i < uuid.length(); i++) {
          Serial.print(uuid[i]);
        }

        Serial.println();
        
        // interate fields[i].uuid until we find a matching i for uuid
        int targetIndex;
        // !! note, loop should use the fields length, but hard coded for now !!
        for (int i = 0; i < 14; i++) {
          if (fields[i].uuid ==targetUUID) {
            targetIndex = i; 
          }
        }
        
        String name = fields[targetIndex].name;
        
// Send the recieved values to matching setter function
        setFieldValue(name, targetValue, fields, fieldCount);
              Serial.print("Setting ");  
              Serial.print(name);
              Serial.print(" to: ");
              Serial.println(targetValue);
      }
    }
};

void setupBLE() {
  Serial.println("Starting BLE...");

  BLEDevice::init("LEDCore mk1");

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  FieldCallbacks *fieldCallbacks = new FieldCallbacks();

  for (uint8_t i = 0; i < fieldCount; i++) {
    Field field = fields[i];

    char uuid[field.uuid.length() + 1];
    memset(uuid, 0, field.uuid.length() + 1);

    for (uint8_t i = 0; i < field.uuid.length(); i++)
      uuid[i] = field.uuid.charAt(i);

    String value = field.getValue();

    Serial.print("Adding characteristic UUID: ");
    Serial.print(uuid);
    Serial.print(", name: ");
    Serial.print(field.name);
    Serial.print(", value: ");
    Serial.println(value);

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                           uuid,
                                           BLECharacteristic::PROPERTY_READ |
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
    pCharacteristic->setCallbacks(fieldCallbacks);

    pCharacteristic->setValue(value.c_str());
  }

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("Characteristics defined!");
}
