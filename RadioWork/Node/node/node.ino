/**
 * node.ino
 * 
 * This sketch is for nodes in the GardeNet system. Functions include:
 *    - Establishing and maintaining a mesh network
 *    - Controlling up to four valves
 *    - Controlling up to three flow meters
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
//#include <SPI.h>
//#include <EEPROM.h>
#include <TimerOne.h>
#include "C:\\Users\\kevin\\Documents\\Senior_Design_Team_16\\RadioWork\\Shared\\settings.h"
//#include "C:\\Users\\kevin\\Documents\\Senior_Design_Team_16\\RadioWork\\Shared\\SharedFunctions.h"

// pins
#define BUTTON    2
#define RF24_IRQ  3 // currently unused
#define RESETPIN  4
#define VALVE_1   5
#define VALVE_2   6
#define VALVE_3   7
#define VALVE_4   8
#define RF24_CE   9
#define RF24_CS   10
// RF24_MOSI        11  //predifined
// RF24_MISO        12  //predifined
// RF24_SCK         13  //predifined
#define FRATE     A0
#define LEDR      A1
#define IDPIN_0   A2
#define IDPIN_1   A3
#define IDPIN_2   A4
#define IDPIN_3   A5
//#define IDPIN_4   A6  //note: analog only
//#define IDPIN_5   A7  //note: analog only


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
bool changedPulseFlag = false; // interrupt is called once a millisecond, looks for any pulses from the sensor

// structs


// other
uint32_t displayTimer = 0;
volatile bool hadButtonPress = false;
volatile bool getNodeStatusFlag = false;
int8_t valveState = 0;
uint8_t myStatus = 0;
uint8_t statusCounter = 0;
bool connections[5] = {0, 0, 0, 0, 0};

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
void getNodeStatusISR(){
  getNodeStatusFlag = true;
}

void getNodeStatus(){
  // disable interrupts
  //noInterrupts();
  
  // print number of times executed
  Serial.print('\n'); Serial.println(statusCounter++);

  // check mesh connection and print status
  if(!mesh.checkConnection()){  //TODO program freezes everytime this check fails
    // if unconnected, try to reconnect
    Serial.println("Not connected, trying to reconnect...");
    setLEDR(DISCONNECTED);
    //delay(2000);
    if(!mesh.renewAddress(RENEWAL_TIMEOUT)){
      setValve(VALVE_1, false);
      setValve(VALVE_2, false);
      setValve(VALVE_3, false);
      setValve(VALVE_4, false);
      Serial.println("NOT CONNECTED");
    }
    else{
      Serial.println("Reconnected");
      setLEDR(CONNECTED_SEQUENCE);
    }
  }
  else{
    Serial.println("Connected");
    setLEDR(LEDR_OFF);
  }

  // print node ID
  Serial.print("My ID:      ");
  Serial.println(mesh.getNodeID());
  // print address DOESN'T WORK
  //Serial.print("My address: ");
  //Serial.println(mesh.getAddress(readMyID));

  // re-enable interrupts
  //interrupts();
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
        setLEDR(DISCONNECTED);
        interrupts();
        return false;
      }
      else{
        // if succeeded, are reconnected and try again
        setLEDR(CONNECTED_SEQUENCE);
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
    changedPulseFlag = true;
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
 * Sets valve to desired state
 * @param uint8_t whichValve: the valve to set; if not in range 1-4 or valve is not connected, returns -1
 * @param bool setTo: the desired state
 * @return int: the actual state it was set to
 */
int8_t setValve(uint8_t whichValve, bool setTo){
  if(!(whichValve==VALVE_1 || whichValve==VALVE_2 || whichValve==VALVE_3 || whichValve==VALVE_4) || !connections[whichValve-VALVE_1]) return -1;
  // set valve
  valveState = setTo ? 1 : 0;
  //if(whichValve < VALVE_3)  digitalWrite(whichValve, setTo);
  //else                      analogWrite(whichValve, setTo*255);
  digitalWrite(whichValve, setTo);

  // reset accumulated water flow
  if(valveState == false)
    pulses = 0;

  // LEDY replicates valve state
  //digitalWrite(LEDY, setTo);
  return valveState;
}

/*
 * Reads the node's ID from IDPIN_0-4
 * @return uint8_t: the node's ID
 */
uint8_t readMyID(){
  uint8_t id = (!digitalRead(IDPIN_0))*1+(!digitalRead(IDPIN_1))*2
              +(!digitalRead(IDPIN_2))*4+(!analogRead(IDPIN_3))*8;
              //+(!(digitalRead(IDPIN_4)>300))*16+(!(digitalRead(IDPIN_5)>300))*32;
  if(id==0) id = 16;
  return id;
}

