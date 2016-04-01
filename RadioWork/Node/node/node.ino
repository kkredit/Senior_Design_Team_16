/**
 * node.ino
 * 
 * This sketch is for nodes in the GardeNet system. Functions include:
 *    - Establishing and maintaining a mesh network
 *    - Controlling up to four valves
 *    - Controlling up one flow meter
 *    - Relaying information to the master
 *    
 * Written by Kevin Kredit for Engr 340 at Calvin College, 2016
 */


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////  Preprocessor   //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// includes
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <EEPROM.h>
#include <TimerOne.h>
#include "C:\\Users\\kevin\\Documents\\Senior_Design_Team_16\\RadioWork\\Shared\\SharedDefinitions.h"

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
#define IDPIN_3   A6  //note: analog input only--has external pullup
#define VIN_REF   A7  //note: analog input only--no external pullup


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Globals     //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// mesh network
RF24 radio(RF24_CE, RF24_CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

// flow sensor
volatile uint16_t pulses = 0; // count how many pulses
volatile uint8_t lastflowpinstate; // track the state of the pulse pin
volatile uint32_t lastflowratetimer = 0; // you can try to keep time of how long it is between pulses
volatile float flowrate; // and use that to calculate a flow rate
//bool changedPulseFlag = false; // interrupt is called once a millisecond, looks for any pulses from the sensor

// structs
//struct Valve_Status{
//  bool isConnected;
//  bool state;
//};

//struct Node_Status{
//  uint8_t storedVIN;
//  uint8_t voltageState;
//  bool hasFlowRateMeter;
//  float currentFlowRate;
//  uint8_t flowState;
//  uint8_t numConnectedValves;
//  Valve_Status valveState1;
//  Valve_Status valveState2;
//  Valve_Status valveState3;
//  Valve_Status valveState4;
//  uint8_t meshState;
//  uint8_t nodeID;
//  int16_t nodeMeshAddress;
//};

// flags
volatile bool hadButtonPress = false;
volatile bool updateNodeStatusFlag = false;

// other
//int8_t valveState = 0;
//uint8_t myStatus = NODE_OK;
Node_Status myStatus;
uint8_t statusCounter = 0;


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     ISRs        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/*
 * handleButtonISR()
 * 
 * Triggered by button press on pin BUTTON
 * Sets flag alterting main loop of buttonpress
 */
void handleButtonISR(){
  if(statusCounter > 1){   // gets rid of startup false positive by ignoring for 2 seconds after startup
    hadButtonPress = true; 
    //Serial.println(F("Detected buttonpress"));
  }
}

/*
 * getNodeStatusISR()
 * 
 * Triggered by timer
 * Sets flag alerting main loop that it's time for diagnostics
 */
void getNodeStatusISR(){
  updateNodeStatusFlag = true;
}

//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Helper Functions//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*
 * getNodeStatus()
 * 
 * Performs diagnostics on the each part of the system:
 *  - mesh connection
 *  - input voltage
 *  - water flow rate
 *  - current valve states  
 */
void getNodeStatus(){

  //////////// CHECK INPUT VOLTAGE ////////////

      // check input voltage only if no valves are open (open valves descrease VIN, could give false error)
      if(myStatus.numOpenValves == 0){
        if(analogRead(VIN_REF)>>2 > myStatus.storedVIN*(1+OK_VIN_RANGE))
          myStatus.voltageState = HIGH_VOLTAGE;
        else if (analogRead(VIN_REF)>>2 < myStatus.storedVIN*(1-OK_VIN_RANGE))
          myStatus.voltageState = LOW_VOLTAGE;
        else //if(myStatus.voltageState == HIGH_VOLTAGE || myStatus.voltageState == LOW_VOLTAGE)
          myStatus.voltageState = GOOD_VOLTAGE;
      }


  //////////// CHECK FLOW RATE ////////////

      // get flow rate, and recheck for false negative regarding having a connected meter
      updateFlowRate();
      
      // if has flow rate meter
      if(myStatus.hasFlowRateMeter){
        // if valves are open...
        if(myStatus.numOpenValves > 0){
          // if water flowing, good
          if(myStatus.currentFlowRate > MIN_MEASUREABLE_GPM)
            myStatus.flowState = FLOWING_GOOD;
          // if no water flowing, bad
          else
            myStatus.flowState = STUCK_AT_OFF;
        }
  
        // if valves are closed...
        else{
          // if water flowing, bad
          if(myStatus.currentFlowRate > MIN_MEASUREABLE_GPM)
            myStatus.flowState = STUCK_AT_ON;
          // if no water flowing, good
          else
            myStatus.flowState = NO_FLOW_GOOD;
        }
        
        // ignore other errors; too hard to accurately diagnose
      }
      
  
  //////////// CHECK VALVE STATES ////////////

      // nothing to do

  //////////// CHECK MESH CONNECTION ////////////

      // check connection and print status
      if(!mesh.checkConnection()){  //TODO program freezes everytime this check fails
        // unconnected, try to reconnect
        Serial.println("\n[Mesh not connected, trying to reconnect...]");
        setLED(DISCONNECTED_SEQUENCE);
        if(!mesh.renewAddress(RENEWAL_TIMEOUT)){
          // reconnection effort failed, disconnected
          setValve(1, OFF);
          setValve(2, OFF);
          setValve(3, OFF);
          setValve(4, OFF);
          Serial.println("[MESH NOT CONNECTED]");
          myStatus.meshState = MESH_DISCONNECTED;
          myStatus.nodeMeshAddress = -1;
        }
        else{
          // reconnection effort succeeded, connected
          Serial.println("[Mesh reconnected]");
          myStatus.meshState = MESH_CONNECTED;
          setLED(CONNECTED_SEQUENCE);
        }
      }
      else{
        // connected
        //Serial.println("[Connected]");
        myStatus.meshState = MESH_CONNECTED;
        setLED(LED_OFF);
      }
    
      // update mesh address in case it changed
      int16_t tempvar = mesh.getAddress(myStatus.nodeID);
      // this sometimes fails, but does not mean disconnected; simply check to see it worked
      if(tempvar != -1) myStatus.nodeMeshAddress = tempvar;
}


/*
 * safeMeshWrite()
 * 
 * A function to do a mesh.write with automatic error handling
 * 
 * @param void* payload: a pointer to the payload
 * @param char header: the type of message
 * @param unsigned datasize: the size of the payload in bytes (use the sizeof() function)
 * @param unsigned timesToTry: the number of times to try to send the message if send fails
 * @return bool: true if send success, false if send fail
 */
bool safeMeshWrite(uint8_t destination, void* payload, char header, uint8_t datasize, uint8_t timesToTry){
  // disable interrupts in this function
  noInterrupts();
  
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
        setLED(DISCONNECTED_SEQUENCE);
        interrupts();
        return false;
      }
      else{
        // if succeeded, are reconnected and try again
        myStatus.meshState = MESH_CONNECTED;
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
          interrupts();
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
        interrupts();
        return false;
      }
    }    
    //return false;
  }
  else {
    // write succeeded
    Serial.print(F("Send of type ")); Serial.print(header); Serial.println(F(" success"));
    //Serial.println(*payload); // cannot dereference void*, and do not know type...
    interrupts();
    return true;
  }
}

