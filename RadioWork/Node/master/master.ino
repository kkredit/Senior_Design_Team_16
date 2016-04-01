/*
 * Copyright: Charles Kingston, Kevin Kredit and Anthony Jin
 * 
 * Arduino Leonardo code that sets up a mesh network. After setting up the 
 * mesh network it becomes the master on the network. This sketch also 
 * contains code to read data coming in from a flow sensors as pulses.
 */

/******************************************************************************
***************************** Includes ****************************************
*******************************************************************************/
//#include <StandardCplusplus.h>
//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>
//#include <Time.h>
// RF24 Mesh Network
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
// Schedule Class
//#include "Schedule.h"
//#include "ScheduleEvent.h"
// Interrupt
#include <TimerOne.h>
// Modify protocol header file directory as needed
//#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/RadioWork/Shared/settings.h"
#include "C:\\Users\\kevin\\Documents\\Senior_Design_Team_16\\RadioWork\\Shared\\SharedDefinitions.h"

/******************************************************************************
***************************** Global Variables ********************************
*******************************************************************************/
/* Randos */
bool dummyValveCommand = true;
char networkRequestChar;
int myNodeID = 0;
int counter = 0;
volatile bool updateStatusFlag = false;
/************************ MESH Network Related *********************************/
uint32_t displayTimer = 0;
int printcount = 0;
bool hadButtonPress = false;
bool defaultNetworkBlink = false;

/******************************* I/O Ports *************************************/
#define BUTTON        3
#define LEDR          4
#define LEDG          5
#define CE            7
#define CS            8

#define RESET_PIN A1////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RESET_GND A0////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/******************************* Node IDs **************************************/
#define KEVINID 1

/******************* Configure RF24 CE & CS ************************************/
RF24 radio(CE,CS);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

void resetAllNodes(){
  uint16_t address;
  uint8_t payload = 1;
  for(address = 1; address < 512; address++){
    mesh.write(address, &payload, FORCE_RESET_H, sizeof(payload));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* the setup function configures and initializes the I/O devices: LEDs, push button, serial port, 
 * and 3G Modem
*/
void setup() {

  // RESET_PIN -- set to low immediately to discharge capacitor////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(RESET_PIN, OUTPUT);////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  digitalWrite(RESET_PIN, LOW);////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  // setup pin modes
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  //while(BUTTON);    // wait for a button press to continue
  // attach interrup to push button
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING);  
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);

  // flash LEDR to indicate the setup process
  digitalWrite(LEDR, HIGH);
  delay(500);
  digitalWrite(LEDR, LOW);
  
  
  //radio.setPALevel(RF24_PA_LOW);

  // initialize serial debug port
  Serial.begin(9600);
  //while (!Serial);

  // initialize 3G Modem
  // setupModem();

  // flash LEDR twice at this point 
//  digitalWrite(LEDR, HIGH);
//  delay(250);
//  digitalWrite(LEDR, LOW);
//  delay(250);
//  digitalWrite(LEDR, HIGH);
//  delay(250);
//  digitalWrite(LEDR, LOW);
//  delay(250);

  // Set the nodeID to 0 for the master node
  mesh.setNodeID(myNodeID);
  Serial.println(mesh.getNodeID());
  // Connect to the mesh
  Serial.print("Output of mesh.begin(): ");
  Serial.println(mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT));
  //mesh.requestAddress(0);
  mesh.setAddress(0,0);

  /*Serial.print("Resetting all nodes...");
  resetAllNodes();
  Serial.println(" done.");*/

  // flash LEDR three at this point in the setup process
