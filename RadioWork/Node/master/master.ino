/* 
 * master.ino
 * 
 * This sketch is for master (gateway) in the GardeNet system. Functions include:
 *    - Establishing and maintaining a mesh network
 *    - Maintaining a 3G connection to the GardeNet server
 *    - Controlling up to 16 nodes
 *    - Tracking several variables with regards to its and the garden's status
 * 
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last Modified: 4/6/16
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
//#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/RadioWork/Shared/SharedDefinitions.h"
//#include <StandardCplusplus.h>
//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>
//#include <Time.h>
//#include "Schedule.h"
//#include "ScheduleEvent.h"

// pins
//#define unused    2
#define BUTTON    3
#define LEDR      4
#define LEDG      5
//#define unused    6
#define RF24_CE   7
#define RF24_CS   8
//RF24_MOSI //predifined on ICSP header
//RF24_MISO //predifined on ICSP header
//RF24_SCK  //predifined on ICSP header
//#define unused    9
//#define unused    10
//#define unused    11
#define ThreeG    12
//#define unused    13
#define RESET_GND A0
#define RESET_PIN A1
//#define unused    A2
//#define unused    A3
//#define unused    A4
//#define unused    A5
#define Modem_Serial Serial1


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////     Globals     ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// mesh network
RF24 radio(RF24_CE, RF24_CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

// flags
volatile bool hadButtonPress = false;
volatile bool updateStatusFlag = false;

// other
Garden_Status gardenStatus;
//Schedule weeklySchedule;
uint8_t statusCounter = 0;


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
    Serial.println(F("\n[Detected buttonpress]"));
  }
}

/* 
 * updateStatusISR()
 * 
 * ISR called when timer1 times out; sets a flag and exits
 * 
 * @preconditions: timer interrupt must be enabled
 * @postconditions: updateNodeStatus flag is set
 */
void updateStatusISR(){
  updateStatusFlag = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Helper Functions///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


void initPins(){  
  // BUTTON
  pinMode(BUTTON, INPUT_PULLUP);
  // attach interrupt at the end of setup() in initPins2()
  
  // LEDR and LEDG
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);  

  // ThreeG
  // handle in modem-specific code; has very specific behavior

  // RESET_GND
  // initialize at the end of setup() in initPins2()

  // RESET_PIN
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
}

