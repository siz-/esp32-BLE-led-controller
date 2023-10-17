#include <NimBLEDevice.h>
#include <EasyButton.h>
#include <Preferences.h>
/**************************************************************/
/********************** NimBLE Client *************************/
/**************************************************************/
static NimBLEAddress address("xx:xx:xx:xx:xx:xx"); //Device address
static NimBLEUUID serviceUUID("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"); //Writeable Service ID
static NimBLEUUID charUUID("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"); //Writeable Char ID
static NimBLERemoteCharacteristic* pRemoteCharacteristic;
static NimBLERemoteService* pRemoteService;
static NimBLEClient* pClient;
boolean pConnect = false;
boolean firstConnect = true;
Preferences preferences;
/**************************************************************/
/*************************     LED    *************************/
/**************************************************************/
/*The following are the sniffed BLE commands that can be used:
RGB LED:
  rgb: 7b 01 07 Rx Gx Bx ff ff bf
  brightness: 7b ff 01 00 xx 02 ff ff bf
  on: 7b ff 04 07 ff ff ff ff bf
  off: 7b ff 04 06 ff ff ff ff bf 
  identify controller: 7e ff 12 xx ff ff ff ff ef //from 02 etc.
  Number of leds: 7b ff 05 04 00 xx 03 ff bf
BLE:
  rgb: 7e ff 05 03 Rx Gx Bx ff ef
  brightness: 7e ff 01 xx 00 ff ff ff ef
  mode_speed: 7e ff 02 xx ff ff ff ff ef
  set_mode:   7e ff 03 xx 03 ff ff ff ef
RGB DMX:
  rgb: 7b 00 07 Rx Gx Bx ff ff bf
  brightness: 7b 00 01 00 xx 00 ff ff bf
  mode_speed: 7b ff 02 xx 00 ff ff ff bf
  set_mode: 7b ff 03 xx ff ff ff ff bf (210 modes - from 01 to d2)
  welcome animation: 7e ff 12 00 ff ff ff ff ef //toggle on welcome animation
  cancel welcome animation: 7e ff 12 01 ff ff ff ff ef

-- CUSTOM DMX mode/animation --
7b 0y 0e fd Rx Gx Bx 0z bf // forward - fd can be changed to backward etc., not had the interest to sniff out other modes

y: color number - starts from 1
z: total number of colors
standard effect speed: 7bff023000ffffffbf */

EasyButton button1(15, 50);
EasyButton button2(13, 50);

int turnoff;
int color_1;
int color_2;
int gbright;
int effect_2;
int toggle;

void gradientFunction(uint8_t rgb[][3], int length) {
  for (int i = 0; i < length; i++) {                                                
    uint8_t bytes[9] = {0x7b, 0x01 + i, 0x0e, 0xfd, rgb[i][0], rgb[i][1], rgb[i][2], length, 0xbf}; 
    pRemoteCharacteristic->writeValue(bytes, 9);
    delay(100);    
  }
}

