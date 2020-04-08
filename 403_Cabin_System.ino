#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <XBee.h>

//LCD display
LiquidCrystal_I2C lcd(0x24, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
Rx16Response rx16 = Rx16Response();

//constants for xbee communication
volatile uint8_t option = 0;
volatile uint8_t data = 0;

//indicators of what to display on lcd
volatile bool messageReceived = false;
volatile bool isToothFallen = false;
volatile bool lowBattery = false;
volatile bool reset = false;

void setup() {
  //enable interrupt for reset button
  attachInterrupt(1, systemReset, FALLING);
  
  lcd.begin(16, 2);
  lcd.backlight();
  pinMode(2, OUTPUT);
  pinMode(3, INPUT);
  lcd.clear();

  //xbee communication
  Serial.begin(9600);
  xbee.setSerial(Serial);
  
  lcd.setCursor(0, 0);
  lcd.print("All systems are");
  lcd.setCursor(0, 1);
  lcd.print("functioning.");
}

void loop() {
  //complete serial reading here 
  xbee.readPacket();
    
    if (xbee.getResponse().isAvailable()) {
      // got something
      
      if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
        // got a rx packet
          xbee.getResponse().getRx16Response(rx16);
          option = rx16.getOption();
          data = rx16.getData(0); 
          messageReceived = true;
          if  (data == 1) { isToothFallen = true;}
          else if (data == 2) { lowBattery = true; }
        // set dataLed PWM to value of the first byte in the data
      } else {
        // not something we were expecting
        lcd.print("unknown"); 
      }
    } else if (xbee.getResponse().isError()) {
      //nss.print("Error reading packet.  Error code: ");  
      //nss.println(xbee.getResponse().getErrorCode());
      // or flash error led
    } 


  //message has been read, now interperet!
    //check if message is tooth or battery
    if(messageReceived) { //if message was detected
      bool printed = false;
      while (data != 1 && data != 2) {
        if (reset) {
          reset = false;
          digitalWrite(2, LOW);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("All systems are");
          lcd.setCursor(0, 1);
          lcd.print("functioning.");
          break;
        }
       //unknown value
       if (!printed) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Unknown message");
          printed = true;
       }
      }
      
      while(isToothFallen) {//tooth fallen
        //check reset on each while
        if (reset) {
          reset = false;
          digitalWrite(2, LOW);
           //send reset to bucket
           lcd.clear();
           lcd.setCursor(0, 0);
           lcd.print("All systems are");
           lcd.setCursor(0, 1);
           lcd.print("functioning.");
           isToothFallen = false;
          break;
        }
   
        lcd.clear();
        digitalWrite(2, LOW);
        delay(500); //keep low for half a second
        lcd.setCursor(0, 0);
        //turn on LED and LCD
        digitalWrite(2, HIGH);
        lcd.print("GET malfunction");
        delay(1000); //keep high for a second
      
      }

      while(lowBattery) {//low battery
        //check reset on each while
        if (reset) {
          reset = false;
          digitalWrite(2, LOW);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("All systems are");
          lcd.setCursor(0, 1);
          lcd.print("functioning.");
          lowBattery = false;
          break;
          
       }

        lcd.clear();
        digitalWrite(2, LOW);
        delay(500); //keep low for half a second
        lcd.setCursor(0, 0);
        //turn on LED and LCD
        digitalWrite(2, HIGH);
        lcd.print("Replace bucket");
        lcd.setCursor(0, 1);
        lcd.print("system battery.");
        delay(1000); //keep high for a second
      }
    }
  }

void systemReset() {
  messageReceived = false;
  data = 0; 
  option = 0;
  
  //enable reset bool to signal restart of loop and serial printing to endpoint
  reset = true;
}
