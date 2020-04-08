#include <XBee.h>
#include <FreqCount.h>

float count;

bool transmitSuccess = false;

//bool message2 = false;
//bool message3 = false;

XBee xbee = XBee();

uint8_t payload[] = { 0 , 0 };

Tx16Request tx = Tx16Request(0x0088, payload, sizeof(payload));

TxStatusResponse txStatus = TxStatusResponse();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  xbee.setSerial(Serial);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); //signal no message sent
  FreqCount.begin(1000);
}

void loop() {
  //transmit if message hasn't already been sent
    /*if (!message1) {
      payload[0] = 1;
      digitalWrite(LED_BUILTIN, LOW);
      xbee.send(tx);
      message1 = true;
    }
    else if (!message2) {
     
      payload[0] = 2;
      delay(10000);
      digitalWrite(LED_BUILTIN, LOW);
      xbee.send(tx);
      message2 = true;
    }
    */

    if (FreqCount.available()) {
       count = FreqCount.read();
       Serial.print("Freq: ");
       Serial.print(count);
       Serial.println(" Hz");
    }

    if (count > 1000000 && !transmitSuccess) {
        payload[0] = 1;
        digitalWrite(LED_BUILTIN, LOW);
        xbee.send(tx);
    }
    
    if (xbee.readPacket(5000)) {
        // got a response!

        // should be a znet tx status              
      if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
         xbee.getResponse().getTxStatusResponse(txStatus);
        
         // get the delivery status, the fifth byte
           if (txStatus.getStatus() == SUCCESS) {
              // success.  time to celebrate
              transmitSuccess = true;
              digitalWrite(LED_BUILTIN, HIGH); //keep high
           } 
        }      
    } else if (xbee.getResponse().isError()) {
      //nss.print("Error reading packet.  Error code: ");  
      //nss.println(xbee.getResponse().getErrorCode());
      // or flash error led
    } else {
      // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
    }
  
}
