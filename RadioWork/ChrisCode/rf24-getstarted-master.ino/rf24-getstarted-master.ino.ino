/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/
#include <SPI.h>
#include "RF24.h"
/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(9, 10);
/**********************************************************/
byte addresses[][6] = {"1Node", "2Node"};
// Used to control whether this node is sending or receiving
bool role = 0;

#define BUTTON  3
#define LEDR    4
#define LEDG    5
#define LEDY    6
const unsigned long CODE = 1010101;
#include <TimerOne.h>
bool LEDRstate = false;
bool LEDYstate = false;
bool RFisListening = false;
bool hadButtonPress = false;
int numFlashes = 0;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDY, OUTPUT);
  delay(5000);
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING);  
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDY, LOW);

  digitalWrite(LEDR, HIGH);
  delay(500);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, HIGH);
  
  noInterrupts();
  Serial.begin(9600);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));
  interrupts();

  radio.begin();
  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if (radioNumber) {
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
  } else {
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
  }

  // Start the radio listening for data
  radio.startListening();
  RFisListening = true;

  digitalWrite(LEDY, LOW);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);
  delay(250);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);

  Timer1.initialize(1000000);
  Timer1.attachInterrupt(flashLEDR);
}

void flashLEDR(){
  LEDRstate = !LEDRstate;
  digitalWrite(LEDR, LEDRstate);
  if(LEDRstate){
    numFlashes++;
    Serial.print("Flash LEDR ");
    Serial.println(numFlashes);
  }
}

/*
 * Regardless of current state, send message to other to turn on LEDY
 */
void handleButton(){
    hadButtonPress = true;
    Serial.println(F("Deteceted Button Press"));
}

void loop() {

  if(hadButtonPress){
    radio.stopListening();                                  // First, stop listening so we can talk.
    if (!radio.write( &CODE, sizeof(unsigned long) )){
      Serial.println(F("failed"));
    }
    hadButtonPress = false;
    if(RFisListening) radio.startListening();               // Second, if was already listening
                                                              // turn back on
    noInterrupts();
    Serial.println(F("Buttonpress: Sending 'turn on LEDY' message"));
    interrupts();
  }   
    
  /****************** Ping Out Role ***************************/
  if (role == 1)  {

    radio.stopListening();                                    // First, stop listening so we can talk.
    RFisListening = false;

    noInterrupts();
    Serial.println(F("Now sending"));
    //unsigned long presentTime = micros();                             // Take the presentTime, and send it.  This will block until complete
    int myNum = 5555;
    if (!radio.write( &myNum, sizeof(myNum) )){
      Serial.println(F("failed"));
    }
    interrupts();

    radio.startListening();                                    // Now, continue listening
    RFisListening = true;

    unsigned long started_waiting_at = micros();               // Set up a presentTimeout period, get the current microseconds
    boolean presentTimeout = false;                                   // Set up a variable to indicate if a response was received or not

    while ( ! radio.available() ) {                            // While nothing is received
      if (micros() - started_waiting_at > 200000 ) {           // If waited longer than 200ms, indicate presentTimeout and exit while loop
        presentTimeout = true;
        break;
      }
    }

    if ( presentTimeout ) {                                            // Describe the results
        noInterrupts();
        Serial.println(F("Failed, response presentTimed out."));
        interrupts();
    }else{
        int got_presentTime;                                 // Grab the response, compare, and send to debugging spew
        radio.read( &got_presentTime, sizeof(unsigned long) );

        if(got_presentTime == CODE){   // TOGGLE LEDY
          LEDYstate = !LEDYstate;
          digitalWrite(LEDY, LEDYstate);
          noInterrupts();
          Serial.println(F("\nToggling LEDY\n"));
          interrupts();
        }
        else{                   // NORMAL
          unsigned long presentTime = micros();
        
          // Spew it
          noInterrupts();
          Serial.print(F("Sent "));
          Serial.print(myNum);
          Serial.print(F(", Got response "));
          Serial.println(got_presentTime);
          interrupts();
        }
    }
    // Try again 1s later
    delay(1000);
  }
  /****************** Ping Back Role ***************************/
  if ( role == 0 )
  {
    unsigned long got_presentTime;

    if ( radio.available()) {
      // Variable for the received presentTimestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &got_presentTime, sizeof(unsigned long) );             // Get the payload
      }

      if(got_presentTime == CODE){   // TOGGLE LEDY
        LEDYstate = !LEDYstate;
        digitalWrite(LEDY, LEDYstate);
        noInterrupts();
        Serial.println(F("\nToggling LEDY\n"));
        interrupts();
      }
      else{                   // NORMAL
        got_presentTime = 55555;
        radio.stopListening();                                        // First, stop listening so we can talk   
        RFisListening = false;
        radio.write( &got_presentTime, sizeof(unsigned long) );              // Send the final one back.      
        radio.startListening();                                       // Now, resume listening so we catch the next packets. 
        RFisListening = true;
        noInterrupts();
        Serial.print(F("Sent response "));
        Serial.println(got_presentTime);
        interrupts();      
      }
    }
  }
  /****************** Change Roles via Serial Commands ***************************/
  if ( Serial.available() )
  {
    noInterrupts();
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)
    
    }else
    if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();
       RFisListening = true;
    }
    interrupts();
  }
} // Loop

