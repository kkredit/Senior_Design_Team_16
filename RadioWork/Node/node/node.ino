
/** RF24Mesh_Example.ino by TMRh20
 *
 * This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
 * master node.
 * The nodes will refresh their network address as soon as a single write fails. This allows the
 * nodes to change position in relation to each other and the master node.
 */


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
//#include <printf.h>
#include <TimerOne.h>

#define CE 7
#define CS 8

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(CE, CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
 * User Configuration: nodeID - A unique identifier for each radio. Allows addressing
 * to change dynamically with physical changes to the mesh.
 *
 * In this example, configuration takes place below, prior to uploading the sketch to the device
 * A unique value from 1-255 must be configuLEDR for each node.
 * This will be stoLEDR in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.
 *
 **/
#define FLOWSENSORPIN  2
#define nodeID      02
#define BUTTON      3
#define LEDY        6
#define LEDR        5

uint32_t displayTimer = 0;
int printcount = 0;
volatile bool hadButtonPress = false;
bool LEDYstate = false;
bool LEDRstate = false;

/*********** Flow sensor vars **************/
// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
bool changedPulseFlag = false;
/*******************************************/

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Setup       //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDY, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING); 
  //delay(5000);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, LOW);

  digitalWrite(LEDR, HIGH);
  delay(500);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, HIGH);

  Serial.begin(9600);
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println("Connecting to the mesh...");
  Serial.print("Output of mesh.begin(): ");
  Serial.println(mesh.begin(12, RF24_250KBPS, 2000));
  //mesh.setAddress(nodeID, nodeID+1); 

  digitalWrite(LEDY, LOW);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);
  delay(250);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);
  
  Timer1.initialize(5000000);
  Timer1.attachInterrupt(printStatus);

  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);
}

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     ISRs        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/*
 * Regardless of current state, send message to other to turn on LEDY
 */
void handleButton(){
    hadButtonPress = true;
    Serial.println(F("Detected buttonpress"));
}

//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Helper Functions//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void printStatus(){
  Serial.println("\n");
  Serial.println(printcount++);
  Serial.print("\nMy ID:      ");
  Serial.println(mesh.getNodeID());
  Serial.print("My address: ");
  Serial.println(mesh.getAddress(0));
}

SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
    changedPulseFlag = true;
  }
  lastflowpinstate = x;
  flowrate = 1000.0 / lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Loop        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void loop() {

  mesh.update();

  // handle button presses
  if(hadButtonPress){
    // Send an 'B' type message to signal the master that there was a button press
    // 'B' message sends the accumulated water flow, in liters
    float liters = pulses/7.5/60.0;
    if (!mesh.write(&liters, 'B', sizeof(float))) {

      // If a write fails, check connectivity to the mesh network
      if ( ! mesh.checkConnection() ) {
        //refresh the network address
        Serial.println("Renewing Address");
        mesh.renewAddress();
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK: "); Serial.println(displayTimer);
    }

    hadButtonPress = false;
  }

  // Send to the master node every second
  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();

    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&displayTimer, 'M', sizeof(displayTimer))) {

      // If a write fails, check connectivity to the mesh network
      if ( ! mesh.checkConnection() ) {
        //refresh the network address
        Serial.println(F("Renewing Address"));
        mesh.renewAddress();
      } else {
        Serial.println(F("Send fail, Test OK"));
      }
    } else {
      Serial.print("Send OK: "); Serial.println(displayTimer);
    }
  }

  if (changedPulseFlag == true){
    Serial.print("Freq: "); Serial.println(flowrate);
    Serial.print("Pulses: "); Serial.println(pulses, DEC);
    // if a plastic sensor use the following calculation
    // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
    // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
    // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
    // Liters = Pulses / (7.5 * 60)
    changedPulseFlag = false;
    float liters = pulses/7.5/60.0;
    //liters /= 7.5;
    //liters /= 60.0;
    Serial.print(liters); Serial.println(" Liters");
  }

  while (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
}


