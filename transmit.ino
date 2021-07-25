/*
   Dec 2014 - TMRh20 - Updated
   Derived from examples by J. Coliz <maniacbug@ymail.com>
*/
#include <SPI.h>
#include "RF24.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <EEPROM.h>

 
// You must include printf and run printf_begin() if you wish to use radio.printDetails();
//#include "printf.h"
 
/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

int STATUS_LED_PIN = 8;
int SENSOR_ACTIVATE_PIN = 7;
int UNIQUE_ID = 127;
int EEPROM_ADDRESS = 0;
int data[2];

int radioID;

 
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(9,10);
/**********************************************************/
                                                                           // Topology
byte addresses[][6] = {"1Node","2Node"};              // Radio pipe addresses for the 2 nodes to communicate.
 


                                                        // A single byte to keep track of the data being sent back and forth
 
 
void setup(){
 // EEPROM.put(EEPROM_ADDRESS, UNIQUE_ID); //change this each time, only run once
  radioID = (int) EEPROM.read(EEPROM_ADDRESS);
  Serial.print("Unique Radio ID is: ");
  Serial.println(radioID);


  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(SENSOR_ACTIVATE_PIN, OUTPUT);
  Serial.begin(9600);
  // printf_begin(); // This is for initializing printf that is used by printDetails()
  Serial.println(F("Beggining Transmission"));
 
  // Setup and configure radio
 
  radio.begin();
  radio.setRetries(3,5); // delay, count



  radio.enableAckPayload();                     // Allow optional ack payloads
  radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads
  
   if(radioNumber){
     radio.openWritingPipe(addresses[1]);        // Both radios listen on the same pipes by default, but opposite addresses
     radio.openReadingPipe(1,addresses[0]);      // Open a reading pipe on address 0, pipe 1
   }else{
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
}
  

  
}

int getSoilReading(){
  int val = analogRead(0); //connect sensor to Analog 0
  Serial.print("Soil Reading is ");
  Serial.println(val); //print the value to serial port
  return val;
}

bool sendData(int val){
     data[0] = radioID;
     data[1] = val;


    uint16_t response;
   
                                                            
    if ( radio.write(&data,sizeof(data)) ){                         // Send the counter variable to the other radio 
                              // If nothing in the buffer, we got an ack but it is blank
        if (radio.isAckPayloadAvailable() ) {
            radio.read(&response, sizeof(response) );                  // Read it, and display the response time
                Serial.print(F("Got response "));
                Serial.print(response);
                return true;
        }
        else {
            Serial.println("Sent Successfully");
            return true;
        }
    
    }else{        Serial.println(F("Sending failed.")); 
                  return false;
    
    }         
  

}

void alert(){
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(1000);
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(1000);
}

void enterSleep(){
  radio.powerDown();
  byte old_ADCSRA = ADCSRA;
  ADCSRA = 0;  

  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();


 
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();
  ADCSRA = old_ADCSRA;
  radio.powerUp();
}
 
void loop(void) {
  if(!radio.isChipConnected()){
      Serial.println("Radio not detected");
      alert();
  }else{
     digitalWrite(SENSOR_ACTIVATE_PIN, HIGH);
     delay(1000); //maybe give it a second to turn on and get the proper reading
     int val = getSoilReading();
     delay(1000); //maybe give it a second to turn on and get the proper reading
     digitalWrite(SENSOR_ACTIVATE_PIN, LOW);
     digitalWrite(STATUS_LED_PIN, HIGH);
     if(sendData(val)){
     digitalWrite(STATUS_LED_PIN, LOW);
      //  sleep
      for(int i= 0; i< 64; i++){
           enterSleep();
      }
     
     }else{
       delay(1000);
     }
  }

 
 

  // SOIL READING

 
 //427 is dry -- [pushed further in it's 420]-pushed even further it's 395 ...so x> 395 is dry
  
  //304 is kind moist --- also 288 

  
  //270 is wet 

/****************** Ping Out Role ***************************/
 
                              // Radio is in ping mode
 
  
  

}

// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect
 
 
 
