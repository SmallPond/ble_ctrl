#include "ble_ctrl_parser.h"


#define BLE_CONTROLLER_INDEX_BUTTONS_DIR 4
#define BLE_CONTROLLER_INDEX_BUTTONS_MAIN 5
#define BLE_CONTROLLER_INDEX_BUTTONS_CENTER 6
#define BLE_CONTROLLER_INDEX_BUTTONS_SHARE 15


BLEControllerNotificationParser::BLEControllerNotificationParser()
{
  btnA = btnB = btnX = btnY = false;
  btnShare = btnStart = btnSelect = btnXbox = false;
  btnLB = btnRB = btnLS = btnRS = false;
  btnDirUp = btnDirLeft = btnDirRight = btnDirDown = false;
  joyLHori = joyLVert = joyRHori = joyRVert = 0xffff / 2;
  trigLT = trigRT = 0;
}

uint8_t BLEControllerNotificationParser::update(uint8_t *data, size_t length)
{
    if (length != 16) {
        return BLE_CONTROLLER_ERROR_INVALID_LENGTH;
    }
    uint8_t btnBits;

    joyLHori = data[0];
    joyLVert = data[1];
    joyRHori = data[2];
    joyRVert = data[3];
    trigLT = data[7];
    trigRT = data[8];

    btnBits = data[4];
    btnDirUp = btnBits == 1 || btnBits == 2 || btnBits == 8;
    btnDirRight = 2 <= btnBits && btnBits <= 4;
    btnDirDown = 4 <= btnBits && btnBits <= 6;
    btnDirLeft = 6 <= btnBits && btnBits <= 8;
    btnA = btnBits & (1 << 4);
    btnB = btnBits & (1 << 5);
    btnX = btnBits & (1 << 6);
    btnY = btnBits & (1 << 7);

    btnBits = data[5];
    
    btnLB = btnBits & (1 << 0);
    btnRB = btnBits & (1 << 1);

    // btnBits = data[BLE_CONTROLLER_INDEX_BUTTONS_CENTER];
    // btnSelect = btnBits & 0b00000100;
    // btnStart = btnBits & 0b00001000;
    // btnXbox = btnBits & 0b00010000;
    // btnLS = btnBits & 0b00100000;
    // btnRS = btnBits & 0b01000000;
    
    // printStatus();
    return 0;
}

void BLEControllerNotificationParser::printStatus()
{
     Serial.printf("btnY: %d, btnX: %d, btnB: %d, btnA: %d, btnLB: %d, btnRB: %d\n"
                 "joyLHori: %d\n joyLVert: %d\n joyRHori: %d\n joyRVert: %d\n"
                 "trigLT: %d\n trigRT: %d\n",
                btnY,
                btnX,
                btnB,
                btnA,
                btnLB,
                btnRB,
                joyLHori,
                joyLVert,
                joyRHori,
                joyRVert,
                trigLT,
                trigRT);
}