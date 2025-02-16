# BLE_CTRL

解析蓝牙游戏手柄信号，控制我的小玩具

# Get Started

```c
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
```

# 参考

1. [XboxControllerNotificationParser](https://github.com/asukiaaa/arduino-XboxControllerNotificationParser)
