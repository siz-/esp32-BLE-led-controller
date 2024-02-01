#include <NimBLEDevice.h>
#include <Preferences.h>
#include <ESP32Encoder.h>
#include <EasyButton.h>
#define DEBUG 0

/********************** NimBLE Client *************************/
static NimBLEAddress address("xx:xx:xx:xx:xx:xx"); //Device address 
static NimBLEUUID serviceUUID("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"); //Writeable Service ID
static NimBLEUUID charUUID("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"); //Writeable Char ID
static NimBLERemoteCharacteristic* pRemoteCharacteristic;
static NimBLERemoteService* pRemoteService;
static NimBLEClient* pClient;
boolean pConnect = false;
boolean firstConnect = true;
Preferences preferences;
/********************** Rotatary encoders *********************/
ESP32Encoder encoder1;
EasyButton Button1(14, 50);
ESP32Encoder encoder2;
EasyButton Button2(25, 50);
ESP32Encoder encoder3;
EasyButton Button3(4, 50);
long oldPosition1 = 0;
long oldPosition2 = 0;
long oldPosition3 = 5;
/*************************     LED    *************************/
/*LED:
  rgb: 7b 01 07 Rx Gx Bx ff ff bf
  brightness: 7b ff 01 00 xx 02 ff ff bf
  on: 7b ff 04 07 ff ff ff ff bf
  off: 7b ff 04 06 ff ff ff ff bf 
  identify controller: 7e ff 12 xx ff ff ff ff ef //from 02 etc.
  Number of leds: 7b ff 05 04 00 xx 03 ff bf
RGB:
  rgb: 7e ff 05 03 Rx Gx Bx ff ef
  brightness: 7e ff 01 xx 00 ff ff ff ef
  mode_speed: 7e ff 02 xx ff ff ff ff ef
  set_mode:   7e ff 03 xx 03 ff ff ff ef
  on: 7e ff 04 01 ff ff ff ff ef
  off: 7e ff 04 00 ff ff ff ff ef
aRGB:
  rgb: 7b 00 07 Rx Gx Bx ff ff bf
  brightness: 7b 00 01 00 xx 00 ff ff bf
  on: 7b ff 04 01 ff ff ff ff bf
  off: 7b ff 04 00 ff ff ff ff bf
  mode_speed: 7b ff 02 xx 00 ff ff ff bf
  set_mode: 7b ff 03 xx ff ff ff ff bf (210 modes - from 01 to d2)
  welcome animation: 7e ff 12 00 ff ff ff ff ef //toggle on welcome animation
  cancel welcome animation: 7e ff 12 01 ff ff ff ff ef

-- CUSTOM aRGB mode/animation --
7b 0y 0e fd Rx Gx Bx 0z bf // forward - fd can be changed to backward etc., not had the interest to sniff out other modes

y: color number - starts from 1
z: total number of colors
standard effect speed: 7bff023000ffffffbf */
int turnoff;
int color_1;
int color_2;
int color_3;
int gbright;
int bright_1;
int bright_2;
int effect_2;
int toggle_1;
int toggle_2;
int toggle_3;
bool off_1;
bool off_2;

void gradientFunction(uint8_t rgb[][3], int length) {
  uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff, 0xbf}; //turn off
  pRemoteCharacteristic->writeValue(bytes, 9);
  delay(100);
  for (int i = 0; i < length; i++) {                                                
    uint8_t bytes[9] = {0x7b, 0x01 + i, 0x0e, 0xfd, rgb[i][0], rgb[i][1], rgb[i][2], length, 0xbf}; 
    pRemoteCharacteristic->writeValue(bytes, 9);
    delay(50);    
  }
    delay(100);
  uint8_t bytes2[9] = {0x7b, 0xff, 0x04, 0x01, 0xff, 0xff, 0xff, 0xff, 0xbf}; //turn on
  pRemoteCharacteristic->writeValue(bytes2, 9);
  delay(50);
  pRemoteCharacteristic->writeValue(bytes2, 9);      
}

