/*
TMRh20 2014 - Updated to work with optimized RF24 Arduino library
*/
 
 
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>
 
using namespace std;
 
//
// Hardware configuration
// Configure the appropriate pins for your connections
 
/****************** Linux ***********************/
// Radio CE Pin, CSN Pin, SPI Speed
// CE Pin uses GPIO number with BCM and SPIDEV drivers, other platforms use their own pin numbering
// CS Pin addresses the SPI bus number at /dev/spidev<a>.<b>
// ie: RF24 radio(<ce_pin>, <a>*10+<b>); spidev1.0 is 10, spidev1.1 is 11 etc..
 
// Generic:
RF24 radio(22,0);
 
/****************** Linux (BBB,x86,etc) ***********************/
// See http://tmrh20.github.io/RF24/pages.html for more information on usage
// See http://iotdk.intel.com/docs/master/mraa/ for more information on MRAA
// See https://www.kernel.org/doc/Documentation/spi/spidev for more information on SPIDEV
 
/********** User Config *********/
// Assign a unique identifier for this node, 0 or 1. Arduino example uses radioNumber 0 by default.
bool radioNumber = 1;
 
/********************************/
 
 
// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t addresses[][6] = {"1Node", "2Node"};
 
uint8_t counter = 1;                                                          // A single byte to keep track of the data being sent back and forth
 
 
int main(int argc, char** argv)
{
 
    cout << "RPi/RF24/examples/gettingstarted_call_response\n";
    radio.begin();
    radio.enableAckPayload();               // Allow optional ack payloads
    radio.enableDynamicPayloads();
    radio.printDetails();                   // Dump the configuration of the rf unit for debugging
 
 
    /***********************************/
    // This opens two pipes for these two nodes to communicate
    // back and forth.
    if (!radioNumber) {
        radio.openWritingPipe(addresses[0]);
        radio.openReadingPipe(1, addresses[1]);
    } else {
        radio.openWritingPipe(addresses[1]);
        radio.openReadingPipe(1, addresses[0]);
    }
    radio.startListening();
    radio.writeAckPayload(1, &counter, 1);
 
    // forever loop
    while (1) {
 
 
 
        /****************** Pong Back Role ***************************/
 

            uint8_t pipeNo, gotByte;                        // Declare variables for the pipe and the byte received
            uint16_t val[2];
            if (radio.available(&pipeNo)) {                // Read all available payloads
                radio.read(&val, sizeof(val));
                // Since this is a call-response. Respond directly with an ack payload.
                                              // Ack payloads are much more efficient than switching to transmit mode to respond to a call
              //  radio.writeAckPayload(pipeNo, &val, sizeof(val));   // This can be commented out to send empty payloads.
                printf("Received from Transmitter %d, a soil reading of %d \n\r", val[0],val[1]);
                delay(500); //Delay after a response to minimize CPU usage on RPi
                //Expects a payload every second
            }
        
 
    } //while 1
} //main
