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
#define nodeID            02
#define masterAddress     0
#define DEFAULTSENDTRIES 5


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
  //Serial.println(mesh.getAddress(nodeID));
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
 * A function to do a mesh.write with automatic error handling
 * @param void* payload: a pointer to the payload
 * @param char header: the type of message
 * @param unsigned datasize: the size of the payload in bytes (use the sizeof() function)
 * @param unsigned timesToTry: the number of times to try to send the message if send fails
 * @return bool: true if send success, false if send fail
 */
bool safeMeshWrite(void* payload, char header, unsigned datasize, unsigned timesToTry){
  // perform write
  if (!mesh.write(masterAddress, payload, header, datasize)) {
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
          delay(50);
          return safeMeshWrite(payload, header, datasize, --timesToTry);
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
        delay(50);
        return safeMeshWrite(payload, header, datasize, --timesToTry);
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
    Serial.print(F("Send of type ")); Serial.print(header); Serial.print(F(" success"));
    //Serial.println(*payload); // cannot dereference void*, and do not know type...
    return true;
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

  // handle button presses--send 'r' type message
  // NOT FINAL CODE
  if(hadButtonPress){
    // 'r' message sends the accumulated water flow, in liters
    float liters = pulses/7.5/60.0;
    safeMeshWrite(&liters, 'r', sizeof(float), DEFAULTSENDTRIES);

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
      safeMeshWrite(&valveState, 'v', sizeof(valveState), DEFAULTSENDTRIES);
      break;
      
    case 'R':    
      // Request; type is char
      char typeOfData;
      network.read(header, &typeOfData, sizeof(typeOfData));
      
      switch( typeOfData ){
      case 'v':
        // tell current valve state
        safeMeshWrite(&valveState, 'v', sizeof(valveState), DEFAULTSENDTRIES);
        break;
      case 'r':
        // tell current flow rate
        float liters;
        liters = pulses/7.5/60.0; // TODO: make flowrate; currently is accumulated water
        safeMeshWrite(&liters, 'r', sizeof(liters), DEFAULTSENDTRIES);
        break;
      case 'n':
        // return status
        safeMeshWrite(&myStatus, 'n', sizeof(myStatus), DEFAULTSENDTRIES);
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

