/**
 * node.ino
 * 
 * This sketch is for nodes in the GardeNet system. Functions include:
 *    - Establishing and maintaining a mesh network
 *    - Controlling a valve
 *    - Controlling a flow meter (optionally)
 *    - Relaying information to the master
 */


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////  Preprocessor   //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// includes
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
#include <TimerOne.h>

// pins
#define CE 7
#define CS 8
#define FLOWSENSORPIN 2
#define BUTTON  3
#define LEDY    6
#define LEDR    5

// mesh settings
#define nodeID        02
#define masterAddress 0


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Globals     //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// mesh network
RF24 radio(CE, CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

// flow sensor
volatile uint16_t pulses = 0; // count how many pulses
volatile uint8_t lastflowpinstate; // track the state of the pulse pin
volatile uint32_t lastflowratetimer = 0; // you can try to keep time of how long it is between pulses
volatile float flowrate; // and use that to calculate a flow rate
bool changedPulseFlag = false; // interrupt is called once a millisecond, looks for any pulses from the sensor

// structs


// other
uint32_t displayTimer = 0;
volatile bool hadButtonPress = false;
//bool LEDYstate = false;
int valveState = false;
bool LEDRstate = false;
int myStatus = 1;
unsigned statusCounter = 0;


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     ISRs        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/*
 * Regardless of current state, send message to other to turn on LEDY
 */
void handleButton(){
    if(statusCounter > 2){   // gets rid of startup error
      hadButtonPress = true; 
      Serial.println(F("Detected buttonpress"));
    }
}


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Helper Functions//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void printStatus(){
  
  noInterrupts();

  Serial.print('\n'); Serial.println(statusCounter++);
  
  if(!mesh.checkConnection()){
    Serial.println("Not connected... renewing address");
    if(mesh.renewAddress() == -1)
      Serial.println("NOT CONNECTED");
    else
      Serial.println("Reconnected");
  }
  else
    Serial.println("Connected");

  Serial.print("My ID:      ");
  Serial.println(mesh.getNodeID());
  //Serial.print("My address: ");
  //int i = nodeID;
  //Serial.println(mesh.getAddress(nodeID));

  interrupts();
}

SIGNAL(TIMER0_COMPA_vect){
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

bool safeMeshWrite(void* payload, char header, unsigned datasize){
  if (!mesh.write(masterAddress, payload, header, datasize)) {
    // If a write fails, check connectivity to the mesh network
    if (!mesh.checkConnection() ) {
      //refresh the network address
      Serial.println(F("Renewing Address"));
      mesh.renewAddress();
    } 
    else {
      Serial.println(F("Send fail, Test OK"));
    }
    
    return false;
  }
  else {
//    Serial.print(F("Send OK: "));
//    Serial.println(*payload);
    return true;
  }
}

int setValve(bool setTo){
  // set valve
  valveState = setTo ? 1 : 0;

  // LEDY replicates valve state
  digitalWrite(LEDY, setTo);
  return valveState;
}


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Setup       //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void setup(){
  // init LEDs and Button
  pinMode(BUTTON, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING);
  pinMode(LEDY, OUTPUT);
  pinMode(LEDR, OUTPUT);

  // "power-on" light sequence
  digitalWrite(LEDY, LOW);
  digitalWrite(LEDR, HIGH);
  delay(500);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, HIGH);

  // begin serial communication
  Serial.begin(9600);

  // begin mesh communication
  mesh.setNodeID(nodeID); // do manually
  Serial.print(F("Connecting to the mesh...\nOutput of mesh.begin(): "));
  Serial.println(mesh.begin(12, RF24_250KBPS, 20000));

  // "connected" light sequence
  digitalWrite(LEDY, LOW);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);
  delay(250);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);

  // init timer for serial communication printing
  Timer1.initialize(5000000);
  Timer1.attachInterrupt(printStatus);

  // init flow sensor
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);

  // close valve
  setValve(false);

  // attach interrupt to button
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING);
}


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Loop        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void loop() {

  // refresh the mesh
  mesh.update();

  // handle button presses--send 'B' type message
  // NOT FINAL CODE
  if(hadButtonPress){
    // 'B' message sends the accumulated water flow, in liters
    float liters = pulses/7.5/60.0;
    safeMeshWrite(&liters, 'r', sizeof(float));
    /*if (!mesh.write(&liters, 'B', sizeof(float))) {
      // If a write fails, check connectivity to the mesh network
      if (!mesh.checkConnection() ) {
        //refresh the network address
        Serial.println("Renewing Address");
        mesh.renewAddress();
      } 
      else
        Serial.println("Send fail, Test OK");
    }
    else {
      Serial.print("Send OK: ");
      Serial.println(liters);
    }*/

    // reset flag
    hadButtonPress = false;
  }

  // handle flow meter pulses
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

  // read in messages
  while (network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    //void* payload = malloc(sizeof(float)); // float is largest possible size
    //network.read(header, payload, sizeof(payload));
    Serial.print(F("Received ")); Serial.print(char(header.type)); Serial.println(F(" type message."));

    switch(header.type){
    case 'V':
      // Valve command, on or off; type is bool
      //  set the value and send return message
      bool onOrOff;
      network.read(header, &onOrOff, sizeof(onOrOff));
      setValve(onOrOff);
      safeMeshWrite(&valveState, 'v', sizeof(valveState));
      break;
      
    case 'R':    
      // Request; type is char
      char typeOfData;
      network.read(header, &typeOfData, sizeof(typeOfData));
      
      switch( typeOfData ){
      case 'v':
        // tell current valve state
        safeMeshWrite(&valveState, 'v', sizeof(valveState));
        break;
      case 'r':
        // tell current flow rate
        float liters;
        liters = pulses/7.5/60.0; // TODO: make flowrate; currently is accumulated water
        safeMeshWrite(&liters, 'r', sizeof(liters));
        break;
      case 'n':
        // return status
        safeMeshWrite(&myStatus, 'n', sizeof(myStatus));
        break;
      default:
        break;
      }
      
      break;
    default:
      break;
    }
  }
}