/*
 * Setup flow meter; borrowed function
 */
SIGNAL(TIMER0_COMPA_vect){
  uint8_t x = digitalRead(FRATE);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
    //changedPulseFlag = true;
  }
  
  lastflowpinstate = x;
  flowrate = 1000.0 / lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

/*
 * Setup flow meter; borrowed function
 * @param bool v: whether to use interrupts
 */
void useInterrupt(bool v) {
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
 * setValve();
 * 
 * Sets valve to desired state
 * 
 * @param uint8_t whichValve: the valve to set; if not in range 1-4 or valve is not connected, returns -1
 * @param bool setTo: the desired state
 * @return int8_t: the actual state it was set to; -1 if valve is not connected
 */
int8_t setValve(uint8_t whichValve, bool setTo){
  // check if valve is connected, then open/close and set state
  switch(whichValve){
  case 1:
    if(myStatus.valveState1.isConnected == false) return -1;
    else{
      digitalWrite(VALVE_1, setTo);
      myStatus.valveState1.state = setTo;
    }
    break;
  case 2:
    if(myStatus.valveState2.isConnected == false) return -1;
    else{
      digitalWrite(VALVE_2, setTo);
      myStatus.valveState2.state = setTo;
    }
    break;
  case 3:
    if(myStatus.valveState3.isConnected == false) return -1;
    else{
      digitalWrite(VALVE_3, setTo);
      myStatus.valveState3.state = setTo;
    }
    break;
  case 4:
    if(myStatus.valveState4.isConnected == false) return -1;
    else{
      digitalWrite(VALVE_4, setTo);
      myStatus.valveState4.state = setTo;
    }
    break;
  default:
    break;
  }
  myStatus.numOpenValves = myStatus.valveState1.state + myStatus.valveState2.state + myStatus.valveState3.state + myStatus.valveState4.state;
  return setTo;
}


void updateFlowRate(){
  // can have false negative when detecting flow meter; if detect none, check again in case
  if(myStatus.hasFlowRateMeter == false){
    myStatus.hasFlowRateMeter = digitalRead(FRATE);
  }
  
  if(myStatus.hasFlowRateMeter){
    float beginLiters = pulses/7.5/60.0;                      // is initial amount of liters
    delay(RATE_MEASURING_PERIOD);                                        // wait 5 seconds
    //endLiters = pulses/7.5/60.0;                      // is end amount of liters
    myStatus.currentFlowRate = (pulses/7.5/60.0-beginLiters)/RATE_MEASURING_PERIOD*15.8503; // convert liters/sec to GPM //TODO something is wrong with this
    if(myStatus.currentFlowRate > MAX_MEASUREABLE_GPM) 
      myStatus.maxedOutFlowMeter = true;
    myStatus.accumulatedFlow += pulses/7.5/60.0*0.264172;  //  end amount in gallons
    pulses = 0;
  }
}

// to be called once per day, from master
void resetAccumulatedFlow(){
  myStatus.accumulatedFlow = 0;
  myStatus.maxedOutFlowMeter = false;
}


/*
 * Reads the node's ID from IDPIN_0-4 pins
 * @return uint8_t: the node's ID
 */
uint8_t readMyID(){
  uint8_t id = (!digitalRead(IDPIN_0))*1+(!digitalRead(IDPIN_1))*2
              +(!(analogRead(IDPIN_2)>300))*4+(!(analogRead(IDPIN_3)>300))*8;
              //+(!(analogRead(IDPIN_4)>300))*16+(!(analogRead(IDPIN_5)>300))*32;
  if(id==0) id = 16;
  return id;
}


/*
 * Sets LED according to desired pattern
 * @param uint8_t setTo: the pattern to set LED to
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
  default:
    break;
  }
}

/*
 * Resets the arduino by pressing its own "reset" button
 */
void hardReset(){
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(1000);
  // arduino resets here
}

void refreshReset(){
  pinMode(RESET_PIN, OUTPUT);  
  digitalWrite(RESET_PIN, LOW);
  delay(50); // 95% charged: 3*tau = 3*RC = 3*200*47*10^-6 = 28 ms
  //digitalWrite(RESETPIN, LOW); // to turn off internal pullup
  pinMode(RESET_PIN, INPUT);
}

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
  useInterrupt(true);
  
  // LED
  pinMode(LED, OUTPUT);
  
  // RESET_GND -- don't init until end of setup() to prevent cycle
  
  // ID pins
  pinMode(IDPIN_0, INPUT_PULLUP);
  pinMode(IDPIN_1, INPUT_PULLUP);
  pinMode(IDPIN_2, INPUT_PULLUP);
  pinMode(IDPIN_3, INPUT_PULLUP);
  
  // VIN
  pinMode(VIN_REF, INPUT);
}

