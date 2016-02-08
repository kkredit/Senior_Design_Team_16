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
//#include "RF24Mesh_config.h"
#include <SPI.h>
#include <EEPROM.h>
#include <TimerOne.h>
#include "C:\\Users\\kevin\\Documents\\Senior_Design_Team_16\\RadioWork\\Shared\\settings.h"
//#include "C:\\Users\\kevin\\Documents\\Senior_Design_Team_16\\RadioWork\\Shared\\SharedFunctions.h"

// pins
#define CE 7
#define CS 8
#define FLOWSENSORPIN 2
#define BUTTON  3
#define LEDY    6
#define LEDR    5
#define IDPIN_0 A0
#define IDPIN_1 A1
#define IDPIN_2 A2
#define IDPIN_3 A3

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
int8_t valveState = 0;
uint8_t myStatus = 0;
uint8_t statusCounter = 0;


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     ISRs        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/*
 * Triggered by button press on pin BUTTON
 * Sets flag, prints to serial port, and exits
 */
void handleButton(){
    if(statusCounter > 2){   // gets rid of startup false positive by ignoring for 2 seconds after startup
      hadButtonPress = true; 
      Serial.println(F("Detected buttonpress"));
    }
}


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Helper Functions//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/*
 * Checks mesh connection, prints status
 */
void printStatus(){
  // print number of times executed
  Serial.print('\n'); Serial.println(statusCounter++);

  // check mesh connection and print status
  if(!mesh.checkConnection()){
    // if unconnected, try to reconnect
    Serial.println("Not connected... renewing address");
    if(mesh.renewAddress() == -1)
      Serial.println("NOT CONNECTED");
    else
      Serial.println("Reconnected");
  }
  else
    Serial.println("Connected");

  // print node ID
  Serial.print("My ID:      ");
  Serial.println(mesh.getNodeID());
  // print address DOESN'T WORK
  //Serial.print("My address: ");
  //Serial.println(mesh.getAddress(readMyID));
}

/*
 * A function to do a mesh.write with automatic error handling
 * @param void* payload: a pointer to the payload
 * @param char header: the type of message
 * @param unsigned datasize: the size of the payload in bytes (use the sizeof() function)
 * @param unsigned timesToTry: the number of times to try to send the message if send fails
 * @return bool: true if send success, false if send fail
 */
bool safeMeshWrite(uint8_t destination, void* payload, char header, uint8_t datasize, uint8_t timesToTry){
  // perform write
  if (!mesh.write(destination, payload, header, datasize)) {
    // if a write fails, check connectivity to the mesh network
    Serial.print(F("Send fail... "));
    if (!mesh.checkConnection() ) {
      //refresh the network address
      Serial.println(F("renewing Address... "));
      if (!mesh.renewAddress()){
        // if failed, connection is down
        Serial.println(F("MESH CONNECTION DOWN"));
        return false;
      }
      else{
        if(timesToTry){
          // if succeeded, are reconnected and try again
          Serial.println(F("reconnected, trying again"));
          delay(RETRY_PERIOD);
          return safeMeshWrite(destination, payload, header, datasize, --timesToTry);
        }
        else{
          // out of tries; send failed
          Serial.println(F("reconnected, but out of send tries: SEND FAIL"));
          return false;
        }
        
      }
    } 
    else {      
      if(timesToTry){
        // if send failed but are connected and have more tries, try again after 50 ms
        Serial.println(F("mesh connected, trying again"));
        delay(RETRY_PERIOD);
        return safeMeshWrite(destination, payload, header, datasize, --timesToTry);
      }
      else{
        // out of tries; send failed
        Serial.println(F("out of send tries: SEND FAIL"));
        return false;
      }
    }    
    //return false;
  }
  else {
    // write succeeded
    Serial.print(F("Send of type ")); Serial.print(header); Serial.println(F(" success"));
    //Serial.println(*payload); // cannot dereference void*, and do not know type...
    return true;
  }
}

/*
 * Setup flow meter; borrowed function
 */
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

/*
 * Setup flow meter; borrowed function
 */
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

/*
 * Sets valve to desired state
 * @param bool setTo: the desired state
 * @param int: the actual state it was set to
 */