//  digitalWrite(LEDR, HIGH);
//  delay(250);
//  digitalWrite(LEDR, LOW);
//  delay(250);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);
  delay(250);
  digitalWrite(LEDR, HIGH);
  delay(250);
  digitalWrite(LEDR, LOW);

  // attach interrupt to the function that prints out current network status
  Timer1.initialize(5000000);
  Timer1.attachInterrupt(updateStatusISR);

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // enable self-resetting ability
  delay(50);  // allow capacitor to discharge if was previously charged before enabling autoreset again
              // 95% = 3*tau = 3*RC = 3*200*100*10^-6 = 60ms -- but never gets fully charged, and has
              //    been dicharging during previous setup, so 50ms is sufficient
  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_GND, OUTPUT);
  digitalWrite(RESET_GND, LOW);
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void loop(){
  // refresh the reset////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  refreshReset();////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  // Call mesh.update to keep the network updated
  mesh.update();
  
  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
  
  
  // Check for incoming data from the sensors
  if(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    Serial.print(F("Received ")); Serial.print(char(header.type)); Serial.println(F(" type message."));
    Serial.print(F("From [logical address]: ")); Serial.println(header.from_node);
    Serial.print(F("From [converted to nodeID]: ")); Serial.println(mesh.getNodeID(header.from_node));
    
    uint32_t dat=0;
    bool boolMessage;
    float flowSensorData = -1;
    // idk if initializing this value is allowed...
    int valveResponse = -1;
    uint8_t networkStatusRequest;
    
    switch(header.type){
      // valve flowrate sensor 
      case SEND_FLOW_RATE_H:
        network.read(header,&flowSensorData,sizeof(flowSensorData));
        if (flowSensorData == float(-1)) {
          Serial.println("Something went terribly wrong with the flowSensorData...");
        } else {
          Serial.print(flowSensorData); 
          Serial.println(" GPM");
        }
        break;
      // read a valve response from one of the nodes
      case SEND_VALVE_H:
        network.read(header, &valveResponse, sizeof(valveResponse));
        if (valveResponse == -1){
          Serial.println("The status is unknown");
        } else if (valveResponse == 0) {
          Serial.println("The valve is closed");
        } else if (valveResponse == 1){
          Serial.println("The valve is open");
        } else {
          Serial.println("Something went terribly wrong with the valveResponse...");
        }
        break;
      case SEND_NODE_STATUS_H:
        network.read(header, &networkStatusRequest, sizeof(networkStatusRequest));
//        if (networkStatusRequest == 1){
//          Serial.println("We're solid");
//        } else if (networkStatusRequest == 2) {
//          Serial.println("There's a low battery warning");
//        } else if (networkStatusRequest == 3){
//          Serial.println("There's a valve error");
//        } else {
//          Serial.println("Something went terribly wrong with the valveResponse...");
//        }
        break;
      default: 
        //network.read(header,0,0); 
        defaultNetworkBlink = !defaultNetworkBlink;
        digitalWrite(LEDR, defaultNetworkBlink);
        network.read(header,&dat,sizeof(dat)); 
        Serial.print("Message recieved: "); Serial.print(dat); Serial.println(", but is undefined");
        break;
    }
  }
  
  if (hadButtonPress){
    //mesh.write(mesh.getAddress(KEVINID), &dummyValveCommand, SET_VALVE_H, sizeof(dummyValveCommand));
    Serial.print("Sending the valve command: "); Serial.println(dummyValveCommand);
    dummyValveCommand = !dummyValveCommand;
    hadButtonPress = false;
    //hardReset();
  }

  if(updateStatusFlag){
    updateStatus();
    updateStatusFlag = false;
  }
}

/*
 * Regardless of current state, send message to other to turn on LEDY
 */
void handleButton(){
  if (counter > 0){
    hadButtonPress = true;
    Serial.println(F("Detected Button Press"));
  } else {
    counter++;
  }
}

uint8_t getNodeStatus(uint8_t nodeID){
  return 0;
//  if(nodeID == MASTER_ADDRESS) return NODE_IS_MASTER;
//  
//  char payload = GET_NODE_STATUS_P;
//  mesh.write(mesh.getAddress(nodeID), &payload, INFO_REQUEST_H, sizeof(payload));
//
//  // wait for response (not the best way to implement...
//  RF24NetworkHeader header;
//  uint16_t startTime = millis();
//  // TODO this part doesn't work for some reason
//  while(header.type != SEND_NODE_STATUS_H){
//    network.peek(header);
//    if(millis() - startTime > 15000) return NODE_DISCONNECTED;
//  }
//  
//  uint8_t nodeStatus;
//  network.read(header, &nodeStatus, sizeof(nodeStatus));
//  return nodeStatus;
}

void updateStatusISR(){
  updateStatusFlag = true;
}

void updateStatus(){
  Serial.println(""); Serial.println(printcount++);
  Serial.print("\nMy ID:      "); Serial.println(MASTER_ADDRESS);
  Serial.print("My address: "); Serial.println(MASTER_ADDRESS);
  Serial.println(F("\n********Assigned Addresses********"));
  for(int i=0; i<mesh.addrListTop; i++){
    Serial.print("NodeID: ");
    Serial.print(mesh.addrList[i].nodeID);
    Serial.print(" RF24Network Address: 0");
    Serial.println(mesh.addrList[i].address);
    Serial.print("   ");
    /*switch(getNodeStatus(mesh.addrList[i].nodeID)){
    case NODE_IS_MASTER:
      //Serial.println(myStatus);
      Serial.println("Me, the gateway");
      break;
    case NODE_OK:
      Serial.println("OK");
      break;
    case NODE_DISCONNECTED:
      Serial.println("DISCONNECTED");
      break;
    case NODE_VALVE_ERROR:
      Serial.println("VALVE ERROR");
      break;
    case NODE_LOW_BATTERY:
      Serial.println("LOW BATTERY");
      break;
    default:
      break;
    }*/
 }
  Serial.println(F("**********************************"));
}
