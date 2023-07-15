
/** NimBLE_Server Demo:
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 *
*/

#include <NimBLEDevice.h>

#include "driver/uart.h"

#define RX1_BUF_SIZE 		(256)
#define TX1_BUF_SIZE 		(256)
#define TXD1_PIN 			(GPIO_NUM_0)
#define RXD1_PIN 			(GPIO_NUM_1)

uint8_t ShouBingData[32];

void uart1_init(void){
	//串口配置结构体
	uart_config_t  uart1_config;
	uart1_config.baud_rate = 115200*8;					//波特率
	uart1_config.data_bits = UART_DATA_8_BITS;			//数据位
	uart1_config.parity = UART_PARITY_DISABLE;			//校验位
	uart1_config.stop_bits = UART_STOP_BITS_1;			//停止位
	uart1_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;	//硬件流控
	uart_param_config(UART_NUM_1, &uart1_config);		//设置串口
	//IO映射-> T:IO12  R:IO13
	uart_set_pin(UART_NUM_1, TXD1_PIN, RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(UART_NUM_1, RX1_BUF_SIZE * 2, TX1_BUF_SIZE * 2, 0, NULL, 0);
}

void scanEndedCB(NimBLEScanResults results);

static NimBLEAdvertisedDevice* advDevice;

static bool doConnect = false;
static uint32_t scanTime = 0; /** 0 = scan forever */
bool scanning = false, connected = false;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println("Connected");
        connected = true;
        /** After connection we should change the parameters if we don't need fast response times.
         *  These settings are 150ms interval, 0 latency, 450ms timout.
         *  Timeout should be a multiple of the interval, minimum is 100ms.
         *  I find a multiple of 3-5 * the interval works best for quick response/reconnect.
         *  Min interval: 120 * 1.25ms = 150, Max interval: 120 * 1.25ms = 150, 0 latency, 60 * 10ms = 600ms timeout
         */
        // pClient->updateConnParams(120,120,0,60);
    };

    void onDisconnect(NimBLEClient* pClient) {
      connected = false;
        // Serial.print(pClient->getPeerAddress().toString().c_str());
        // Serial.println(" Disconnected - Starting scan");
        // NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
    };

    /** Called when the peripheral requests a change to the connection parameters.
     *  Return true to accept and apply them or false to reject and keep
     *  the currently used parameters. Default will return true.
     */
    // bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params) {
    //     if(params->itvl_min < 24) { /** 1.25ms units */
    //         return false;
    //     } else if(params->itvl_max > 40) { /** 1.25ms units */
    //         return false;
    //     } else if(params->latency > 2) { /** Number of intervals allowed to skip */
    //         return false;
    //     } else if(params->supervision_timeout > 100) { /** 10ms units */
    //         return false;
    //     }

    //     return true;
    // };

    // /********************* Security handled here **********************
    // ****** Note: these are the same return values as defaults ********/
    // uint32_t onPassKeyRequest(){
    //     Serial.println("Client Passkey Request");
    //     /** return the passkey to send to the server */
    //     return 123456;
    // };

    // bool onConfirmPIN(uint32_t pass_key){
    //     Serial.print("The passkey YES/NO number: ");
    //     Serial.println(pass_key);
    // /** Return false if passkeys don't match. */
    //     return true;
    // };

    // /** Pairing process complete, we can check the results in ble_gap_conn_desc */
    // void onAuthenticationComplete(ble_gap_conn_desc* desc){
    //     if(!desc->sec_state.encrypted) {
    //         Serial.println("Encrypt connection failed - disconnecting");
    //         /** Find the client with the connection handle provided in desc */
    //         NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
    //         return;
    //     }
    // };
};


NimBLEAddress Address("04:03:23:68:8b:b0"); // 蓝牙手柄地址
NimBLEUUID ServiceUUID("91680001-1111-6666-8888-0123456789AB"); // 蓝牙手柄有数据输出的服务UUID

