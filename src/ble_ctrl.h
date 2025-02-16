#ifndef __BLE_CTRL_H__
#define __BLE_CTRL_H__


#include <iostream>
#include <string>
#include <cstring>
#include <NimBLEDevice.h>
#include "ble_ctrl_parser.h"

#define BLE_ADDRESS "04:0A:11:11:90:10"
#define SERVICE_UUID "91680001-1111-6666-8888-0123456789AB"



class BLECtrl{
public:
    BLECtrl() {}
    ~BLECtrl() {}

    void setup() {
        setup(BLE_ADDRESS);
    }

    void setup(const char* ble_addr);

    void loop(void);

    BLEControllerNotificationParser *get_status(void);
    void print_status(void);
private:
    const char *_ble_address;
};


#endif 