/*
 * Resets the arduino by pressing its own "reset" button
 */
void hardReset(){
  pinMode(RESETPIN, OUTPUT);
  analogWrite(RESETPIN, 0); // is analog pin
}

/*
 * Determines the node's connections--attached valves and flow meters--and populates bool connections[7]
 */
void readConnections(){
  // valves first: read if connected (LOW=conn, HIGH=disc), then return to output
  pinMode(VALVE_1, INPUT_PULLUP);
  pinMode(VALVE_2, INPUT_PULLUP);
  pinMode(VALVE_3, INPUT_PULLUP);
  pinMode(VALVE_4, INPUT_PULLUP);
  
  connections[0] = !digitalRead(VALVE_1);
  connections[1] = !digitalRead(VALVE_2);
  connections[2] = !digitalRead(VALVE_3);
  connections[3] = !digitalRead(VALVE_4);
  
  pinMode(VALVE_1, OUTPUT);
  pinMode(VALVE_2, OUTPUT);
  pinMode(VALVE_3, OUTPUT);
  pinMode(VALVE_4, OUTPUT);

  // if valve is disconnected, drive pin to 0
  if(!connections[0]) digitalWrite(VALVE_1, LOW);
  if(!connections[1]) digitalWrite(VALVE_2, LOW);
  if(!connections[2]) digitalWrite(VALVE_3, LOW);
  if(!connections[3]) digitalWrite(VALVE_4, LOW);

  // flow meters second: read if connected (LOW=conn, HIGH=disc)
  connections[4] = digitalRead(FRATE);
}


/*
 * Sets LEDR according to desired pattern
 * @param uint8_t setTo: the pattern to set LEDR to
 */