/* 定义一个类来处理接收到广告时的回调 */
class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        // Serial.print("Advertised Device found: ");
        // Serial.println(advertisedDevice->toString().c_str());
        if(advertisedDevice->getAddress() == Address)
        {
          NimBLEDevice::getScan()->stop();
            /** Save the device reference in a global for the client to use*/
            advDevice = advertisedDevice;
            /** Ready to connect now */
            doConnect = true;
        }
        // if(advertisedDevice->isAdvertisingService(NimBLEUUID("DEAD")))
        // {
        //     // Serial.println("Found Our Service");
        //     /** stop scan before connecting */
        //     NimBLEDevice::getScan()->stop();
        //     /** Save the device reference in a global for the client to use*/
        //     advDevice = advertisedDevice;
        //     /** Ready to connect now */
        //     doConnect = true;
        // }

    };
};


/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
    // std::string str = (isNotify == true) ? "Notification" : "Indication";
    // str += " from ";
    // /** NimBLEAddress and NimBLEUUID have std::string operators */
    // str += std::string(pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress());
    // str += ": Service = " + std::string(pRemoteCharacteristic->getRemoteService()->getUUID());
    // str += ", Characteristic = " + std::string(pRemoteCharacteristic->getUUID());
    // str += ", Value = " + std::string((char*)pData, length);
    // Serial.println(str.c_str());

    uart_write_bytes(UART_NUM_1, (char *)pData, length);

    // static bool isPrinting = false;
    // static unsigned long printedAt = 0;
    // if(isPrinting || millis() - printedAt < 100)
    //   return;
    // isPrinting = true;
    // for(int i = 3; i < 12; ++i)
    // {
    //   Serial.printf("%d/",i);
    //   Serial.printf("%03d/",pData[i]);
    // }
    // Serial.println(" ");
    // printedAt = millis();
    // isPrinting = false;
}

/** 回调处理最后一次扫描的结果或重新启动 */
void scanEndedCB(NimBLEScanResults results){
    scanning = false;
    // Serial.println("Scan Ended");
}


/** 创建一个回调类的全局实例供所有客户端使用 */
static ClientCallbacks clientCB;

void charaSubscribeNotification(NimBLERemoteCharacteristic *pChara) // 订阅通知函数
{
  if(pChara->canNotify())
  {
    Serial.println("canNotify");
    if(pChara->subscribe(true,notifyCB,true)) // 订阅通知
    {
      Serial.println("set NotifyCb");
    }
  }
}

bool afterConnect(NimBLEClient *pClient)
{
  for(auto pService: *pClient->getServices(true))
  {
    auto sUuid = pService->getUUID();
    if(!sUuid.equals(ServiceUUID))
    {
      continue;
    }
    for(auto pChara: *pService->getCharacteristics(true))
    {
      charaSubscribeNotification(pChara);
    }
  }
  return true;
}

