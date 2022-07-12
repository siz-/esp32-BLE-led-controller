/**
 * A BLE client example that is rich in capabilities. 
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 * Modified by siz- in order to control a aftermarket BLE LED controller
 */

#include "BLEDevice.h"
#include <EasyButton.h>

EasyButton button1(15, 50);
EasyButton button2(13, 50);

// The remote services we wish to connect to.
static BLEUUID serviceUUID("add-advertised-service-id-here"); //Advertised Service ID
static BLEUUID serviceUUID2("add-writeable-service-id-here2"); //Writeable Service ID
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("add-writeable-characteristic-id-here"); //Writeable Char ID

boolean first_start = true;
boolean turnoff = false;
//brightness in hex (decimal: 0-100)
uint8_t bright[6] = {0x05, 0x0A, 0x19, 0x32, 0x4B, 0x64};
//colors in hex (decimal: 0-255) - the last 3 in rgb_r are effects
uint8_t rgb_r[13] = {0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x8a, 0x88, 0x95};
uint8_t rgb_g[10] = {0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x0c, 0x00};
uint8_t rgb_b[10] = {0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x21, 0x00, 0x19}; 
int cyclecount = 0;
int brightcount = 0;
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID2);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID2.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void onPressed1() {
   if (first_start == true) {
      uint8_t bytes[9] = {0x7e, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0xef};
      pRemoteCharacteristic->writeValue(bytes, 9);
      Serial.println("Button 1: TURN ON");
      first_start = false;
   } else {
      if (cyclecount > 9) {
        uint8_t bytes[9] = {0x7e, 0x00, 0x03, rgb_r[cyclecount], 0x03, 0x00, 0x00, 0x00, 0xef};
        pRemoteCharacteristic->writeValue(bytes, 9);                       
        uint8_t bytes2[9] = {0x7e, 0x00, 0x02, 0x58, 0x00, 0x00, 0x00, 0x00, 0xef}; //effect speed
        pRemoteCharacteristic->writeValue(bytes2, 9);
        cyclecount++;    
          if (cyclecount > 12) {
            cyclecount = 0;
          }
        Serial.println("Button 1:  Cycle Effect");                                         
      } else {
        uint8_t bytes[9] = {0x7e, 0x00, 0x05, 0x03, rgb_r[cyclecount], rgb_g[cyclecount], rgb_b[cyclecount], 0x00, 0xef};
        pRemoteCharacteristic->writeValue(bytes, 9);   
        cyclecount++;     
        Serial.println("Button 1:  Cycle Color");
      }    
   }
}

void onPressed2() {
   if (first_start == true) {
      uint8_t bytes[9] = {0x7e, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0xef};
      pRemoteCharacteristic->writeValue(bytes, 9);
      Serial.println("Button 2: TURN ON");
      first_start = false;     
   } else {
      uint8_t bytes[9] = {0x7e, 0x00, 0x01, bright[brightcount], 0x00, 0x00, 0x00, 0x00, 0xef};
      pRemoteCharacteristic->writeValue(bytes, 9);
      Serial.println("Button 2 pressed - Brightness");
      brightcount++;
        if (brightcount > 5) {
          brightcount = 0;
        }
   }
}

void turnOff() {
  uint8_t bytes[9] = {0x7e, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef}; 
  pRemoteCharacteristic->writeValue(bytes, 9);     
  Serial.println("TURN OFF");          
  first_start = true;
}

void(* resetFunc) (void) = 0;

void resetEsp() {
  Serial.println("Resetting.. Goodbye"); 
  digitalWrite(LED_BUILTIN, LOW);
  resetFunc();
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  button1.begin();
  button2.begin();
  button1.onPressed(onPressed1); 
  button2.onPressed(onPressed2); 
  button1.onPressedFor(1500, turnOff); 
  button2.onPressedFor(3000, resetEsp);     
  Serial.begin(115200);
  delay(3000);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.

void loop() {
 button1.read();
 button2.read();
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      digitalWrite(LED_BUILTIN, LOW);
    }
    doConnect = false;
  }
  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  } 
} 