void setLEDR(uint8_t setTo){
  switch(setTo){
  case LEDR_OFF:
    digitalWrite(LEDR, LOW);
    break;
  case LEDR_ON:
    analogWrite(LEDR, LEDR_BRIGHTNESS);
    break;
  case TURN_ON_SEQUENCE:
    analogWrite(LEDR, LEDR_BRIGHTNESS);
    delay(250);
    digitalWrite(LEDR, LOW);
    delay(250);
    analogWrite(LEDR, LEDR_BRIGHTNESS);
    delay(250);
    digitalWrite(LEDR, LOW);
    break;
  case CONNECTED_SEQUENCE:
    analogWrite(LEDR, LEDR_BRIGHTNESS);
    delay(250);
    digitalWrite(LEDR, LOW);
    delay(250);
    analogWrite(LEDR, LEDR_BRIGHTNESS);
    delay(250);
    digitalWrite(LEDR, LOW);
    break;
  case DISCONNECTED:
    analogWrite(LEDR, LEDR_BRIGHTNESS);
    break;
  default:
    break;
  }
}

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Setup       //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void setup(){
  
  // begin serial communication
  Serial.begin(BAUD_RATE);
  
  // init LED and Button
  pinMode(LEDR, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING);

  // "power-on" light sequence
  setLEDR(TURN_ON_SEQUENCE);

  // init ID pins
  pinMode(IDPIN_0, INPUT_PULLUP);
  pinMode(IDPIN_1, INPUT_PULLUP);
  pinMode(IDPIN_2, INPUT_PULLUP);
  pinMode(IDPIN_3, INPUT_PULLUP);
  //pinMode(IDPIN_4, INPUT_PULLUP);
  //pinMode(IDPIN_5, INPUT_PULLUP);

  // init flow sensors
  pinMode(FRATE, INPUT);
  lastflowpinstate = digitalRead(FRATE);
  useInterrupt(true);

  // read attached valves, flow meters, and init their pins
  readConnections();

  // print ID and number and location of connected valves and flow meters
  Serial.print(F("\n/////////Booted//////////\nNodeID: ")); Serial.println(readMyID());
  Serial.print("Valve 1: "); connections[0] ? Serial.println("CONNECTED") : Serial.println("disconnected");
  Serial.print("Valve 2: "); connections[1] ? Serial.println("CONNECTED") : Serial.println("disconnected");
  Serial.print("Valve 3: "); connections[2] ? Serial.println("CONNECTED") : Serial.println("disconnected");
  Serial.print("Valve 4: "); connections[3] ? Serial.println("CONNECTED") : Serial.println("disconnected");
  Serial.print("FRate:   "); connections[4] ? Serial.println("CONNECTED") : Serial.println("disconnected");

  // close valves
  setValve(VALVE_1, false);
  setValve(VALVE_2, false);
  setValve(VALVE_3, false);
  setValve(VALVE_4, false);

  // set NodeID and prep for mesh.begin()
  mesh.setNodeID(readMyID()); // do manually
  setLEDR(DISCONNECTED);

  // while unconnected, try CONNECTION_TRIES (5) times consecutively every DISCONNECTED_SLEEP (15) minutes indefinitely
  bool success = false;
  while(!success) {
    uint8_t attempt;
    for(attempt=1; attempt<=CONNECTION_TRIES; attempt++){
      Serial.print(F("\nConnecting to the mesh (attempt ")); Serial.print(attempt); Serial.print(")... ");
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
  setLEDR(CONNECTED_SEQUENCE);

  // allow children to connect
  mesh.setChild(true);

  // init timer for regular system checks
  Timer1.initialize(TIMER1_PERIOD);
  Timer1.attachInterrupt(getNodeStatusISR);

  // attach interrupt to button
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING);
}


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Loop        //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void loop() {

  // refresh the mesh
  mesh.update();

  // update node status if necessary
  if(getNodeStatusFlag){
    getNodeStatus();
    getNodeStatusFlag = false;
  }

  // handle button presses--send 'r' type message
  // NOT FINAL CODE
  if(hadButtonPress){
    // tell current flow rate
    Serial.println("Measuring the current flowrate...");
    float beginLiters, flowrate;//endLiters, flowrate;
    beginLiters = pulses/7.5/60.0;                      // is initial amount of liters
    delay(RATE_MEASURING_PERIOD);                                        // wait 5 seconds
    //endLiters = pulses/7.5/60.0;                      // is end amount of liters
    flowrate = (pulses/7.5/60.0-beginLiters)/5*15.8503; // convert liters/sec to GPM
    safeMeshWrite(MASTER_ADDRESS, &flowrate, SEND_FLOW_RATE_H, sizeof(flowrate), DEFAULT_SEND_TRIES);
    
    // reset flag
    hadButtonPress = false;

    hardReset();
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
      bool onOrOff;
      network.read(header, &onOrOff, sizeof(onOrOff));
      Serial.print(F("Command is to turn ")); onOrOff ? Serial.println("ON") : Serial.println("OFF");
      setValve(VALVE_1, onOrOff); //TODO
      safeMeshWrite(MASTER_ADDRESS, &valveState, SEND_VALVE_H, sizeof(valveState), DEFAULT_SEND_TRIES);
      break;
      
    case INFO_REQUEST_H:    
      // Request; type is char
      char typeOfData;
      network.read(header, &typeOfData, sizeof(typeOfData));
      
      switch( typeOfData ){
      case GET_VALVE_P:
        // tell current valve state
        Serial.print(F("Command is to tell valve state, ")); Serial.println(valveState);
        safeMeshWrite(MASTER_ADDRESS, &valveState, SEND_VALVE_H, sizeof(valveState), DEFAULT_SEND_TRIES);        
        break;
      case GET_FLOW_RATE_P:
        // tell current flow rate
        Serial.print(F("Command is to current flow rate... "));
        float beginLiters, flowrate;//endLiters, flowrate;
        beginLiters = pulses/7.5/60.0;                      // is initial amount of liters
        delay(RATE_MEASURING_PERIOD);                       // wait 5 seconds (or whatever RATE_MEASURING_PERIOD in settings.h is)
        //endLiters = pulses/7.5/60.0;                      // is end amount of liters
        flowrate = (pulses/7.5/60.0-beginLiters)/5*15.8503; // convert liters/sec to GPM
        Serial.print(F("Flow rate is ")); Serial.print(flowrate); Serial.println(F("GPM"));
        safeMeshWrite(MASTER_ADDRESS, &flowrate, SEND_FLOW_RATE_H, sizeof(flowrate), DEFAULT_SEND_TRIES);
        break;
      case GET_NODE_STATUS_P:
        // return status
        Serial.print(F("Command is to tell my status, ")); Serial.println(myStatus);
        safeMeshWrite(MASTER_ADDRESS, &myStatus, SEND_NODE_STATUS_H, sizeof(myStatus), DEFAULT_SEND_TRIES);
        break;
      default:
        break;
      }      
      break;
      
    case FORCE_RESET:
      hardReset();
      break;
      
    default:
      break;
    }
  }
}