void initPins2(){
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

/* 
 * flashLED()
 * 
 * Flashes an LED based on the arguments
 * 
 * @preconditions: pins initialized
 * @postconditions: none
 * 
 * @param uint8_t whichLED: the pin name of the LED you want to flash, must be LEDR or LEDG
 * @param uint8_t myNum: the number of times you want LED to flash
 * @param uint8_t myTime: the duration of LED's on/off state when it flashes
*/
void flashLED(uint8_t whichLED, uint8_t myNum, uint8_t myTime){
  if(whichLED != LEDR && whichLED != LEDG) return;
  for (int i = 0; i < myNum; i++) {
    digitalWrite(whichLED, HIGH);
    delay(myTime);  
    digitalWrite(whichLED, LOW);
    delay(myTime);
  }  
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

void checkSchedule(){
  // for each node
  uint8_t node;
  for(node=1; node<=16; node++){
    // if registered
    if(gardenStatus.nodeStatusPtrs[node] != NULL){
      // if connected
      if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED){
        // for each valve
        uint8_t valve;
        for(valve=1; valve<=4; valve++){
          // if schedule says should be open and is closed
          bool shouldBeOn;
          
          // TODO insert code to get shouldBeOpen according to schedule
          
          if(shouldBeOn && gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state == OFF){
            // send open signal
            Valve_Command vc;
            vc.whichValve = valve;
            vc.state = ON;
            safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
          }
          // else if schedule says should be closed and is open
          else if(shouldBeOn == false && gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state == ON){
            // send close signal
            Valve_Command vc;
            vc.whichValve = valve;
            vc.state = OFF;
            safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
          }
          // else the state is as it should be
        }
      }
      // else node is not connected to mesh
      else{
        // TODO alert the server?
      }
    }
    // else unregistered
  }    
}


/* 
 * safeMeshWrite()
 *
 * Performs mesh.writes, but adds reliability features, hence "safe". If mesh.write doesn't work, 
 * then tries again after a set period of time; if a set number of tries doesn't work, then the 
 * addressed node is considered disconnected. Maximum latency = 5 seconds.
 * 
 * @preconditions: mesh is configured
 * @postconditions: message is sent, or else node is considered to be disconnected
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
    // if a write fails, check if have more tries
    if(timesToTry > 0){
      delay(RETRY_PERIOD);
      return safeMeshWrite(destination, payload, header, datasize, --timesToTry);
    }
    else{
      if(mesh.getNodeID(destination) != -1 && gardenStatus.nodeStatusPtrs[mesh.getNodeID(destination)] != NULL){
        gardenStatus.nodeStatusPtrs[mesh.getNodeID(destination)]->meshState = MESH_DISCONNECTED; // TODO is this the only way to tell?
      }
      return false;
    }
  }
  else {
    // write succeeded
    Serial.print(F("Send of type ")); Serial.print(header); Serial.println(F(" success"));
    return true;
  }
}

bool checkNodeRegistered(uint8_t nodeID){
  if(gardenStatus.nodeStatusPtrs[nodeID] == NULL){
    gardenStatus.nodeStatusPtrs[nodeID] = new Node_Status;
    gardenStatus.numRegisteredNodes++;
    return false;
  }
  return true;
}

void readMeshMessages(){
  while(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    Serial.print(F("Received ")); Serial.print(char(header.type));
    Serial.print(F(" type message from node ")); Serial.println(mesh.getNodeID(header.from_node));

    switch(header.type){
    case SEND_VALVE_H:
      // A node is responding to a valve command. Read the response; if not what is expected, send
      //  another command to correct it; else all is well. (TODO add counter, time-to-live, so 
      //  don't have endless cycle in case things fail?

      // TODO implement the above
      break;
    case SEND_NODE_STATUS_H:
      // A node is reporting its status. Update its status in gardenStatus

      // check that it's registered
      checkNodeRegistered(mesh.getNodeID(header.from_node));
      
      // read in the new status
      network.read(header, gardenStatus.nodeStatusPtrs[mesh.getNodeID(header.from_node)], sizeof(Node_Status));
      break;
    case SEND_JUST_RESET_H:
      // means that a node has recovered from reset; true means it was told to do so, false means a crash

      bool toldToReset;      
      network.read(header, &toldToReset, sizeof(toldToReset));
      
      // TODO what to do with this information? ask for status update?
      
      break;
    case SEND_NODE_SLEEP_H:
      // node had its button pressed and toggled sleep; update its status and TODO let the website know?

      // check that it's registered
      checkNodeRegistered(mesh.getNodeID(header.from_node));
      
      bool nodeIsAwake;
      network.read(header, &nodeIsAwake, sizeof(nodeIsAwake));

      gardenStatus.nodeStatusPtrs[mesh.getNodeID(header.from_node)]->isAwake = nodeIsAwake;

      // TODO what else to do with this information?
      
      break;
    case SEND_NEW_DAY_H:
      // node is responding that it got the "new day" message; should respond "true", meaning that it is awake
      
      // check that it's registered
      checkNodeRegistered(mesh.getNodeID(header.from_node));

      bool response;
      network.read(header, &response, sizeof(response));

      // TODO what else to do with this information?
      
      break;
    default:
      Serial.println(F("Unknown message type."));
      break;
    }
  }
}

/* 
 * initStatus()
 * 
 * Initializes the Garden_Status struct gardenStatus.
 * 
 * @preconditions: pins are configured
 * @postconditions: myStatus is gardenStatus
 */
void initGardenStatus(){  
  // isAwake
  gardenStatus.isAwake = true;
  
  // TODO
}

/* 
 * updateGardenStatus()
 *
 * Performs diagnostics on the 3G connection, mesh connection, and nodes
 * 
 * @preconditions: gardenStatus is initialized
 * @postconditions: gardenStatus is updated
 */
void updateGardenStatus(){

  //////////// CHECK 3G CONNECTION ////////////  

  // TODO
  

  //////////// CHECK MESH CONNECTION ////////////

  // TODO
  
  
  //////////// CHECK NODE_STATUSES ////////////

  // TODO  
}

/* 
 * printGardenStatus()
 *
 * Prints gardenStatus to Serial port in a user-friendly way.
 * 
 * @preconditions: myStatus is initialized, Serial port is active
 * @postconditions: none
 */ 
void printGardenStatus(){
  // print number of times executed
  Serial.println(F("")); Serial.println(statusCounter++);

  if(gardenStatus.isAwake == false) Serial.println(F("GARDEN IS IN STANDBY"));

  // TODO
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

  // initaialize all the pins
  initPins();

  // flash LEDR once to indicate the beginning of the setup process
  flashLED(LEDR, 1, 500);
  
  // begin serial communication
  Serial.begin(BAUD_RATE);
  while (!Serial);
  
  // check if button is being pressed; if so, do special startup
  if(digitalRead(BUTTON) == 0){
    flashLED(LEDG, 3, 125);

    // do special stuff
  }

  // initialize gardenStatus
  initGardenStatus();

  // TODO print status?

  // Setup mesh
  mesh.setNodeID(MASTER_NODE_ID);
  while(!mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT)){
    Serial.println(F("Trouble setting up the mesh, trying again..."));
    delay(1000);
  }
  Serial.println(F("Mesh created"));
  mesh.setAddress(MASTER_NODE_ID, MASTER_ADDRESS);


  // init timer for regular system checks
  Timer1.initialize(TIMER1_PERIOD);
  Timer1.attachInterrupt(updateStatusISR);

  initPins2();
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
  mesh.DHCP();

  // update node status if necessary
  if(updateStatusFlag){
    updateGardenStatus();
    printGardenStatus();

    // reset the flag
    updateStatusFlag = false;
  }

  // check if need to open/close valves according to schedule
  checkSchedule();

  // if had buttonpress, toggle between awake and asleep
  if(hadButtonPress){
    // toggle states between asleep and awake
    gardenStatus.isAwake = !gardenStatus.isAwake;

    // if asleep, tell valves to shut
    if(gardenStatus.isAwake == false){
      // TODO set light sequence
      
      // TODO for each node, shut off all valves
    }
    else{
      // TODO set light sequence
    }

    // TODO report state to the server/website?
    
    // reset flag
    hadButtonPress = false;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// INSERT 3G STUFF HERE /////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // read in and respond to mesh messages
  readMeshMessages();
}