void gradientColors(int d) {
  switch (d) {
    case 0: {
      uint8_t rgb[4][3] = { {0xff, 0x00, 0x00}, {0xff, 0x11, 0x11}, {0xea, 0x1f, 0x10}, {0xff, 0x00, 0x00} };            
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Red gradient");
      #endif
      break;
      }
    case 1: {
      uint8_t rgb[4][3] = { {0x00, 0xff, 0x00}, {0x1a, 0xff, 0x1a}, {0x38, 0xfc, 0x38}, {0x00, 0xff, 0x00} };
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1      
      Serial.println("Green gradient");
      #endif
      break;
    }
    case 2: {
      uint8_t rgb[4][3] = { {0x00, 0x00, 0xff}, {0x2b, 0x5e, 0xf6}, {0x58, 0x85, 0xf7}, {0x00, 0x00, 0xff} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Blue gradient");
      #endif
      break;
    }
    case 3: {
      uint8_t rgb[3][3] = { {0xff, 0xff, 0x00}, {0xf3, 0xa2, 0x20}, {0xff, 0xff, 0x20} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Yellow gradient");
      #endif
      break;
    }
    case 4: {
      uint8_t rgb[3][3] = { {0xff, 0x00, 0xff}, {0x7c, 0x18, 0xf5}, {0xd1, 0x4c, 0xf7} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Magenta gradient");
      #endif
      break;
    }
    case 5: {
      uint8_t rgb[3][3] = { {0x00, 0xff, 0xff}, {0x1, 0x5f, 0xfa}, {0x45, 0xff, 0xff} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Lightblue gradient");
      #endif
      break;
    }
    case 6: {
      uint8_t rgb[3][3] = { {0x00, 0xff, 0x37}, {0x75, 0xfb, 0x89}, {0x00, 0xff, 0x21} };
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Lightgreen gradient");
      #endif
      break;
    }
    case 7: {
      uint8_t rgb[4][3] = { {0xff, 0x0c, 0x00}, {0xff, 0x26, 0x00}, {0xff, 0x3a, 0x00}, {0xff, 0x0c, 0x00} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Amber gradient");
      #endif
      break;
    }
    case 8: {
      uint8_t rgb[4][3] = { {0x64, 0x00, 0xff}, {0x64, 0x00, 0xff}, {0x00, 0xff, 0x6e}, {0x00, 0xff, 0x6e} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Purple/Cyan");
      #endif
      break;
    }
    case 9: {
      uint8_t rgb[4][3] = { {0x00, 0xff, 0x52}, {0x00, 0xff, 0x52}, {0xff, 0x00, 0x85}, {0xff, 0x00, 0x85} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Cyan/Pink");
      #endif
      break;
    }
    case 10: {
      uint8_t rgb[6][3] = { {0xff, 0x3c, 0x00}, {0xff, 0x3c, 0x00}, {0x64, 0x00, 0xff}, {0xff, 0x3c, 0x00}, {0x64, 0x00, 0xff}, {0x64, 0x00, 0xff} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Orange/Purple");
      #endif
      break;
    }
    case 11: {
      uint8_t rgb[6][3] = { {0x00, 0x6e, 0xff}, {0x00, 0x6e, 0xff}, {0x00, 0xff, 0x52}, {0x00, 0xff, 0x52}, {0xff, 0x00, 0x85}, {0xff, 0x00, 0x85} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Lighter blue/Cyan/Pink");
      #endif
      break;
    }
    case 12: {
      uint8_t rgb[6][3] = { {0x00, 0xff, 0xff}, {0x00, 0xff, 0xff}, {0x64, 0x00, 0xff}, {0x64, 0x00, 0xff}, {0x00, 0xff, 0x52}, {0x00, 0xff, 0x52} }; 
      gradientFunction(rgb, (sizeof(rgb) / sizeof(rgb[0])));
      #if DEBUG == 1
      Serial.println("Lightblue/Purple/Cyan");
      #endif
      break;
    }
    default:
      return;
  }
}

void changeColor(int d) {
//colors in hex (decimal: 0-255)
uint8_t rgb_r[10] = {0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff};
uint8_t rgb_g[10] = {0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x0c, 0x00};
uint8_t rgb_b[10] = {0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x21, 0x00, 0x19}; 
  switch (d) { 
    case 0: {      
      //uint8_t bytes[9] = {0x7e, 0xff, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff, 0xef};  //turn off
      uint8_t bytes[9] = {0x7e, 0xff, 0x05, 0x03, 0x00, 0x00, 0x00, 0xff, 0xef};  
      pRemoteCharacteristic->writeValue(bytes, 9);
      delay(200);
      uint8_t bytes2[9] = {0x7e, 0xff, 0x05, 0x03, rgb_r[color_1], rgb_g[color_1], rgb_b[color_1], 0xff, 0xef};  
      pRemoteCharacteristic->writeValue(bytes2, 9);
      delay(50);
      pRemoteCharacteristic->writeValue(bytes2, 9);
      #if DEBUG == 1
      Serial.println("RGB solid color");
      Serial.println(color_1);
      #endif      
      preferences.putInt("color_1", color_1);    
      break;
    }
    case 1: {
      if (color_2 < 13) {
        gradientColors(color_2);
      } else if (color_2 > 12) {
        uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff, 0xbf};  //turn off
        pRemoteCharacteristic->writeValue(bytes, 9);
        delay(100);
        uint8_t bytes2[9] = {0x7b, 0x00, 0x07, rgb_r[color_2 - 13], rgb_g[color_2 - 13], rgb_b[color_2 - 13], 0xff, 0xff, 0xbf}; 
        pRemoteCharacteristic->writeValue(bytes2, 9);
        #if DEBUG == 1
        Serial.println("aRGB solid color");
        Serial.println(color_2);
        #endif        
        delay(100);
        uint8_t bytes3[9] = {0x7b, 0xff, 0x04, 0x01, 0xff, 0xff, 0xff, 0xff, 0xbf};  //turn on
        pRemoteCharacteristic->writeValue(bytes3, 9);
        delay(50);
        pRemoteCharacteristic->writeValue(bytes3, 9);

      }        
      preferences.putInt("color_2", color_2);
      break;
    }
    case 2: {
      uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x06, 0xff, 0xff, 0xff, 0xff, 0xbf};  //turn off
      pRemoteCharacteristic->writeValue(bytes, 9);
      delay(100);      
      uint8_t bytes2[9] = {0x7b, 0x01, 0x07, rgb_r[color_3], rgb_g[color_3], rgb_b[color_3], 0xff, 0xff, 0xbf};  
      pRemoteCharacteristic->writeValue(bytes2, 9);         
      #if DEBUG == 1
      Serial.println("LED solid color");
      Serial.println(color_3);
      #endif      
      delay(100);
      uint8_t bytes3[9] = {0x7b, 0xff, 0x04, 0x07, 0xff, 0xff, 0xff, 0xff, 0xbf};  //turn on
      pRemoteCharacteristic->writeValue(bytes3, 9);         
      preferences.putInt("color_3", color_3);
      break;  
    }
   }
}

void cycleEffects() {
  uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff, 0xbf};  //turn off
  pRemoteCharacteristic->writeValue(bytes, 9);
  delay(100);
  uint8_t bytes2[9] = {0x7b, 0xff, 0x03, effect_2, 0xff, 0xff, 0xff, 0xff, 0xbf}; 
  #if DEBUG == 1
  Serial.println("aRGB effect cycle");
  #endif
  pRemoteCharacteristic->writeValue(bytes2, 9);
  preferences.putInt("effect_2", effect_2);
  delay(100);
  uint8_t bytes3[9] = {0x7b, 0xff, 0x04, 0x01, 0xff, 0xff, 0xff, 0xff, 0xbf};  //turn on
  pRemoteCharacteristic->writeValue(bytes3, 9);
}

boolean turnOn() {
  if (turnoff == 1) { 
    uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x07, 0xff, 0xff, 0xff, 0xff, 0xbf}; 
    #if DEBUG == 1
    Serial.println("TURN ON");
    #endif
    pRemoteCharacteristic->writeValue(bytes, 9);   
    turnoff = 0;
    preferences.putInt("turnoff", turnoff);
    encoder1.resumeCount();
    encoder2.resumeCount();
    encoder3.resumeCount();
    return true;
  } else {
     return false;
  }
}

void resetLEDs () { //Resets connection to subcontrollers by updating number of leds to avoid garbage data/wrong colors being shown
    delay(50);
    uint8_t bytes[9] = {0x7b, 0xff, 0x05, 0x04, 0x00, 0xa4, 0x03, 0xff, 0xbf}; 
    #if DEBUG == 1
    Serial.println("RESET LEDs");
    #endif
    pRemoteCharacteristic->writeValue(bytes, 9);   
    delay(50);
    uint8_t bytes2[9] = {0x7b, 0xff, 0x05, 0x04, 0x00, 0xa8, 0x03, 0xff, 0xbf}; 
    pRemoteCharacteristic->writeValue(bytes2, 9);   
}

void turnOff() {
  if (turnoff == 0) {
    uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x06, 0xff, 0xff, 0xff, 0xff, 0xbf}; 
    #if DEBUG == 1
    Serial.println("TURN OFF");
    #endif
    pRemoteCharacteristic->writeValue(bytes, 9);                 
    turnoff = 1;
    preferences.putInt("turnoff", turnoff);
    encoder1.pauseCount();
    encoder2.pauseCount();
    encoder3.pauseCount();
    resetLEDs();
  }
} 

void btn1() {
  if ((turnOn() == true) || (off_2 == true)) {
    return;
  } else {
  toggle_1++;
  switch (toggle_1) {      
    case 1:{             
      encoder1.setCount(effect_2 * 2);
      break; 
      }
    default: {
      toggle_1 = 0;
      encoder1.setCount(color_2 *2);
      break; 
      }
  }
  #if DEBUG == 1
  Serial.print("Button 1 toggle: ");
  Serial.println(toggle_1);
  #endif
  preferences.putInt("toggle_1", toggle_1);
  }
}

void btn1long() {
  if ((turnOn() == true) || (off_2 == true)) {
    return;
  } else {  
    if (toggle_1 == 1) {
      encoder1.setCount(1 * 2);
      #if DEBUG == 1
      Serial.println("Button 1: Reset effects");      
      #endif    

    } else {
      turnOff();
      #if DEBUG == 1
      Serial.println("Button 1: Turn off");
      #endif      
    }
  } 
}

void btn2() {
   if ((turnOn() == true) || (off_1 == true)) {
    return;
  } else {
    toggle_2++;
  switch (toggle_2) {      
    case 1:{             
      //oldPosition2 = color_3 - 1;
      encoder2.setCount(color_3 * 2);
      break; 
      }
    default: {
      toggle_2 = 0;
      encoder2.setCount(color_1 *2);
         if (toggle_1 == 1) {                              
          encoder1.setCount(effect_2 * 2);
          cycleEffects();
          } else {          
          encoder1.setCount(color_2 * 2);                              
          changeColor(1);
          }              
      break; 
      }
  }
  #if DEBUG == 1
  Serial.print("Button 2 toggle: ");
  Serial.println(toggle_2);
  #endif
  preferences.putInt("toggle_2", toggle_2); 
  }
}

void btn2long() {
  if ((turnOn() == true) || (off_1 == true)) {
    return;
  } else {  
      if ((toggle_1 == 1) && (toggle_2 = 1) && (toggle_3 == 2)) {
        if (pClient->isConnected()) {
          pClient->disconnect();
          digitalWrite(LED_BUILTIN, LOW);
          #if DEBUG == 1
          Serial.println("Button 2: Disconnect and go to deep sleep for 10 seconds"); //If you want to connect to the controller by phone
          #endif       
          esp_sleep_enable_timer_wakeup(6 * 1000000); //6 seconds + 4 seconds startup time for NimBLE
          esp_deep_sleep_start();
          #if DEBUG == 1
          Serial.println("Button 2: Delay over"); //This will never print as it runs setup() again after sleep
          #endif
      }  
    } else {
        turnOff();
        #if DEBUG == 1
        Serial.println("Button 2: Turn off");
        #endif
    }    
  }
} 

void turnOffaRGB() {
    if (off_2) {
    uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff, 0xbf}; //turn off
    pRemoteCharacteristic->writeValue(bytes, 9);
    encoder1.pauseCount();
  } else {      
    uint8_t bytes[9] = {0x7b, 0xff, 0x04, 0x01, 0xff, 0xff, 0xff, 0xff, 0xbf}; //turn on
    pRemoteCharacteristic->writeValue(bytes, 9);
    delay(50);
    pRemoteCharacteristic->writeValue(bytes, 9);
    encoder1.resumeCount();
  }
}

void turnOffRGB() {
  if (off_1) {
    uint8_t bytes[9] = {0x7e, 0xff, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff, 0xef}; //turn off
    pRemoteCharacteristic->writeValue(bytes, 9);  
    encoder2.pauseCount();
} else {
    uint8_t bytes[9] = {0x7e, 0xff, 0x04, 0x01, 0xff, 0xff, 0xff, 0xff, 0xef}; //turn on
    pRemoteCharacteristic->writeValue(bytes, 9);  
    delay(50);
    pRemoteCharacteristic->writeValue(bytes, 9);
    encoder2.resumeCount();
}
}

void btn3() {
  if (turnOn() == true) {
    return;
  } else {
    toggle_3++;
    switch (toggle_3) {      
      case 1: {
        encoder3.setCount((gbright / 5) * 2);
        bright_1 = gbright;
        break;
      }
      case 2: {
        encoder3.setCount((gbright / 5) * 2);
        bright_2 = bright_1;
        break;
      }
      default: {
        if (toggle_3 == 3) {
          encoder3.setCount(bright_1);
          gbright = bright_1;
        } else {
          encoder3.setCount((gbright / 5) * 2);
        }
        toggle_3 = 0;
        off_1 = false;
        off_2 = false;
        turnOffaRGB();
        turnOffRGB();
        break;
      }
    }
    #if DEBUG == 1
    Serial.print("Button 3 toggle: ");
    Serial.println(toggle_3);
    #endif
    preferences.putInt("toggle_3", toggle_3);
    }
}

void btn3long () {
  switch (toggle_3) {
    case 0: {
      turnOff();
      break;
    }
    case 1: {
        off_2 = !off_2;
        turnOffaRGB();
        break;
    }
    case 2: {
        off_1 = !off_1;
        turnOffRGB();
        break;
    }
  }
}

void encoder1Loop () {
long stepSize = 1;
long minVal;
long maxVal;
  if (toggle_1 == 1) {
    minVal = 1;
    maxVal = 209;
  } else {
    minVal = 0;
    maxVal = 22;
  }

long newPosition = (encoder1.getCount() / 2) * stepSize;

  if (oldPosition1 != newPosition) {
      if (newPosition < minVal) {
        encoder1.setCount((minVal * 2) / stepSize);
        newPosition = minVal;
      } else if ((newPosition > maxVal)) {
        encoder1.setCount((maxVal * 2) / stepSize);
        newPosition = maxVal;  
      } else {
        switch (toggle_1) {
          case 0: {
            color_2 = newPosition;
            changeColor(1);
            break;
          }
          case 1: {
            effect_2 = newPosition;
            cycleEffects();
            break;
          }
        }
        #if DEBUG == 1
        Serial.println(newPosition);
        #endif        
      }       
    oldPosition1 = newPosition;       
    }
}

void encoder2Loop () {
long minVal = 0;
long maxVal = 9;
long stepSize = 1;

long newPosition = (encoder2.getCount() / 2) * stepSize;

  if (oldPosition2 != newPosition) {
      if (newPosition < minVal) {
        encoder2.setCount((minVal * 2) / stepSize);
        newPosition = minVal;
      } else if ((newPosition > maxVal)) {
        encoder2.setCount((maxVal * 2) / stepSize);
        newPosition = maxVal;  
      } else {
        switch (toggle_2) {
            case 0: {                          
              color_1 = newPosition;
              changeColor(0); 
              break;
            }
            case 1: {
              color_3 = newPosition;
              changeColor(2);
              break;                        
            }
        }
      }       
    oldPosition2 = newPosition;       
    }
}

void encoder3Loop () {
long minVal = 10;
long maxVal = 100;
long stepSize = 5;

long newPosition = (encoder3.getCount() / 2) * stepSize;

  if (oldPosition3 != newPosition) {    
      if (newPosition < minVal) {
        encoder3.setCount((minVal * 2) / stepSize);
        newPosition = minVal;
      } else if ((newPosition > maxVal)) {
        encoder3.setCount((maxVal * 2) / stepSize);
        newPosition = maxVal;  
      } else {
        switch (toggle_3) {
          case 0: { //all LED
            gbright = newPosition;
            uint8_t bytes[9] = {0x7b, 0xff, 0x01, 0x00, newPosition, 0x02, 0xff, 0xff, 0xbf}; 
            pRemoteCharacteristic->writeValue(bytes, 9);
            preferences.putInt("gbright", gbright);
            #if DEBUG == 1
            Serial.print("Brightness: ");
            #endif
            break;         
          }
          case 1: { //aRGB         
            bright_1 = newPosition;
            uint8_t bytes[9] = {0x7b, 0x00, 0x01, 0x00, newPosition, 0x00, 0xff, 0xff, 0xbf}; 
            pRemoteCharacteristic->writeValue(bytes, 9);
            preferences.putInt("bright_1", bright_1);
            #if DEBUG == 1
            Serial.print("aRGB Brightness: ");
            #endif
            break;
            
          }
          case 2: { //RGB          
            bright_2 = newPosition;
            uint8_t bytes[9] = {0x7e, 0xff, 0x01, newPosition, 0x00, 0xff, 0xff, 0xff, 0xef}; 
            pRemoteCharacteristic->writeValue(bytes, 9);
            preferences.putInt("bright_2", bright_2);
            #if DEBUG == 1
            Serial.print("RGB Brightness: ");
            #endif
            break;
            }
        }
        #if DEBUG == 1
        Serial.println(newPosition);
        #endif
      }       
    oldPosition3 = newPosition;       
    }
}
/**************************************************************/
void loadSettings() {
turnoff = preferences.getInt("turnoff", 0);
color_1 = preferences.getInt("color_1", 0);
color_2 = preferences.getInt("color_2", 0);
color_3 = preferences.getInt("color_3", 0);
gbright = preferences.getInt("gbright", 15);;
bright_1 = preferences.getInt("bright_1", 15);
bright_2 = preferences.getInt("bright_2", 15);
effect_2 = preferences.getInt("effect_2", 0);
toggle_1 = preferences.getInt("toggle_1", 0);
toggle_2 = preferences.getInt("toggle_2", 0);
toggle_3 = preferences.getInt("toggle_3", 0);
off_1 = preferences.getBool("off_1", false);
off_2 = preferences.getBool("off_2", false);
}

void loadEncoderValues () {
  switch (toggle_1) {
    case 0:    
      encoder1.setCount(color_2 * 2);
      oldPosition1 = color_2;
      break;
    case 1:
      encoder1.setCount(effect_2 * 2);
      oldPosition1 = effect_2;
      break;
 }
encoder2.setCount(color_1 * 2);
oldPosition2 = color_1;
  switch (toggle_3) {
    case 0:
      encoder3.setCount((gbright / 5) * 2);
      oldPosition3 = gbright;
      break;
    case 1:
      encoder3.setCount((bright_1 / 5) * 2);
      oldPosition3 = bright_1;
      break;
    case 2:
      encoder3.setCount((bright_2 / 5) * 2);
      oldPosition3 = bright_2;
      break;
  }
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);     
  preferences.begin("my-app", false);
  loadSettings();
  #if DEBUG == 1
  Serial.begin(115200);  
  #endif
  //Rotary encoder
  encoder1.attachHalfQuad(13, 12);
  encoder1.setFilter(1023);
  encoder2.attachHalfQuad(27, 26);
  encoder2.setFilter(1023);
  encoder3.attachHalfQuad(16, 17);
  encoder3.setFilter(1023);
  loadEncoderValues();
  //Buttons
  Button1.begin();
  Button2.begin();
  Button3.begin();
  Button1.onPressed(btn1);
  Button1.onPressedFor(1000, btn1long);
  Button2.onPressed(btn2);
  Button2.onPressedFor(1000, btn2long);
  Button3.onPressed(btn3);
  Button3.onPressedFor(1000, btn3long);
    //NimBLE
  delay(4000); //Startup is too fast for the LED controller to handle :)
  #if DEBUG == 1
  Serial.println("Starting NimBLE Client");
  #endif
  
  NimBLEDevice::init("");
  pClient = NimBLEDevice::createClient(address);
  pClient->connect();
  pRemoteService = pClient->getService(serviceUUID);
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  
  #if DEBUG == 1
  Serial.print("Connected to: ");
  Serial.println(pClient->getPeerAddress().toString().c_str());
  Serial.print("RSSI: ");
  Serial.println(pClient->getRssi());
  #endif
} 

void loop() {  
  if ((!pClient->isConnected())) {
      #if DEBUG == 1
      Serial.println("Trying to reconnect!");
      #endif
      digitalWrite(LED_BUILTIN, LOW);
      pClient->connect();
      pConnect = false;
  } else if ((pClient->isConnected()) && (!pConnect)) {
      #if DEBUG == 1
      Serial.println("Connected to server!");
      #endif
      digitalWrite(LED_BUILTIN, HIGH);    
      pConnect = true;    
      if (firstConnect == true) {
        resetLEDs();
      }
  }
  encoder1Loop();
  Button1.read();
  encoder2Loop();
  Button2.read();  
  encoder3Loop();
  Button3.read();  
}