/** 处理客户端供应和连接/接口与服务器 */
bool connectToServer(NimBLEAdvertisedDevice *advDevice) {
    NimBLEClient* pClient = nullptr;

    /** 检查我们是否有一个客户端，我们应该首先重用 **/
    if(NimBLEDevice::getClientListSize()) 
    {
        /** 特殊情况下，当我们已经知道这个设备，我们发送false作为
         *  connect()中的第二个参数，防止刷新服务数据库
         *  这节省了大量的时间和精力
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if(pClient){
          pClient->connect();
            // if(!pClient->connect(advDevice, false)) {
            //     Serial.println("Reconnect failed"); // 连接失败了
            //     return false;
            }
            // Serial.println("Reconnected client"); // 连接客户端
    }
        /** We don't already have a client that knows this device,
         *  we will check for a client that is disconnected that we can use.
         */
    //     else {
    //         pClient = NimBLEDevice::getDisconnectedClient();
    //     }
    // }

    /** 没有客户端可重用?创建一个新的 */
    if(!pClient) {
          pClient = NimBLEDevice::createClient();
          pClient->setClientCallbacks(&clientCB,false);
          pClient->setConnectTimeout(5);
          pClient->connect(advDevice,false);
        // if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
        //     Serial.println("Max clients reached - no more connections available");
        //     return false;
        }

    //     pClient = NimBLEDevice::createClient();

    //     Serial.println("New client created"); // 创建新客户端

    //     pClient->setClientCallbacks(&clientCB, false);
    //     /** 设置初始连接参数:这些设置为15ms间隔，0延迟，120ms超时
    //      *  这些设置是安全的3个客户端连接可靠，可以走得更快，如果你有更少
    //      *  连接。超时时间应该是间隔的倍数，最小为100ms
    //      *  最小时间间隔:12 * 1.25ms = 15，最大时间间隔:12 * 1.25ms = 15, 0延迟，51 * 10ms = 510ms超时
    //      */
    //     pClient->setConnectionParams(12,12,0,51);
    //     /** 设置我们愿意等待连接完成的时间(秒)，默认为30. */
    //     pClient->setConnectTimeout(5);


    //     if (!pClient->connect(advDevice)) {
    //         /** 创建了一个客户端，但连接失败，不需要保留它，因为它没有数据 */
    //         NimBLEDevice::deleteClient(pClient);
    //         Serial.println("Failed to connect, deleted client");
    //         return false;
    //     }
    // }

    // if(!pClient->isConnected()) {
    //     if (!pClient->connect(advDevice)) {
    //         Serial.println("Failed to connect");
    //         return false;
    //     }
    // }

    // Serial.print("Connected to: ");
    // Serial.println(pClient->getPeerAddress().toString().c_str());
    // Serial.print("RSSI: ");
    // Serial.println(pClient->getRssi());

    // /** 现在我们可以读/写/订阅我们感兴趣的服务的特征 */
    // NimBLERemoteService* pSvc = nullptr;
    // NimBLERemoteCharacteristic* pChr = nullptr;
    // NimBLERemoteDescriptor* pDsc = nullptr;

    // pSvc = pClient->getService("DEAD");
    // if(pSvc) {     /** make sure it's not null */
    //     pChr = pSvc->getCharacteristic("BEEF");

    //     if(pChr) {     /** make sure it's not null */
    //         if(pChr->canRead()) {
    //             Serial.print(pChr->getUUID().toString().c_str());
    //             Serial.print(" Value: ");
    //             Serial.println(pChr->readValue().c_str());
    //         }

    //         if(pChr->canWrite()) {
    //             if(pChr->writeValue("Tasty")) {
    //                 Serial.print("Wrote new value to: ");
    //                 Serial.println(pChr->getUUID().toString().c_str());
    //             }
    //             else {
    //                 /** Disconnect if write failed */
    //                 pClient->disconnect();
    //                 return false;
    //             }

    //             if(pChr->canRead()) {
    //                 Serial.print("The value of: ");
    //                 Serial.print(pChr->getUUID().toString().c_str());
    //                 Serial.print(" is now: ");
    //                 Serial.println(pChr->readValue().c_str());
    //             }
    //         }

    //         /** registerForNotify() has been deprecated and replaced with subscribe() / unsubscribe().
    //          *  Subscribe parameter defaults are: notifications=true, notifyCallback=nullptr, response=false.
    //          *  Unsubscribe parameter defaults are: response=false.
    //          */
    //         if(pChr->canNotify()) {
    //             //if(!pChr->registerForNotify(notifyCB)) {
    //             if(!pChr->subscribe(true, notifyCB)) {
    //                 /** Disconnect if subscribe failed */
    //                 pClient->disconnect();
    //                 return false;
    //             }
    //         }
    //         else if(pChr->canIndicate()) {
    //             /** Send false as first argument to subscribe to indications instead of notifications */
    //             //if(!pChr->registerForNotify(notifyCB, false)) {
    //             if(!pChr->subscribe(false, notifyCB)) {
    //                 /** Disconnect if subscribe failed */
    //                 pClient->disconnect();
    //                 return false;
    //             }
    //         }
    //     }

    // } else {
    //     Serial.println("DEAD service not found.");
    // }

    // pSvc = pClient->getService("BAAD");
    // if(pSvc) {     /** make sure it's not null */
    //     pChr = pSvc->getCharacteristic("F00D");

    //     if(pChr) {     /** make sure it's not null */
    //         if(pChr->canRead()) {
    //             Serial.print(pChr->getUUID().toString().c_str());
    //             Serial.print(" Value: ");
    //             Serial.println(pChr->readValue().c_str());
    //         }

    //         pDsc = pChr->getDescriptor(NimBLEUUID("C01D"));
    //         if(pDsc) {   /** make sure it's not null */
    //             Serial.print("Descriptor: ");
    //             Serial.print(pDsc->getUUID().toString().c_str());
    //             Serial.print(" Value: ");
    //             Serial.println(pDsc->readValue().c_str());
    //         }

    //         if(pChr->canWrite()) {
    //             if(pChr->writeValue("No tip!")) {
    //                 Serial.print("Wrote new value to: ");
    //                 Serial.println(pChr->getUUID().toString().c_str());
    //             }
    //             else {
    //                 /** Disconnect if write failed */
    //                 pClient->disconnect();
    //                 return false;
    //             }

    //             if(pChr->canRead()) {
    //                 Serial.print("The value of: ");
    //                 Serial.print(pChr->getUUID().toString().c_str());
    //                 Serial.print(" is now: ");
    //                 Serial.println(pChr->readValue().c_str());
    //             }
    //         }

    //         /** registerForNotify() has been deprecated and replaced with subscribe() / unsubscribe().
    //          *  Subscribe parameter defaults are: notifications=true, notifyCallback=nullptr, response=false.
    //          *  Unsubscribe parameter defaults are: response=false.
    //          */
    //         if(pChr->canNotify()) {
    //             //if(!pChr->registerForNotify(notifyCB)) {
    //             if(!pChr->subscribe(true, notifyCB)) {
    //                 /** Disconnect if subscribe failed */
    //                 pClient->disconnect();
    //                 return false;
    //             }
    //         }
    //         else if(pChr->canIndicate()) {
    //             /** Send false as first argument to subscribe to indications instead of notifications */
    //             //if(!pChr->registerForNotify(notifyCB, false)) {
    //             if(!pChr->subscribe(false, notifyCB)) {
    //                 /** Disconnect if subscribe failed */
    //                 pClient->disconnect();
    //                 return false;
    //             }
    //         }
    //     }

    // } else {
    //     Serial.println("BAAD service not found.");
    // }

    // Serial.println("Done with this device!");
    // return true;
    bool result = afterConnect(pClient);
    if(!result)
    {
      return result;
    }
    return true;
}

void setup (){
    Serial.begin(115200);
    Serial.println("Starting NimBLE Client");
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

    /** 可选:设置不希望接收广告的设备 */
    // NimBLEDevice::addIgnored(NimBLEAddress ("aa:bb:cc:dd:ee:ff"));

    /** 创建新扫描 */
    NimBLEScan* pScan = NimBLEDevice::getScan();

    /** 创建一个回调函数，当找到广告客户时调用它 */
    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());

    /** 设置扫描间隔(多长时间)和窗口(多长时间)以毫秒为单位 */
    pScan->setInterval(45);
    pScan->setWindow(15);

    /** 主动扫描将收集扫描响应数据从广告商但两种设备都会消耗更多的能量*/
    pScan->setActiveScan(true);
    /** Start scanning for advertisers for the scan time specified (in seconds) 0 = forever
     *  扫描停止时的可选回调.
     */
    pScan->start(scanTime, scanEndedCB);
    uart1_init();
}

void loop (){
    /** 循环这里，直到我们找到一个我们想要连接的设备 */
    while(!doConnect){
        delay(1);
    }

    // doConnect = false;
      // if(connectToServer()) {
      //     // Serial.println("Success! we should now be getting notifications, scanning for more!");
      // } else {
      //     // Serial.println("Failed to connect, starting scan");
      // } 

      /** 找到我们想要连接的设备，现在就连接 */
     
      // NimBLEDevice::getScan()->start(scanTime,scanEndedCB);
      if(!connected)
      {
        if(advDevice != nullptr)
        {
          if(connectToServer(advDevice))
          {
            
          }
          advDevice = nullptr;
        }
        else if(!scanning)
        {
          NimBLEDevice::getScan()->start(scanTime,scanEndedCB);
        }
      }
}