void gradientColors(int d) {
  if (d == 0) {
    uint8_t rgb[4][3] = { {0xff, 0x00, 0x00}, {0xff, 0x11, 0x11}, {0xea, 0x1f, 0x10}, {0xff, 0x00, 0x00} };            
    gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Red gradient");
  } else if (d == 1) { 
    uint8_t rgb[4][3] = { {0x00, 0xff, 0x00}, {0x1a, 0xff, 0x1a}, {0x38, 0xfc, 0x38}, {0x00, 0xff, 0x00} };
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Green gradient");
  } else if (d == 2) { 
    uint8_t rgb[4][3] = { {0x00, 0x00, 0xff}, {0x2b, 0x5e, 0xf6}, {0x58, 0x85, 0xf7}, {0x00, 0x00, 0xff} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Blue gradient");
  } else if (d == 3) {   
    uint8_t rgb[3][3] = { {0xff, 0xff, 0x00}, {0xf3, 0xa2, 0x20}, {0xff, 0xff, 0x20} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Yellow gradient");
  } else if (d == 4) { 
    uint8_t rgb[3][3] = { {0xff, 0x00, 0xff}, {0x7c, 0x18, 0xf5}, {0xd1, 0x4c, 0xf7} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Purple gradient");
  } else if (d == 5) { 
    uint8_t rgb[3][3] = { {0x00, 0xff, 0xff}, {0x1, 0x5f, 0xfa}, {0x45, 0xff, 0xff} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Lightblue gradient");
  } else if (d == 6) { 
    uint8_t rgb[3][3] = { {0x00, 0xff, 0x37}, {0x75, 0xfb, 0x89}, {0x00, 0xff, 0x21} };
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Lightgreen gradient");
  } else if (d == 7) { 
    uint8_t rgb[4][3] = { {0xff, 0x0c, 0x00}, {0xff, 0x26, 0x00}, {0xff, 0x3a, 0x00}, {0xff, 0x0c, 0x00} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Amber gradient");
  } else if (d == 8) { 
    uint8_t rgb[4][3] = { {0x00, 0xff, 0xff}, {0x00, 0xff, 0xff}, {0x00, 0xff, 0x52}, {0x00, 0xff, 0x52} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Lightblue/Cyan");
  } else if (d == 9) { 
    uint8_t rgb[6][3] = { {0x00, 0xff, 0xff}, {0x00, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}, {0x00, 0xff, 0x52}, {0x00, 0xff, 0x52} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Lightblue/White/Cyan");
  } else if (d == 10) { 
    uint8_t rgb[6][3] = { {0x00, 0xff, 0xff}, {0x00, 0xff, 0xff}, {0x64, 0x00, 0xff}, {0x64, 0x00, 0xff}, {0x00, 0xff, 0x52}, {0x00, 0xff, 0x52} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Lightblue/Purple/Cyan");
  } else if (d == 11) { 
    uint8_t rgb[4][3] = { {0xff, 0x00, 0xff}, {0xff, 0x00, 0xff}, {0x64, 0x00, 0xff}, {0x64, 0x00, 0xff} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Magenta/Purple");
  } else if (d == 12) { 
    uint8_t rgb[4][3] = { {0x64, 0x00, 0xff}, {0x64, 0x00, 0xff}, {0x00, 0xff, 0x6e}, {0x00, 0xff, 0x6e} }; 
   gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
    Serial.println("Purple/Cyan");
  }else {
    return;
  }
}
void changeColor(int d) {
//colors in hex (decimal: 0-255) - the last 3 in rgb_r are effects
uint8_t rgb_r[10] = {0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff};
uint8_t rgb_g[10] = {0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x0c, 0x00};
uint8_t rgb_b[10] = {0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x21, 0x00, 0x19}; 
  if (d == 0) { //BLE      
      if (color_1 > 9) {
        color_1 = 0;
      }
    uint8_t bytes[9] = {0x7e, 0xff, 0x05, 0x03, rgb_r[color_1], rgb_g[color_1], rgb_b[color_1], 0xff, 0xef};  
    Serial.println("BLE solid color");
    pRemoteCharacteristic->writeValue(bytes, 9);        
    color_1++;  
    preferences.putInt("color_1", color_1);
   } else if (d == 1) { //DMX        
      if (color_2 > 22) {
        color_2 = 0;
        gradientColors(color_2);
      } else if (color_2 < 13) {
        gradientColors(color_2);
      } else if (color_2 > 12) {
        uint8_t bytes[9] = {0x7b, 0x00, 0x07, rgb_r[color_2 - 13], rgb_g[color_2 - 13], rgb_b[color_2 - 13], 0xff, 0xff, 0xbf}; 
        Serial.println("DMX solid color");
        pRemoteCharacteristic->writeValue(bytes, 9);
      }        
      color_2++;
      preferences.putInt("color_2", color_2);
   }
}

 void controlBrightness () {
int analogValue = analogRead(34); //using 1K potentiometer to control brightness from 10 to 100 - pin 34
int brightness = map(analogValue, 0, 4095, 10, 100);
  if ((gbright > brightness+5) || (gbright < brightness-5)) { //filtering out noise from the poteniometer    
      gbright = brightness;
      uint8_t bytes[9] = {0x7b, 0xff, 0x01, 0x00, brightness, 0x02, 0xff, 0xff, 0xbf}; 
      pRemoteCharacteristic->writeValue(bytes, 9);
      preferences.putInt("gbright", gbright);
  }
}

void cycleEffects(int d) {
    if (d == 0) {        
    effect_2++;      
    if (effect_2 > 0xd1) { //There are 210 default animations/effects
          effect_2 = 0x01;
        }          
    } else if (d == 1) {    
    effect_2--;  
    if (effect_2 < 0x01) {
        effect_2 = 0xd1;  
      }            
    }
  uint8_t bytes[9] = {0x7b, 0xff, 0x03, effect_2, 0xff, 0xff, 0xff, 0xff, 0xbf}; 
  Serial.println("DMX effect cycle");
  pRemoteCharacteristic->writeValue(bytes, 9);
  preferences.putInt("effect_2", effect_2);
}

boolean turnOn() {
  if (turnoff == 1) { 
    uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x07, 0xff, 0xff, 0xff, 0xff, 0xbf}; 
    Serial.println("TURN ON");
    pRemoteCharacteristic->writeValue(bytes, 9);   
    turnoff = 0;
    preferences.putInt("turnoff", turnoff);
    return true;
  } else {
     return false;
  }
}

void resetLEDs () { //Resets connection to subcontrollers by updating number of leds to avoid garbage data/wrong colors being shown
    delay(50);
    uint8_t bytes[9] = {0x7b, 0xff, 0x05, 0x04, 0x00, 0xa4, 0x03, 0xff, 0xbf}; 
    Serial.println("RESET LEDs");
    pRemoteCharacteristic->writeValue(bytes, 9);   
    delay(50);
    uint8_t bytes2[9] = {0x7b, 0xff, 0x05, 0x04, 0x00, 0xa8, 0x03, 0xff, 0xbf}; 
    pRemoteCharacteristic->writeValue(bytes2, 9);   
}

void turnOff() {
  if (turnoff == false) {
    uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x06, 0xff, 0xff, 0xff, 0xff, 0xbf}; 
    Serial.println("TURN OFF");
    pRemoteCharacteristic->writeValue(bytes, 9);                 
    turnoff = 1;
    preferences.putInt("turnoff", turnoff);
    resetLEDs();
  }
} 

void toggleSwitch() {
 toggle++;
 if (toggle == 1) {
  effect_2 = 0x00; //reset effects for DMX
  preferences.putInt("effect_2", effect_2);
 } else if (toggle > 1) {
  toggle = 0;
  color_2 = 0;
  preferences.putInt("color_2", color_2);
 }
 preferences.putInt("toggle", toggle);
}

void onPressed1() {
  if (turnOn() == true) {
    return;
  } else {
    if (toggle == 0) {
      Serial.println("Input 1: DMX"); 
      changeColor(1);
    } else if (toggle == 1) {
      Serial.println("Input 1: ++effect");       
      cycleEffects(0);
    }
  } 
  delay(100);
}

void onPressed2() {
    if (turnOn() == true) {
      return;
  } else {
    if (toggle == 0) {
      Serial.println("Input 2: BLE"); 
      changeColor(0);
    } else if (toggle == 1) {
      Serial.println("Input 2: --effect");       
      cycleEffects(1);
    }
  }
  delay(100);
} 

/**************************************************************/
/**************************************************************/
/**************************************************************/

void loadSettings() {
turnoff = preferences.getInt("turnoff", 0);
color_1 = preferences.getInt("color_1", 0);
color_2 = preferences.getInt("color_2", 0);
gbright = preferences.getInt("gbright", 0);;
effect_2 = preferences.getInt("effect_2", 0);
toggle = preferences.getInt("toggle", 0);
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  button1.begin();
  button2.begin();
  button1.onPressed(onPressed1); 
  button2.onPressed(onPressed2); 
  button1.onPressedFor(1000, toggleSwitch);
  button2.onPressedFor(1000, turnOff);    
  preferences.begin("my-app", false);
  loadSettings();
  Serial.begin(115200);  
  delay(4000); //Startup is too fast for the LED controller to handle :)
  Serial.println("Starting NimBLE Client");
  NimBLEDevice::init("");
  pClient = NimBLEDevice::createClient(address);
  pClient->connect();
  pRemoteService = pClient->getService(serviceUUID);
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);

  Serial.print("Connected to: ");
  Serial.println(pClient->getPeerAddress().toString().c_str());
  Serial.print("RSSI: ");
  Serial.println(pClient->getRssi());
} 

void loop (){
    button1.read();
    button2.read();
    controlBrightness();
    /** Found a device we want to connect to, do it now */
    if(!pClient->isConnected()) {
        Serial.println("Failed to connect, trying to reconnect!");
        digitalWrite(LED_BUILTIN, LOW);
        pClient->connect();
        pConnect = false;
    } else if ((pClient->isConnected()) && (!pConnect)) {
        Serial.println("Connected to server!");
        digitalWrite(LED_BUILTIN, HIGH);    
        pConnect = true;    
        if (firstConnect == true) {
          resetLEDs();
        }
    }
}