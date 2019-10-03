/*
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

        // Convert the std string to a char for comparsion later
        // ref: https://en.cppreference.com/w/cpp/string/basic_string/copy
        char targetUUID[uuid.length()+1];  
        uuid.copy(targetUUID, sizeof targetUUID+1);
        targetUUID[36]='\0';

        
        Serial.println("*********");
        // This line of code can be removed to just use targetUUID later, but it'll stay here for now for reference
        Serial.print("Received write for characteristic UUID: ");
        for (int i = 0; i < uuid.length(); i++) {
          Serial.print(uuid[i]);
        }
        Serial.println();
        Serial.print("Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();

        Serial.print("Finding set function linked with uuid: ");
        Serial.print(targetUUID);
        Serial.println();

        // interate fields[i].uuid until we find a matching i for uuid
        //Serial.print(fields[0].uuid);
        
        int targetIndex;
        // !! note, loop should use the fields length, but hard coded for now !!
        for (int i = 0; i < 14; i++) {
          if (fields[i].uuid ==targetUUID) {
                Serial.print("Comparing ");
                Serial.print(fields[i].uuid);
                Serial.print(" with ");
                Serial.print(targetUUID);
                Serial.println(); 
            targetIndex = i;
                Serial.print("Index found at: "); 
                Serial.print(targetIndex);
                Serial.println();  
          }
        }

        // !! Trying to figure out how to send values through the "FieldSetter". Maybe see the web server for reference?
        fields[targetIndex].setValue(rxValue); // This doesn't work!
              Serial.print("Sending ");  
              Serial.print(RxValue);
              Serial.print(" to :");  
              Serial.print(fields[targetIndex].name)
        /*
        if (rxValue.find("1") != -1) {
          Serial.println("Turning ON!");
          power = 1;
        }
        else if (rxValue.find("0") != -1) {
          Serial.println("Turning OFF!");
          power = 0;
        }
        */

        
        Serial.println();
        Serial.println("*********");
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
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}