void initStatus(){
  // storedVIN
  myStatus.storedVIN = EEPROM.read(VIN_EEPROM_ADDR);

  // voltageState
  if(analogRead(VIN_REF)>>2 > myStatus.storedVIN*(1+OK_VIN_RANGE))
    myStatus.voltageState = HIGH_VOLTAGE;
  else if (analogRead(VIN_REF)>>2 < myStatus.storedVIN*(1-OK_VIN_RANGE))
    myStatus.voltageState = LOW_VOLTAGE;
  else
    myStatus.voltageState = GOOD_VOLTAGE;
    
  // hasFlowRateMeter
  myStatus.hasFlowRateMeter = digitalRead(FRATE);

  // currentFlowRate
  myStatus.currentFlowRate = 0;
  updateFlowRate();

  // flowState
  if(myStatus.hasFlowRateMeter == false) myStatus.flowState = HAS_NO_METER;
  else{
    // if water flowing, bad
    if(myStatus.currentFlowRate > MIN_MEASUREABLE_GPM)
      myStatus.flowState = STUCK_AT_ON;
    // if no water flowing, good
    else
      myStatus.flowState = NO_FLOW_GOOD;
  }

  // accumulatedFlow
  myStatus.accumulatedFlow = 0;

  // maxedOutFlowMeter
  myStatus.maxedOutFlowMeter = false;

  // numConnectedValves
  myStatus.numConnectedValves = 0; // incremented as appropriate below

  // numOpenValves
  myStatus.numOpenValves = 0;

  // valves
  pinMode(VALVE_1, INPUT_PULLUP);
  myStatus.valveState1.isConnected = !digitalRead(VALVE_1);
  pinMode(VALVE_1, OUTPUT);
  if(myStatus.valveState1.isConnected) myStatus.numConnectedValves++;
  digitalWrite(VALVE_1, LOW); // TODO: use setValve() to handle this and next line?
  myStatus.valveState1.state = OFF;

  pinMode(VALVE_2, INPUT_PULLUP);
  myStatus.valveState2.isConnected = !digitalRead(VALVE_2);
  pinMode(VALVE_2, OUTPUT);
  if(myStatus.valveState2.isConnected) myStatus.numConnectedValves++;
  digitalWrite(VALVE_2, LOW);
  myStatus.valveState2.state = OFF;

  pinMode(VALVE_3, INPUT_PULLUP);
  myStatus.valveState3.isConnected = !digitalRead(VALVE_3);
  pinMode(VALVE_3, OUTPUT);
  if(myStatus.valveState3.isConnected) myStatus.numConnectedValves++;
  digitalWrite(VALVE_3, LOW);
  myStatus.valveState3.state = OFF;

  pinMode(VALVE_4, INPUT_PULLUP);
  myStatus.valveState4.isConnected = !digitalRead(VALVE_4);
  pinMode(VALVE_4, OUTPUT);
  if(myStatus.valveState4.isConnected) myStatus.numConnectedValves++;
  digitalWrite(VALVE_4, LOW);
  myStatus.valveState1.state = OFF;

  // meshState
  myStatus.meshState = MESH_DISCONNECTED;

  // nodeID
  myStatus.nodeID = readMyID();

  // nodeMeshAddress
  myStatus.nodeMeshAddress = -1;
}

