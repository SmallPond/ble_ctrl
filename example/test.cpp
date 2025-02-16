#include <Arduino.h>
#include "ble_ctrl.h"

BLECtrl ble_ctrl;

void setup() {
    Serial.begin(115200);
    ble_ctrl.setup();
}

void loop() {
    ble_ctrl.loop();
}