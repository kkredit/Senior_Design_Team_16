/* 
 * node.ino
 * 
 * This sketch is for nodes in the GardeNet system. Functions include:
 *    - Establishing and maintaining a mesh network
 *    - Controlling up to four valves
 *    - Controlling up one flow meter
 *    - Tracking several variables with regards to its status
 *    - Relaying information to the master
 * 
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last Modified: 4/3/16
 */



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////  Preprocessor   ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// includes
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <EEPROM.h>
#include <TimerOne.h>
#include "C:/Users/kevin/Documents/Senior_Design_Team_16/RadioWork/Shared/SharedDefinitions.h"

// pins
#define BUTTON    2
#define RF24_IRQ  3 // currently unused
#define RESET_PIN 4
#define VALVE_1   5
#define VALVE_2   6
#define VALVE_3   7
#define VALVE_4   8
#define RF24_CE   9
#define RF24_CS   10
//RF24_MOSI         11  //predifined
//RF24_MISO         12  //predifined
//RF24_SCK          13  //predifined
#define FRATE     A0
#define LED       A1
#define RESET_GND A2
#define IDPIN_0   A3
#define IDPIN_1   A4
#define IDPIN_2   A5
#define IDPIN_3   A6  //note: analog input only, no internal pullup--has external pullup
#define VIN_REF   A7  //note: analog input only, no internal pullup--no external pullup



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////     Globals     ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// mesh network
RF24 radio(RF24_CE, RF24_CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

// flow sensor
volatile uint8_t lastflowpinstate;
volatile uint32_t lastflowratetimer = 0;
volatile float flowRates[FLOW_RATE_SAMPLE_SIZE] = {0};
volatile uint16_t flowRatePos = 0;

// flags
volatile bool hadButtonPress = false;
volatile bool updateNodeStatusFlag = false;

// other
Node_Status myStatus;
uint8_t statusCounter = 0;
const uint8_t VALVE_PINS[5] = {255, VALVE_1, VALVE_2, VALVE_3, VALVE_4};



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////     ISRs        ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/* 
 * handleButtonISR()
 *
 * Interrupt service routine (ISR) called when the button is pressed; sets a flag and exits
 * 
 * @preconditions: button is tied to interrupt; button is pressed
 * @postconditions: hadButtonPress flag is set
 */ 
void handleButtonISR(){
  // gets rid of startup false positive by ignoring for a few seconds after startup
  if(statusCounter > 0){
    hadButtonPress = true; 
    //Serial.println(F("Detected buttonpress"));
  }
}


/* 
 * updateNodeStatusISR()
 *
 * ISR called when timer1 times out; sets a flag and exits
 * 
 * @preconditions: timer interrupt must be enabled
 * @postconditions: updateNodeStatus flag is set
 */ 
void updateNodeStatusISR(){
  updateNodeStatusFlag = true;
}


/* 
 * SIGNAL()
 *
 * ISR called every 1 ms; checks if flow meter pin has changed states, updates currentFlowRate and
 * accumulatedFlow
 * See https://github.com/adafruit/Adafruit-Flow-Meter, which provided the basis for this function
 * 
 * @preconditions: interrupt is enabled
 * @postconditions: myStatus.currentFlowRate and myStatus.accumulatedFlow up to date
 * 
 * @param TIMER0_COMPA_vect: not sure what this is, got it from the sample code
 */
SIGNAL(TIMER0_COMPA_vect){
  uint8_t x = digitalRead(FRATE);

  // if no flow since last check
  if (x == lastflowpinstate){
    lastflowratetimer++;
  }

  myStatus.currentFlowRate -= flowRates[flowRatePos]/FLOW_RATE_SAMPLE_SIZE;
  flowRates[flowRatePos] = 60.0*2.0*0.264172/lastflowratetimer;
  myStatus.currentFlowRate += flowRates[flowRatePos]/FLOW_RATE_SAMPLE_SIZE;
  flowRatePos = (flowRatePos + 1) % FLOW_RATE_SAMPLE_SIZE;
  myStatus.currentFlowRate = max(0, myStatus.currentFlowRate); // can't be negative

  if(myStatus.currentFlowRate > MAX_MEASUREABLE_GPM){
    myStatus.maxedOutFlowMeter = true;
  }
  
  // else, have flow
  if (x != lastflowpinstate){
    lastflowpinstate = x;
    lastflowratetimer = 0;
    
    // if low to high transition, add to accumulatedFlow
    if(x == HIGH){
      //low to high transition!
      //pulses++;
      myStatus.accumulatedFlow += 0.000528344; // add 2 mL (converted to gal) to accumulatedFlow
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Helper Functions///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* 
 * initPins()
 *
 * Initializes all pins as input/input_pullup/output
 * 
 * @preconditions: board just booted
 * @postconditions: pins are configured
 */ 
void initPins(){
  // BUTTON
  pinMode(BUTTON, INPUT_PULLUP);

  // RF24 IRQ -- NOTE: unused
  //pinMode(RF24_IRQ, INPUT);
  
  // RESET_PIN -- set to low immediately to discharge capacitor
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);

  // VALVES -- leave as inputs for now (is default, valves will be off)
  
  // FRATE -- also init variables
  pinMode(FRATE, INPUT);
  lastflowpinstate = digitalRead(FRATE);
  // setup interrupt
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  // LED
  pinMode(LED, OUTPUT);
  
  // RESET_GND -- don't init until end of setup() to prevent resetting cycle
  
  // ID pins
  pinMode(IDPIN_0, INPUT_PULLUP);
  pinMode(IDPIN_1, INPUT_PULLUP);
  pinMode(IDPIN_2, INPUT_PULLUP);
  pinMode(IDPIN_3, INPUT_PULLUP);
  
  // VIN
  pinMode(VIN_REF, INPUT);
}


/* 
 * readMyID()
 *
 * Uses ID_PINs 0-3 to determine its NodeID
 * 
 * @preconditions: the pins must be configured as INPUT_PULLUPs
 * @postconditions: none
 * 
 * @return uint8_t id: the node's ID, range [1,16] in this implementation
 */ 
uint8_t readMyID(){
  uint8_t id = (!digitalRead(IDPIN_0))*1+(!digitalRead(IDPIN_1))*2
              +(!(analogRead(IDPIN_2)>300))*4+(!(analogRead(IDPIN_3)>300))*8;
              //+(!(analogRead(IDPIN_4)>300))*16+(!(analogRead(IDPIN_5)>300))*32;
  if(id==0) id = 16;
  return id;
}


/* 
 * hardReset()
 *
 * "Presses" the reset button by turning on the reset circuit
 * 
 * @preconditions: the pins are configured appropriately and the RESET_GND is set to LOW
 * @postconditions: the board resets
 */ 
void hardReset(){
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(1000);
  // arduino resets here
}


/* 
 * refreshReset()
 *
 * Discharges the capacitor in the resetting circuit. Must be called once every 5 minutes in order
 * to not reset the board.
 * 
 * @preconditions: pins configured
 * @postconditions: reset circuit is discharged, and will not reset for at least 5 minutes
 */ 
void refreshReset(){
  pinMode(RESET_PIN, OUTPUT);  
  digitalWrite(RESET_PIN, LOW);
  delay(50); // 95% charged: 3*tau = 3*RC = 3*200*100*10^-6 = 60 ms
  pinMode(RESET_PIN, INPUT);
}


/* 
 * setLED()
 *
 * Sets the LED accoring to the given pattern. Maximum latency of 1 second
 * 
 * @preconditions: pins configured
 * @postconditions: none
 * 
 * @param uint8_t setTo: the pattern to flash the LED to; numbers are #defined codes
 */
void setLED(uint8_t setTo){
  switch(setTo){
  case LED_OFF:
    digitalWrite(LED, LOW);
    break;
  case LED_ON:
    analogWrite(LED, LED_BRIGHTNESS);
    break;
  case TURN_ON_SEQUENCE:
    analogWrite(LED, LED_BRIGHTNESS);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
    analogWrite(LED, LED_BRIGHTNESS);
    delay(250);
    digitalWrite(LED, LOW);
    break;
  case CONNECTED_SEQUENCE:
    analogWrite(LED, LED_BRIGHTNESS);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
    analogWrite(LED, LED_BRIGHTNESS);
    delay(250);
    digitalWrite(LED, LOW);
    break;
  case DISCONNECTED_SEQUENCE:
    analogWrite(LED, LED_BRIGHTNESS);
    break;
  case SPECIAL_BOOT_SEQUENCE:
    int i;
    for(i=0; i<4; i++){
      analogWrite(LED, LED_BRIGHTNESS);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
    }
    break;
  case GO_TO_SLEEP_SEQUENCE:
    analogWrite(LED, LED_BRIGHTNESS);
    delay(125);
    digitalWrite(LED, LOW);
    delay(125);
    analogWrite(LED, LED_BRIGHTNESS);
    delay(500);
    digitalWrite(LED, LOW);
    break;
  case AWAKE_SEQUENCE:
    analogWrite(LED, LED_BRIGHTNESS);
    delay(500);
    digitalWrite(LED, LOW);
    delay(125);
    analogWrite(LED, LED_BRIGHTNESS);
    delay(125);
    digitalWrite(LED, LOW);
    delay(125);
    analogWrite(LED, LED_BRIGHTNESS);
    delay(125);
    digitalWrite(LED, LOW);
    break;
  default:
    break;
  }
}


/* 
 * setValve()
 *
 * Sets the given valve(s) to the given position (on or off).
 * 
 * @preconditions: pins configured
 * @postconditions: said valve(s) is opened or closed
 * 
 * @param uint8_t whichValve: the valve to control; range is [1-5], 1-4 is for single given valve,
 *                            5 is for all
 * @param bool setTo: ON or true means to open, OFF or false means to close
 * 
 * @return int8_t setTo: if the valve that was controlled is not connected, returns -1,
 *                       else returns the input values, setTo
 */ 
int8_t setValve(uint8_t whichValve, bool setTo){
  // check if valve is connected, then open/close and set state
  if(whichValve >= 1 && whichValve <= 4){
    if(myStatus.valveStates[whichValve].isConnected == false) return NO_VALVE_ERROR;
    else{
      digitalWrite(VALVE_PINS[whichValve], setTo);
      myStatus.valveStates[whichValve].state = setTo;
    }
  }
  else if(whichValve == ALL_VALVES){
    uint8_t valve;
    for(valve=1; valve<=4; valve++){
      if(myStatus.valveStates[valve].isConnected){
        digitalWrite(VALVE_PINS[valve], setTo);
        myStatus.valveStates[valve].state = setTo;
      }
    }
  }
  else{
    return NO_VALVE_ERROR;
  }
  myStatus.numOpenValves = myStatus.valveStates[1].state + myStatus.valveStates[2].state
                          + myStatus.valveStates[3].state + myStatus.valveStates[4].state;
  uint8_t result = 0;
  if(setTo) result = 1;
  return result;
}


/* 
 * safeMeshWrite()
 *
 * Performs mesh.writes, but adds reliability features, hence "safe". If mesh.write doesn't work, 
 * then checks connection; if connected, retries sending, else tries to reconnect. Tries the send
 * the message up to DEFAULT_SEND_TRIES times. Maximum latency of 5 seconds.
 * 
 * @preconditions: mesh is connected
 * @postconditions: message is sent, or else myStatus.meshStatus is updated (likely to DISCONNECTED)
 * 
 * @param uint8_t destination: the mesh address of the recipient
 * @param void* payload: the address of the data to send
 * @param char header: the message type that you are sending
 * @param uint8_t datasize: the size of the data to send, in bytes
 * @param uint8_t timesToTry: the number of remaining times to try to send
 * 
 * @return bool: true means send success, false means send failure
 */ 
bool safeMeshWrite(uint8_t destination, void* payload, char header, uint8_t datasize, uint8_t timesToTry){  
  // perform write
  if (!mesh.write(destination, payload, header, datasize)) {
    // if a write fails, check connectivity to the mesh network
    Serial.print(F("Send fail... "));
    if (!mesh.checkConnection()){
      //refresh the network address
      Serial.println(F("renewing address... "));
      if (!mesh.renewAddress(RENEWAL_TIMEOUT)){
        // if failed, connection is down
        Serial.println(F("MESH CONNECTION DOWN"));
        myStatus.meshState = MESH_DISCONNECTED;
        setValve(ALL_VALVES, OFF);
        setLED(DISCONNECTED_SEQUENCE);
        return false;
      }
      else{
        // if succeeded, are reconnected and try again
        myStatus.meshState = MESH_CONNECTED;
        safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
        setLED(CONNECTED_SEQUENCE);
        if(timesToTry){
          // more tries allowed; try again
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
  }
  else {
    // write succeeded
    Serial.print(F("Send of type ")); Serial.print(header); Serial.println(F(" success"));
    return true;
  }
}


/* 
 * initStatus()
 *
 * Initializes the Node_Status struct myStatus.
 * 
 * @preconditions: pins are configured
 * @postconditions: myStatus is configured
 */ 
void initStatus(){  
  // isAwake
  myStatus.isAwake = true;
  
  // storedVIN
  EEPROM.get(VIN_EEPROM_ADDR, myStatus.storedVIN);

  // voltageState
  if(analogRead(VIN_REF) > myStatus.storedVIN*(1+OK_VIN_RANGE))
    myStatus.voltageState = HIGH_VOLTAGE;
  else if (analogRead(VIN_REF) < myStatus.storedVIN*(1-OK_VIN_RANGE))
    myStatus.voltageState = LOW_VOLTAGE;
  else
    myStatus.voltageState = GOOD_VOLTAGE;
    
  // hasFlowRateMeter
  myStatus.hasFlowRateMeter = digitalRead(FRATE);

  // currentFlowRate
  myStatus.currentFlowRate = 0;
  //updateFlowRate();

  // flowState
  if(myStatus.hasFlowRateMeter == false) myStatus.flowState = HAS_NO_METER;
  else{
    // if water flowing, bad
    if(myStatus.currentFlowRate > MIN_MEASUREABLE_GPM){
      myStatus.flowState = STUCK_AT_ON;
    }
    // if no water flowing, good
    else{
      myStatus.flowState = NO_FLOW_GOOD;
    }
  }

  // accumulatedFlow
  EEPROM.get(ACC_FLOW_EEPROM_ADDR, myStatus.accumulatedFlow);

  // maxedOutFlowMeter
  myStatus.maxedOutFlowMeter = false;

  // numConnectedValves
  myStatus.numConnectedValves = 0; // incremented as appropriate below

  // numOpenValves
  myStatus.numOpenValves = 0;

  // valves
  uint8_t valve;
  for(valve=1; valve<=4; valve++){
    pinMode(VALVE_PINS[valve], INPUT_PULLUP);
    myStatus.valveStates[valve].isConnected = !digitalRead(VALVE_PINS[valve]);
    pinMode(VALVE_PINS[valve], OUTPUT);
    if(myStatus.valveStates[valve].isConnected) myStatus.numConnectedValves++;
    digitalWrite(VALVE_PINS[valve], LOW);
    myStatus.valveStates[valve].state = OFF;
  }

  // meshState
  myStatus.meshState = MESH_DISCONNECTED;

  // nodeID
  myStatus.nodeID = readMyID();

  // nodeMeshAddress
  myStatus.nodeMeshAddress = -1;
}


/* 
 * updateNodeStatus()
 *
 * Performs diagnostics on the input voltage, water flow rate, and mesh connection
 * 
 * @preconditions: myStatus is initialized
 * @postconditions: myStatus is updated
 */
void updateNodeStatus(){

  //////////// CHECK INPUT VOLTAGE ////////////

  // check input voltage if no valves are open (open valves descrease VIN, could give false error)
  if(myStatus.numOpenValves == 0){
    if(analogRead(VIN_REF) > myStatus.storedVIN*(1+OK_VIN_RANGE))
      myStatus.voltageState = HIGH_VOLTAGE;
    else if (analogRead(VIN_REF) < myStatus.storedVIN*(1-OK_VIN_RANGE))
      myStatus.voltageState = LOW_VOLTAGE;
    else //if(myStatus.voltageState == HIGH_VOLTAGE || myStatus.voltageState == LOW_VOLTAGE)
      myStatus.voltageState = GOOD_VOLTAGE;
  }


  //////////// CHECK FLOW RATE ////////////

  // get flow rate, and recheck for false negative regarding having a connected meter
  if(myStatus.hasFlowRateMeter == false && myStatus.accumulatedFlow > 0){
    myStatus.hasFlowRateMeter = true;
  }
  
  // if has flow rate meter
  if(myStatus.hasFlowRateMeter){

    // check for maxed out flow rate measurement
    if(myStatus.currentFlowRate > MAX_MEASUREABLE_GPM){
      myStatus.maxedOutFlowMeter = true;
    }

    // store current value of accumulatedFlow in EEPROM in case of crash/reset
    EEPROM.put(ACC_FLOW_EEPROM_ADDR, myStatus.accumulatedFlow);
    
    // if valves are open...
    if(myStatus.numOpenValves > 0){
      // if water flowing, good
      if(myStatus.currentFlowRate > MIN_MEASUREABLE_GPM){
        myStatus.flowState = FLOWING_GOOD;
      }
      // if no water flowing, bad
      else{
        myStatus.flowState = STUCK_AT_OFF;
      }
    }

    // if valves are closed...
    else{
      // if water flowing, bad
      if(myStatus.currentFlowRate > MIN_MEASUREABLE_GPM){
        myStatus.flowState = STUCK_AT_ON;
      }
      // if no water flowing, good
      else{
        myStatus.flowState = NO_FLOW_GOOD;
      }
    }
    
    // ignore other errors; too hard to accurately diagnose
  }
      

  //////////// CHECK MESH CONNECTION ////////////

  // check connection and print status
  if(!mesh.checkConnection()){
    // unconnected, try to reconnect
    Serial.println(F("\n[Mesh not connected, trying to reconnect...]"));
    if(!mesh.renewAddress(RENEWAL_TIMEOUT)){
      // reconnection effort failed, disconnected
      setValve(ALL_VALVES, OFF);
      Serial.println(F("[MESH NOT CONNECTED]"));
      setLED(DISCONNECTED_SEQUENCE);
      myStatus.meshState = MESH_DISCONNECTED;
      myStatus.nodeMeshAddress = -1;
    }
    else{
      // reconnection effort succeeded, connected
      Serial.println(F("[Mesh reconnected]"));
      myStatus.meshState = MESH_CONNECTED;
      safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
      setLED(CONNECTED_SEQUENCE);
    }
  }
  else{
    // connected
    myStatus.meshState = MESH_CONNECTED;
    //safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
    setLED(LED_OFF);
  }

  // update mesh address in case it changed
  int16_t tempvar = mesh.getAddress(myStatus.nodeID);
  // this sometimes fails, but does not mean disconnected; simply check to see it worked
  if(tempvar > 0) myStatus.nodeMeshAddress = tempvar;
}


/* 
 * printNodeStatus()
 *
 * Prints myStatus to Serial port in a user-friendly way.
 * 
 * @preconditions: myStatus is initialized, Serial port is active
 * @postconditions: none
 */ 
void printNodeStatus(){
  // print number of times executed
  Serial.println(F("")); Serial.println(statusCounter++);

  if(myStatus.isAwake == false) Serial.println(F("NODE IS IN STANDBY"));

  Serial.print(F("Input voltage     : "));
  Serial.print(analogRead(VIN_REF)*3*4.8/1023.0); Serial.print(F(" V  : "));
  if(myStatus.voltageState == GOOD_VOLTAGE) Serial.println(F("good"));
  else if(myStatus.voltageState == HIGH_VOLTAGE) Serial.println(F("HIGH INPUT VOLTAGE!"));
  else if(myStatus.voltageState == LOW_VOLTAGE) Serial.println(F("LOW INPUT VOLTAGE!"));

  // if has flow rate meter, tell current rate and accumulated flow
  if(myStatus.hasFlowRateMeter){
    Serial.print(F("Current flow rate : ")); Serial.print(myStatus.currentFlowRate); 
    Serial.print(F(" GPM : "));
    if(myStatus.flowState == NO_FLOW_GOOD) Serial.println(F("good"));
    else if(myStatus.flowState == FLOWING_GOOD) Serial.println(F("good"));
    else if(myStatus.flowState == STUCK_AT_OFF) Serial.println(F("A VALVE IS OPEN BUT HAVE NO FLOW!"));
    else if(myStatus.flowState == STUCK_AT_ON) Serial.println(F("A VALVE IS LEAKING!"));
    Serial.print(F("Accumulated flow  : ")); Serial.print(myStatus.accumulatedFlow); 
    Serial.print(F(" gal"));
    myStatus.maxedOutFlowMeter ? Serial.println(F("*")) : Serial.println(F(""));
  }
  // else tell that has no meter
  else Serial.println(F("No flow meter"));

  // if valve is connected, tell state
  uint8_t valve;
  for(valve=1; valve<=4; valve++){
    if(myStatus.valveStates[valve].isConnected){
      Serial.print(F("Valve ")); Serial.print(valve); Serial.print(F(" is        : ")); 
      myStatus.valveStates[valve].state ? Serial.println(F("OPEN")) : Serial.println(F("closed"));
    }
  }

  // tell mesh state
  Serial.print(F("Node ID, address  : ")); Serial.print(myStatus.nodeID); Serial.print(F(", ")); 
  Serial.print(myStatus.nodeMeshAddress); Serial.print(F("     : "));
  if(myStatus.meshState == MESH_CONNECTED) Serial.println(F("good"));
  else if(myStatus.meshState == MESH_DISCONNECTED) Serial.println(F("DISCONNECTED!"));
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////     Setup       ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/* 
 * setup()
 *
 * The beginning of execution when the board boots; uses helper functions to initialize the pins, 
 * initialized myStatus, begin mesh communication, and more; see internal comments for more detail.
 * 
 * @preconditions: board just booted
 * @postconditions: all initializaion is complete, ready for normal operation
 */ 
void setup(){
  
  // check if recovering from self-inflicted reboot; if so, report to master
  pinMode(RESET_PIN, INPUT);
  bool resetFlag = digitalRead(RESET_PIN);
  // note: can't send message until mesh is connected...

  // initaialize all the pins
  initPins();
  
  // begin serial communication
  Serial.begin(BAUD_RATE);
  
  // check if button is being pressed; if so, do special startup
  if(digitalRead(BUTTON) == 0){
    setLED(SPECIAL_BOOT_SEQUENCE);
    Serial.println(F("\n\n////Special boot sequence////"));

    // store VIN value in EEPROM
    Serial.print(F("Reading voltage source: ")); Serial.print(analogRead(VIN_REF)*3*4.8/1023.0); 
    Serial.println(F("V\n"));
    myStatus.storedVIN = analogRead(VIN_REF); // shave off the last two bits to fit in one byte
    EEPROM.put(VIN_EEPROM_ADDR, myStatus.storedVIN);

    // set toldToReset to false in EEPROM
    EEPROM.put(RESET_EEPROM_ADDR, false);

    // set accumulatedFlow to 0 in EEPROM
    myStatus.accumulatedFlow = 0;
    EEPROM.put(ACC_FLOW_EEPROM_ADDR, myStatus.accumulatedFlow);
  }

  // "power-on" light sequence
  setLED(TURN_ON_SEQUENCE);

  // initialize myStatus
  initStatus();

  // print ID and number and location of connected valves and flow meters
  Serial.print(F("\n/////////Booted//////////\nNodeID: ")); Serial.println(readMyID());
  Serial.print(F("Voltage source: ")); Serial.print(analogRead(VIN_REF)*3*4.8/1023.0); Serial.print(F("V, "));
  if(analogRead(VIN_REF) > myStatus.storedVIN*(1+OK_VIN_RANGE)){
    Serial.println(F("HI VOLTAGE WARNING"));
  }
  else if (analogRead(VIN_REF) < myStatus.storedVIN*(1-OK_VIN_RANGE)){
    Serial.println(F("LOW VOLTAGE WARNING"));
  }
  else{
    Serial.println(F("within expected range"));
  }
  uint8_t valve;
  for(valve=1; valve<=4; valve++){
    Serial.print(F("Valve ")); Serial.print(valve); Serial.print(F(": "));
    if(myStatus.valveStates[valve].isConnected){
      Serial.println(F("CONNECTED"));
    }
    else{
      Serial.println(F("disconnected"));
    }
  }
  
  // should use myStatus.hasFlowRateMeter, but in case of false negative, check again
  Serial.print(F("FRate:   "));
  digitalRead(FRATE) ? Serial.println(F("CONNECTED\n")) : Serial.println(F("disconnected\n"));

  // set NodeID and prep for mesh.begin()
  mesh.setNodeID(myStatus.nodeID); // do manually
  setLED(DISCONNECTED_SEQUENCE);

  // while unconnected, try 5 times consecutively every 15 minutes indefinitely
  bool success = false;
  while(!success){
    uint8_t attempt;
    for(attempt=1; attempt<=CONNECTION_TRIES; attempt++){
      Serial.print(F("Connecting to the mesh (attempt ")); Serial.print(attempt); Serial.print(F(")... "));
      success = mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT);
      if(success){
        Serial.println(F("CONNECTED"));
        break;
      }
      else Serial.println(F("FAILED"));
    }
    if(!success){
      Serial.println(F("Trying again in 15 minutes. Else powerdown or reset.\n"));
      delay(DISCONNECTED_SLEEP);
    }
  }

  // "connected" light sequence
  setLED(CONNECTED_SEQUENCE);
  myStatus.meshState = MESH_CONNECTED;

  // report the reset to the master
  //    reset flag is read from RESET_PIN, meaning that this was either a timeout reset or software reset 
  //    (as opposed to hitting the reset button)
  if(resetFlag){
    bool toldToReset;
    EEPROM.get(RESET_EEPROM_ADDR, toldToReset);
    safeMeshWrite(MASTER_ADDRESS, &toldToReset, SEND_JUST_RESET_H, sizeof(toldToReset), DEFAULT_SEND_TRIES);
  }
  EEPROM.put(RESET_EEPROM_ADDR, false);

  // allow children to connect
  mesh.setChild(true);

  // init timer for regular system checks
  Timer1.initialize(TIMER1_PERIOD);
  Timer1.attachInterrupt(updateNodeStatusISR);

  // attach interrupt to button
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButtonISR, FALLING);

  // enable self-resetting ability
  delay(50);  // allow capacitor to discharge if was previously charged before enabling autoreset again
              // 95% = 3*tau = 3*RC = 3*200*100*10^-6 = 60ms -- but never gets fully charged, and has
              //    been dicharging during previous setup, so 50ms is sufficient
  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_GND, OUTPUT);
  digitalWrite(RESET_GND, LOW);

  // send status so that master knows I exist
  Serial.print(F("Registering myself with the gateway\n"));
  safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////     Loop        ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/* 
 * loop()
 *
 * Run indefinitely after setup() completes; contains the core node features. Uses helper functions
 * to control the reset circuit, maintain the mesh, update myStatus, handle buttonpresses, and 
 * handle communication with the master.
 * 
 * @preconditions: asetup() has successfully completed
 * @postconditions: none--runs forever
 */ 
void loop() {  
  
  // refresh the reset
  refreshReset();

  // refresh the mesh
  mesh.update();

  // update node status if necessary
  if(updateNodeStatusFlag){
    updateNodeStatus();
    printNodeStatus();

    // reset the flag
    updateNodeStatusFlag = false;
  }

  // if had buttonpress, toggle between awake and asleep
  if(hadButtonPress){
    if(statusCounter > 0){
      // toggle states between asleep and awake
      myStatus.isAwake = !myStatus.isAwake;
  
      // if asleep, close valves
      if(myStatus.isAwake == false){
        setValve(ALL_VALVES, OFF);
        setLED(GO_TO_SLEEP_SEQUENCE);
      }
      else{
        setLED(AWAKE_SEQUENCE);
      }
  
      // let the master know
      safeMeshWrite(MASTER_ADDRESS, &myStatus.isAwake, SEND_NODE_SLEEP_H, sizeof(myStatus.isAwake), DEFAULT_SEND_TRIES);
      
      // reset flag
      hadButtonPress = false;
    }
  }
    

  // read in messages
  while(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    Serial.print(F("Received ")); Serial.print(char(header.type)); Serial.println(F(" type message."));
    Serial.print(F("From: ")); Serial.println(mesh.getNodeID(header.from_node));

    switch(header.type){
    case SET_VALVE_H:
      Valve_Command vc;
      network.read(header, &vc, sizeof(vc));
      Serial.print(F("Command is to turn valve ")); Serial.print(vc.whichValve); Serial.print(F(" ")); vc.onOrOff ? Serial.println(F("ON")) : Serial.println(F("OFF"));
      int8_t result;

      Valve_Response vr;
      vr.nodeIsAwake = myStatus.isAwake;
      vr.whichValve = vc.whichValve;
      vr.isConnected = myStatus.valveStates[vc.whichValve].isConnected;
      vr.commandedOnOrOff = vc.onOrOff;
      vr.actualState = myStatus.valveStates[vc.whichValve].state;
      vr.timeToLive = vc.timeToLive;
      
      // if node is asleep, throw error
      if(myStatus.isAwake == false){
        Serial.println(F("But I am asleep, so I am ignoring it."));
      }
      // else execute instruction and return result
      else{
        //vr.actualState = setValve(vc.whichValve, vc.onOrOff);
        setValve(vc.whichValve, vc.onOrOff);
        vr.actualState = myStatus.valveStates[vc.whichValve].state;  
        Serial.print("valve is now "); Serial.println(vr.actualState); 
      }
      //safeMeshWrite(MASTER_ADDRESS, &vr, SEND_VALVE_H, sizeof(vr), DEFAULT_SEND_TRIES);
      safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
      break;

    case GET_NODE_STATUS_H:
      char placeholder1;
      network.read(header, &placeholder1, sizeof(placeholder1));
      Serial.print(F("Command is to tell my status: ")); printNodeStatus();
      safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
      break;
    
    case FORCE_RESET_H:
      char placeholder2;
      network.read(header, &placeholder2, sizeof(placeholder2));
      Serial.println(F("Command is to reset--RESETTING"));
      EEPROM.put(RESET_EEPROM_ADDR, true);
      hardReset();
      break;

    case IS_NEW_DAY_H:
      char placeholder3;
      network.read(header, &placeholder3, sizeof(placeholder3));
      Serial.println(F("It is a new day!"));
      myStatus.accumulatedFlow = 0;
      EEPROM.put(ACC_FLOW_EEPROM_ADDR, myStatus.accumulatedFlow);
      myStatus.maxedOutFlowMeter = false;
      myStatus.isAwake = true;
      setLED(AWAKE_SEQUENCE);
      //safeMeshWrite(MASTER_ADDRESS, &myStatus.isAwake, SEND_NEW_DAY_H, sizeof(myStatus.isAwake), DEFAULT_SEND_TRIES);
      safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
      break;

    case CONNECTION_TEST_H:
      // read in message to clear buffer, but do nothing
      char placeholder4;
      network.read(header, &placeholder4, sizeof(placeholder4));
      break;
      
    default:
      Serial.println(F("Unknown message type."));
      break;
    }
  }
}