int setValve(bool setTo){
  // set valve
  valveState = setTo ? 1 : 0;

  // reset accumulated water flow
  if(valveState == false)
    pulses = 0;

  // LEDY replicates valve state
  digitalWrite(LEDY, setTo);
  return valveState;
}

uint8_t readMyID(){
  return (!digitalRead(IDPIN_0))*1+(!digitalRead(IDPIN_1))*2+(!digitalRead(IDPIN_2))*4+(!digitalRead(IDPIN_3))*8;
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

  // init ID read pins
  pinMode(IDPIN_0, INPUT_PULLUP);
  pinMode(IDPIN_1, INPUT_PULLUP);
  pinMode(IDPIN_2, INPUT_PULLUP);
  pinMode(IDPIN_3, INPUT_PULLUP);

  // "power-on" light sequence
  digitalWrite(LEDY, LOW);
  digitalWrite(LEDR, HIGH);
  delay(500);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, HIGH);

  // begin serial communication
  Serial.begin(BAUD_RATE);

  // begin mesh communication
  mesh.setNodeID(readMyID()); // do manually
  Serial.print(F("Booted... ID is ")); Serial.println(readMyID());
  Serial.print(F("Connecting to the mesh...\nOutput of mesh.begin(): "));
  bool success = mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT);
  Serial.println(success);

  // "connected" light sequence
  digitalWrite(LEDY, LOW);
  if(success){
    digitalWrite(LEDR, HIGH);
    delay(250);
    digitalWrite(LEDR, LOW);
    delay(250);
    digitalWrite(LEDR, HIGH);
    delay(250);
  }
  digitalWrite(LEDR, LOW);

  // allow children to connect
  mesh.setChild(true);

  // init timer for serial communication printing
  Timer1.initialize(TIMER1_PERIOD);
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

  // handle button presses--send 'r' type message
  // NOT FINAL CODE
  if(hadButtonPress){
    // tell current flow rate
    float beginLiters, flowrate;//endLiters, flowrate;
    beginLiters = pulses/7.5/60.0;                      // is initial amount of liters
    delay(RATE_MEASURING_PERIOD);                                        // wait 5 seconds
    //endLiters = pulses/7.5/60.0;                      // is end amount of liters
    flowrate = (pulses/7.5/60.0-beginLiters)/5*15.8503; // convert liters/sec to GPM
    safeMeshWrite(MASTER_ADDRESS, &flowrate, 'r', sizeof(flowrate), DEFAULT_SEND_TRIES);

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
    Serial.print(F("From [logical address]: ")); Serial.println(header.from_node);
    Serial.print(F("From [converted to nodeID]: ")); Serial.println(mesh.getNodeID(header.from_node));

    switch(header.type){
    case SET_VALVE_H:
      // Valve command, on or off; type is bool
      //  set the value and send return message
      bool onOrOff;
      network.read(header, &onOrOff, sizeof(onOrOff));
      setValve(onOrOff);
      safeMeshWrite(MASTER_ADDRESS, &valveState, SEND_VALVE_H, sizeof(valveState), DEFAULT_SEND_TRIES);
      break;
      
    case INFO_REQUEST_H:    
      // Request; type is char
      char typeOfData;
      network.read(header, &typeOfData, sizeof(typeOfData));
      
      switch( typeOfData ){
      case GET_VALVE_P:
        // tell current valve state
        safeMeshWrite(MASTER_ADDRESS, &valveState, SEND_VALVE_H, sizeof(valveState), DEFAULT_SEND_TRIES);
        break;
      case GET_FLOW_RATE_P:
        // tell current flow rate
        float beginLiters, flowrate;//endLiters, flowrate;
        beginLiters = pulses/7.5/60.0;                      // is initial amount of liters
        delay(RATE_MEASURING_PERIOD);                       // wait 5 seconds (or whatever RATE_MEASURING_PERIOD in settings.h is)
        //endLiters = pulses/7.5/60.0;                      // is end amount of liters
        flowrate = (pulses/7.5/60.0-beginLiters)/5*15.8503; // convert liters/sec to GPM
        safeMeshWrite(MASTER_ADDRESS, &flowrate, SEND_FLOW_RATE_H, sizeof(flowrate), DEFAULT_SEND_TRIES);
        break;
      case GET_NODE_STATUS_P:
        // return status
        safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
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