void printNodeStatus(){
  // print number of times executed
  Serial.print('\n'); Serial.println(statusCounter++);

  Serial.print("Input voltage     : ");
  Serial.print((analogRead(VIN_REF)>>2<<2)*3*4.6/1023.0); Serial.print(" V  : ");
  if(myStatus.voltageState == GOOD_VOLTAGE) Serial.println("good");
  else if(myStatus.voltageState == HIGH_VOLTAGE) Serial.println("HIGH INPUT VOLTAGE!");
  else if(myStatus.voltageState == LOW_VOLTAGE) Serial.println("LOW INPUT VOLTAGE!");

  // if has flow rate meter, tell current rate and accumulated flow
  if(myStatus.hasFlowRateMeter){
    Serial.print("Current flow rate : "); Serial.print(myStatus.currentFlowRate); Serial.print(" GPM : ");
    if(myStatus.flowState == NO_FLOW_GOOD) Serial.println("good");
    else if(myStatus.flowState == FLOWING_GOOD) Serial.println("good");
    else if(myStatus.flowState == STUCK_AT_OFF) Serial.println("VALVE OPEN BUT NO FLOW!");
    else if(myStatus.flowState == STUCK_AT_ON) Serial.println("VALVE LEAKING!");
    Serial.print("Accumulated flow  : "); Serial.print(myStatus.accumulatedFlow); Serial.println(" gal");
  }
  // else tell that has no meter
  else Serial.println("No flow meter");

  // if valve is connected, tell state
  if(myStatus.valveState1.isConnected){
    Serial.print("Valve 1 is        : "); 
    myStatus.valveState1.state ? Serial.println("OPEN") : Serial.println("closed");
  }
  if(myStatus.valveState2.isConnected){
    Serial.print("Valve 2 is        : "); 
    myStatus.valveState2.state ? Serial.println("OPEN") : Serial.println("closed");
  }
  if(myStatus.valveState3.isConnected){
    Serial.print("Valve 3 is        : "); 
    myStatus.valveState3.state ? Serial.println("OPEN") : Serial.println("closed");
  }
  if(myStatus.valveState4.isConnected){
    Serial.print("Valve 4 is        : "); 
    myStatus.valveState4.state ? Serial.println("OPEN") : Serial.println("closed");
  }

  // tell mesh state
  Serial.print("Node ID, address  : "); Serial.print(myStatus.nodeID); Serial.print(", "); 
  Serial.print(myStatus.nodeMeshAddress); Serial.print("     : ");
  if(myStatus.meshState == MESH_CONNECTED) Serial.println("good");
  else if(myStatus.meshState == MESH_DISCONNECTED) Serial.println("DISCONNECTED!");
}

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Setup       //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void setup(){

  initPins();
  
  // begin serial communication
  Serial.begin(BAUD_RATE);
  
  // check if button is being pressed; if so, do special startup
  if(digitalRead(BUTTON) == 0){
    setLED(SPECIAL_BOOT_SEQUENCE);

    // do special stuff
    Serial.println("\n\n////Special boot sequence////");

    // store VIN value in EEPROM
    Serial.print("Reading voltage source: "); Serial.print((analogRead(VIN_REF)>>2<<2)*3*4.6/1023.0); Serial.println("V\n");
    myStatus.storedVIN = analogRead(VIN_REF)>>2; // shave off the last two bits to fit in one byte
    EEPROM.write(VIN_EEPROM_ADDR, myStatus.storedVIN);
  }

  // "power-on" light sequence
  setLED(TURN_ON_SEQUENCE);

  initStatus();

  //printNodeStatus();
  // print ID and number and location of connected valves and flow meters
  Serial.print(F("\n/////////Booted//////////\nNodeID: ")); Serial.println(readMyID());
  Serial.print(F("Voltage source: ")); Serial.print((analogRead(VIN_REF)>>2<<2)*3*4.6/1023.0); Serial.print("V, ");
  if(analogRead(VIN_REF)>>2 > myStatus.storedVIN*(1+OK_VIN_RANGE))
    Serial.println("HI VOLTAGE WARNING");
  else if (analogRead(VIN_REF)>>2 < myStatus.storedVIN*(1-OK_VIN_RANGE))
    Serial.println("LOW VOLTAGE WARNING");
  else
    Serial.println("within expected range");
  Serial.print("Valve 1: "); myStatus.valveState1.isConnected ? Serial.println("CONNECTED") : Serial.println("disconnected");
  Serial.print("Valve 2: "); myStatus.valveState2.isConnected ? Serial.println("CONNECTED") : Serial.println("disconnected");
  Serial.print("Valve 3: "); myStatus.valveState3.isConnected ? Serial.println("CONNECTED") : Serial.println("disconnected");
  Serial.print("Valve 4: "); myStatus.valveState4.isConnected ? Serial.println("CONNECTED") : Serial.println("disconnected");
  //Serial.print("FRate:   "); myStatus.hasFlowRateMeter ? Serial.println("CONNECTED\n") : Serial.println("disconnected\n");
  // should be the above, but have slightly high probability of false negative on the first try, so just check directly
  Serial.print("FRate:   "); digitalRead(FRATE) ? Serial.println("CONNECTED\n") : Serial.println("disconnected\n");


  // set NodeID and prep for mesh.begin()
  mesh.setNodeID(myStatus.nodeID); // do manually
  setLED(DISCONNECTED_SEQUENCE);

  // while unconnected, try CONNECTION_TRIES (5) times consecutively every DISCONNECTED_SEQUENCE_SLEEP (15) minutes indefinitely
  bool success = false;
  while(!success){
    uint8_t attempt;
    for(attempt=1; attempt<=CONNECTION_TRIES; attempt++){
      Serial.print(F("Connecting to the mesh (attempt ")); Serial.print(attempt); Serial.print(")... ");
      success = mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT);
      if(success){
        Serial.println("CONNECTED");
        break;
      }
      else Serial.println("FAILED");
    }
    if(!success){
      Serial.println("Trying again in 15 minutes. Else powerdown or reset.\n");
      delay(DISCONNECTED_SLEEP);
    }
  }

  // "connected" light sequence
  setLED(CONNECTED_SEQUENCE);
  myStatus.meshState = MESH_CONNECTED;

  // allow children to connect
  mesh.setChild(true);

  // init timer for regular system checks
  Timer1.initialize(TIMER1_PERIOD);
  Timer1.attachInterrupt(getNodeStatusISR);

  // attach interrupt to button
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButtonISR, FALLING);

  // enable self-resetting ability
  delay(50);  // allow capacitor to discharge if was previously charged before enabling autoreset again
              // 95% = 3*tau = 3*RC = 3*200*100*10^-6 = 60ms -- but never gets fully charged, and has
              //    been dicharging during previous setup, so 50ms is sufficient
  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_GND, OUTPUT);
  digitalWrite(RESET_GND, LOW);
}


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Loop        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void loop() {
  // refresh the reset
  refreshReset();

  // refresh the mesh
  mesh.update();

  // update node status if necessary
  if(updateNodeStatusFlag){
    getNodeStatus();
    printNodeStatus();
    updateNodeStatusFlag = false;
  }

  // handle button presses--send 'r' type message
  // NOT FINAL CODE
  if(hadButtonPress){
    // tell current flow rate
    Serial.println(F("Detected buttonpress"));
    Serial.println(F("Measuring the current flowrate..."));
    float beginLiters, flowrate;//endLiters, flowrate;
    updateFlowRate();
    safeMeshWrite(MASTER_ADDRESS, &myStatus.currentFlowRate, SEND_FLOW_RATE_H, sizeof(myStatus.currentFlowRate), DEFAULT_SEND_TRIES);
    
    // reset flag
    hadButtonPress = false;

    // for test
    //hardReset();
  }

  // handle flow meter pulses
  /*if (changedPulseFlag == true){
    //Serial.print("Freq: "); Serial.println(flowrate);
    //Serial.print("Pulses: "); Serial.println(pulses, DEC);
    // if a plastic sensor use the following calculation
    // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
    // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
    // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
    // Liters = Pulses / (7.5 * 60)
    changedPulseFlag = false;
    float liters = pulses/7.5/60.0;
    //liters /= 7.5;
    //liters /= 60.0;
    //Serial.print(liters); Serial.println(" Liters");
  }*/

  // read in messages
  while(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    //void* payload = malloc(sizeof(float)); // float is largest possible size
    //network.read(header, payload, sizeof(payload));
    Serial.print(F("Received ")); Serial.print(char(header.type)); Serial.println(F(" type message."));
    Serial.print(F("From: ")); Serial.println(mesh.getNodeID(header.from_node));

    switch(header.type){
    case SET_VALVE_H:
      // Valve command, on or off; type is bool
      //  set the value and send return message
      Valve_Command vc;
      network.read(header, &vc, sizeof(vc));
      Serial.print(F("Command is to turn valve ")); Serial.print(vc.whichValve); Serial.print(" "); vc.onOrOff ? Serial.println("ON") : Serial.println("OFF");
      int8_t result;
      result = setValve(vc.whichValve, vc.onOrOff);
      safeMeshWrite(MASTER_ADDRESS, &result, SEND_VALVE_H, sizeof(result), DEFAULT_SEND_TRIES);
      break;
      
    case INFO_REQUEST_H:    
      // Request; type is char
      char typeOfData;
      network.read(header, &typeOfData, sizeof(typeOfData));
      
      switch( typeOfData ){
      case GET_VALVE_P:
        // tell current valve state
        uint8_t whichValve;
        network.read(header, &whichValve, sizeof(whichValve));
        Serial.print(F("Command is to tell valve state of valve ")); Serial.print(whichValve);
        Valve_Status vs;
        if(whichValve == 1) vs = myStatus.valveState1;
        else if(whichValve == 2) vs = myStatus.valveState2;
        else if(whichValve == 3) vs = myStatus.valveState3;
        else myStatus.valveState4;
        Serial.print(F(", which is ")); vs.isConnected ? (vs.state ? Serial.println("ON") : Serial.println("OFF")) : Serial.println("DISCONNECTED");
        safeMeshWrite(MASTER_ADDRESS, &vs, SEND_VALVE_H, sizeof(vs), DEFAULT_SEND_TRIES);        
        break;
      case GET_FLOW_RATE_P:
        // tell current flow rate
        Serial.print(F("Command is to current flow rate... "));
        updateFlowRate();
        Serial.print(F("Flow rate is ")); Serial.print(myStatus.currentFlowRate); Serial.println(F(" GPM"));
        safeMeshWrite(MASTER_ADDRESS, &myStatus.currentFlowRate, SEND_FLOW_RATE_H, sizeof(myStatus.currentFlowRate), DEFAULT_SEND_TRIES);
        break;
      case GET_NODE_STATUS_P:
        // return status
        Serial.print(F("Command is to tell my status: ")); printNodeStatus();
        safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
        break;
      default:
        break;
      }      
      break;
      
    case FORCE_RESET_H:
      hardReset();
      break;

    case IS_NEW_DAY_H:
      resetAccumulatedFlow();
      break;
      
    default:
      break;
    }
  }
}

