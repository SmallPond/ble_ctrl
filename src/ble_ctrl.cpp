
/** NimBLE_Client Demo:
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 *
*/
#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ble_ctrl.h"
#include "ble_ctrl_parser.h"
void scanEndedCB(NimBLEScanResults results);

static NimBLEAdvertisedDevice* advDevice;

NimBLEUUID ServiceUUID(SERVICE_UUID); // 蓝牙手柄有数据输出的服务UUID
BLEControllerNotificationParser bleParser;

static bool doConnect = false;
static uint32_t scanTime = 0; /** 0 = scan forever */
bool scanning = false, connected = false;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println("Connected");
        connected = true;
    };

    void onDisconnect(NimBLEClient* pClient) {
        Serial.print(pClient->getPeerAddress().toString().c_str());
        connected = false;
    };

};


/** Define a class to handle the callbacks when advertisments are received */
class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
public:
    AdvertisedDeviceCallbacks(const char *ble_str) {
        _addr = new NimBLEAddress(ble_str);
    }

    ~AdvertisedDeviceCallbacks() {
        delete _addr;
    }

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.print("Advertised Device found: ");
        Serial.println(advertisedDevice->toString().c_str());
        if(advertisedDevice->getAddress() == *_addr || 
            advertisedDevice->getName().find("BM769") != std::string::npos) {
            log_i("Found Our Service Name %s", advertisedDevice->getName().c_str());
            /** stop scan before connecting */
            NimBLEDevice::getScan()->stop();
            /** Save the device reference in a global for the client to use*/
            advDevice = advertisedDevice;
            /** Ready to connect now */
            doConnect = true;
        }
    };
private:
    NimBLEAddress *_addr;
};

std::string HexToStr(const std::string& str)
{
    std::string result;
    for (size_t i = 0; i < str.length(); i += 2)
    {
        std::string byte = str.substr(i, 2);
        char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
        result.push_back(chr);
    }
    return result;
}

/** Notification / Indication receiving handler callback */
/*
 * length = 
*/
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){

    // Serial.printf("length: %d, Value = 0x", length);
    // for(int i = 0; i < length; i++) {
    //     Serial.printf("%02x", pData[i]);
    //     if(i % 2 == 1) {
    //         Serial.printf("_");
    //     } 
    // }
    // Serial.println();

    bleParser.update(pData+3, BLE_CONTROLLER_DATA_LEN);
    // bleParser.printStatus();
    
}

/** Callback to process the results of the last scan or restart it */
void scanEndedCB(NimBLEScanResults results){
    Serial.println("Scan Ended");
    scanning = false;
}


/** Create a single global instance of the callback class to be used by all clients */
static ClientCallbacks clientCB;
/*
 * 订阅通知函数
*/
void charaSubscribeNotification(NimBLERemoteCharacteristic *pChara) 
{
    if(pChara->canNotify()) {
        if(pChara->subscribe(true, notifyCB, true)) {
            Serial.println("set NotifyCb");
        }
    }
}

bool afterConnect(NimBLEClient *pClient)
{
  for(auto pService: *pClient->getServices(true)){
      auto sUuid = pService->getUUID();
      if(!sUuid.equals(ServiceUUID)) {
          continue;
      }
      for(auto pChara: *pService->getCharacteristics(true)) {
          charaSubscribeNotification(pChara);
      }
  }
  return true;
}

/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer(NimBLEAdvertisedDevice *advDevice) {
    NimBLEClient* pClient = nullptr;

    /** Check if we have a client we should reuse first **/
    if(NimBLEDevice::getClientListSize()) {
        /** Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if(pClient){
            if(!pClient->connect(advDevice, false)) {
                Serial.println("Reconnect failed");
                return false;
            }
            Serial.println("Reconnected client");
        }
        /** We don't already have a client that knows this device,
         *  we will check for a client that is disconnected that we can use.
         */
        else {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if(!pClient) {
        if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
            Serial.println("Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        Serial.println("New client created");

        pClient->setClientCallbacks(&clientCB, false);
        /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
         *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
         *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
         *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
         */
        pClient->setConnectionParams(12,12,0,51);
        /** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
        pClient->setConnectTimeout(5);


        if (!pClient->connect(advDevice)) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            Serial.println("Failed to connect, deleted client");
            return false;
        }
    }

    if(!pClient->isConnected()) {
        if (!pClient->connect(advDevice)) {
            Serial.println("Failed to connect");
            return false;
        }
    }

    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());

    return afterConnect(pClient);
}

void BLECtrl::setup(const char* ble_addr)
{
    // Serial.begin(115200);
    Serial.println("Starting NimBLE Client");
    _ble_address = strdup(ble_addr);
    /** Initialize NimBLE, no device name spcified as we are not advertising */
    NimBLEDevice::init("");

    /** Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_KEYBOARD_ONLY    - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY); // use passkey
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /** 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, secure connections.
     *
     *  These are the default values, only shown here for demonstration.
     */
    //NimBLEDevice::setSecurityAuth(false, false, true);
    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    /** Optional: set the transmit power, default is 3db */
#ifdef ESP_PLATFORM
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
#else
    NimBLEDevice::setPower(9); /** +9db */
#endif

    /** Optional: set any devices you don't want to get advertisments from */
    // NimBLEDevice::addIgnored(NimBLEAddress ("aa:bb:cc:dd:ee:ff"));

    /** create new scan */
    NimBLEScan* pScan = NimBLEDevice::getScan();

    /** create a callback that gets called when advertisers are found */
    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks(ble_addr));

    /** Set scan interval (how often) and window (how long) in milliseconds */
    pScan->setInterval(45);
    pScan->setWindow(15);

    /** Active scan will gather scan response data from advertisers
     *  but will use more energy from both devices
     */
    pScan->setActiveScan(true);
    /** Start scanning for advertisers for the scan time specified (in seconds) 0 = forever
     *  Optional callback for when scanning stops.
     */
    pScan->start(scanTime, scanEndedCB);
}


void BLECtrl::loop(void)
{
    if (!doConnect){
        // Must call Delay
        vTaskDelay(1 / portTICK_PERIOD_MS);
        return;
    }

    doConnect = false;

    /** Found a device we want to connect to, do it now */
    if(!connected) {
        if (advDevice != NULL) {
            if(connectToServer(advDevice)) {
                log_i("Success! we should now be getting notifications!");
            }
            advDevice = nullptr;
        }
    } else if (!scanning) {
        NimBLEDevice::getScan()->start(scanTime,scanEndedCB);
    }
}

BLEControllerNotificationParser* BLECtrl::get_status(void)
{
    return &bleParser;
}

void BLECtrl::print_status(void)
{
    bleParser.printStatus();
}
