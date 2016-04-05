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
#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>
#include <Time.h>
// RF24 Mesh Network
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
// Schedule Class
#include "Schedule.h"
#include "ScheduleEvent.h"
#include <TimerOne.h>
// Modify protocol header file directory as needed
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/RadioWork/Shared/settings.h"

/******************************************************************************
***************************** Global Variables ********************************
*******************************************************************************/
/* Randos */
bool dummyValveCommand = true;
char networkRequestChar;
int myNodeID = 0;
int counter = 0;
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
#define ThreeG        12
#define Modem_Serial  Serial1
/******************************* Node IDs **************************************/
#define KEVINID 1
/******************* Configure RF24 CE & CS ************************************/
RF24 radio(CE,CS);
RF24Network network(radio);
RF24Mesh mesh(radio,network);
/********************************* Modem ***************************************/
String currentString = "";
int incomingByte = 0;

/******************************** Timer ***************************/
list<int> currentEvents;
/*************** ISR Routine ***************/
volatile uint8_t interruptCounter = 0;
time_t previousSec = 0;

/******************************* RESET ***************************************/
#define RESET_GND A0
#define RESET_PIN A1

/* the setup function configures and initializes the I/O devices: LEDs, push button, serial port, 
 * and 3G Modem
*/
void setup() {
  setupPinMode();

  // flash LEDR once to indicate the beginning of the setup process
  flashLEDR(1, 500);

  // initialize serial debug port
  Serial.begin(9600);
  while (!Serial) ; 

  // initialize 3G Modem
  setupModem();
  getModemIP();
  
  // flash LEDR twice at this point 
  flashLEDR(2, 250);

  openSocket();

  // flash LEDR three times at this point 
  flashLEDR(3, 250);

  setupMesh();

  // flash LEDR four times at this point
  flashLEDR(4, 500);

  selfReset();

  // flash LEDR two times at this point
  flashLEDR(2, 1000);

  // attach interrupt to the function that prints out current network status
  Timer1.initialize(5000000);
  Timer1.attachInterrupt(printStatus);
  
  // flash LEDG to indicate the end of the setup
  flashLEDG(1,1000);
}

void loop() {
  while(Modem_Serial.available() > 0) {
    getModemResponse();
  }
  if(currentString == "NO CARRIER"){
    openSocket();
  }
  schedule();
  updateMesh();
}


/*******************************************************************************************
****************************** Function Declarations ***************************************
*******************************************************************************************/

/* This function manages the mesh network as needed
 * @param: none
 * @return: none
*/
void updateMesh() {
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
    int networkStatusRequest;
    
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
        if (networkStatusRequest == 1){
          Serial.println("We're solid");
        } else if (networkStatusRequest == 2) {
          Serial.println("There's a low battery warning");
        } else if (networkStatusRequest == 3){
          Serial.println("There's a valve error");
        } else {
          Serial.println("Something went terribly wrong with the valveResponse...");
        }
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
    mesh.write(mesh.getAddress(KEVINID), &dummyValveCommand, SET_VALVE_H, sizeof(dummyValveCommand));
    Serial.print("Sending the valve command: "); Serial.println(dummyValveCommand);
    dummyValveCommand = !dummyValveCommand;
    hadButtonPress = false;
  }
}

void schedule() {
  if(currentString == "true") {
    Serial.println("Message received. Now I need to send something to Kevin...");
    hadButtonPress = true;
  }
}

/* This function prints out the current status of the mesh network to the serial port
 * @param: none
 * @return: none
*/
void printStatus(){
  Serial.println("");
  Serial.println(printcount++);
  Serial.print("\nMy ID:      ");
  Serial.println(mesh.getNodeID());
  Serial.print("My address: ");
  Serial.println(mesh.getAddress(myNodeID));
  Serial.println(" ");
  Serial.println(F("********Assigned Addresses********"));
  for(int i=0; i<mesh.addrListTop; i++){
    Serial.print("NodeID: ");
    Serial.print(mesh.addrList[i].nodeID);
    Serial.print(" RF24Network Address: 0");
    Serial.println(mesh.addrList[i].address,OCT);
 }
  // Serial.println(F("**********************************"));
}

/* This function flashes LEDR based on the arguments
 * @param: myNum, the number of times you want LEDR to flash
 * @param: myTime, the duration of LEDR's on/off state when it flashes
*/
void flashLEDR(int myNum, int myTime) {
  for (int i = 0; i < myNum; i++) {
    digitalWrite(LEDR, HIGH);
    delay(myTime);  
    digitalWrite(LEDR, LOW);
    delay(myTime);
  }  
}

/* This function flashes LEDG based on the arguments
 * @param: myNum, the number of times you want LEDG to flash
 * @param: myTime, the duration of LEDG's on/off state in miliseconds when it flashes
*/
void flashLEDG(int myNum, int myTime) {
  for (int i = 0; i < myNum; i++) {
    digitalWrite(LEDG, HIGH);
    delay(myTime);  
    digitalWrite(LEDG, LOW);
    delay(myTime);
  }  
}

/********************************* Setups ************************************/

/*  This function sets up the pin modes
 *  @param: none
 *  @return: none
*/
void setupPinMode() {
  // setup pin modes
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  // attach interrup to push button
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);

  // RESET_PIN -- set to low immediately to discharge capacitor
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
}

/* This function sets up the 3G modem and ensures that it connects to the 
 * network
 * @param: none
 * @return: none
*/
void setupModem() {
  Serial.println("Starting Cellular Modem");
  // properly toggle 3G Modem I/O pin 
  digitalWrite(ThreeG, LOW);
  pinMode(ThreeG, OUTPUT);
  digitalWrite(ThreeG, LOW);
  delay(1100); // modem requires >1s pulse
  // return I/O pin 12 to input/hi-Z state  
  pinMode(ThreeG, INPUT);

  // initialize serial port to communicate with modem
  Modem_Serial.println("Initializing modem COM port...");
  Modem_Serial.begin(115200);
  while (!Modem_Serial);

  // Soft reset of modem
  Serial.println("Reseting modem");

  Modem_Serial.println("AT#SGACT=1,0");
  delay(250);
  while(PrintModemResponse() > 0);
  
  Modem_Serial.println("ATZ");
  delay(500);
  while(PrintModemResponse() > 0);

  // Connect to 3G cellular network
  Serial.println("Waiting for network connection...");
  boolean connectionGood = false;
  while(!connectionGood){
    Modem_Serial.println("AT+CREG?");
    currentString = "";
    delay(500);
    while(Modem_Serial.available() > 0) {
      getModemResponse();
      // check currentString to see if network status is "0,1" or "0,5" which means we are connected
      if((currentString.substring(currentString.length()-3, currentString.length()) == "0,1") || 
        (currentString.substring(0, currentString.length()) == "0,5")) {
        connectionGood = true;
        Serial.println(); 
        Serial.println("Connection successful!");  
      }
    }
  }
  while(PrintModemResponse() > 0);
}

void setupMesh() {
  // radio.setPALevel(RF24_PA_LOW);
  // Set the nodeID to 0 for the master node
  mesh.setNodeID(myNodeID);
  // Serial.println(mesh.getNodeID());
  // Connect to the mesh
  Serial.print("Output of mesh.begin(): ");
  // Serial.println(mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT));
  // mesh.requestAddress(0);
  mesh.setAddress(0-48,0);
}

void selfReset() {
  // enable self-resetting ability
  delay(50);  // allow capacitor to discharge if was previously charged before enabling autoreset again
              // 95% = 3*tau = 3*RC = 3*200*100*10^-6 = 60ms -- but never gets fully charged, and has
              //    been dicharging during previous setup, so 50ms is sufficient
  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_GND, OUTPUT);
  digitalWrite(RESET_GND, LOW);
}

/************************************ Modem Specific *******************************************/

/* This function gets an IP address for the 3G modem by resetting the modem's
 * PDP context once
 * @param: none
 * @return: none
*/
void getModemIP() {
  //  setup TCP socket
  Modem_Serial.println("AT#SCFG=1,1,0,0,600,2");
  delay(500);
  while(PrintModemResponse() > 0);

  Modem_Serial.println("AT#TCPMAXDAT=1420");
  delay(500);
  while(PrintModemResponse() > 0);

  Modem_Serial.println("at#sgact=1,1");
  delay(500);
  while(PrintModemResponse() > 0); 
  delay(5000);
  while(PrintModemResponse() > 0);  
}

/* This function opens a TCP socket to the GardeNet server and send a message to the server
 * @param: none
 * @return: none
*/
void openSocket() {
  // initiate TCP socket dial
  Modem_Serial.println("AT#SD=1,0,5531,\"gardenet.ddns.net\",0,0");
  // Modem_Serial.println("AT#SD=1,0,13,\"time.nist.gov\",255,0");
  delay(500);
  while(PrintModemResponse() > 0);
}

/* This function disconnects the 3G modem from the network and reports the data usage from 
 * last the previous session as well as total data usage through the serial port
 * @param: none
 * @return: none
*/
void disconnectModem() {
  Modem_Serial.println("AT#SGACT=1,0");
  delay(250);
  while(PrintModemResponse() > 0);
  delay(2000);
  
  // monitor data usage since last reset: <cid>, <tot>, <sent>, <received>
  // mode 0: reset data counter
  // mode 1: report data from last session
  // mode 2: report data from last reset (of the data counter) 
  Modem_Serial.println("AT#GDATAVOL=1");
  delay(250);
  while(PrintModemResponse() > 0);
  Modem_Serial.println("AT#GDATAVOL=2");
  delay(250);
  while(PrintModemResponse() > 0);
}

/* This function prints the modem response and monitors the availability of
 * TX1/RX1, which are used by the modem
 * @param: none
 * @return: Modem_Serial.available(), 1 if more bytes are still avaialble
 *          in the UART buffer, 0 if false
*/
int PrintModemResponse() {
  while(Modem_Serial.available() > 0) {
    //read incoming byte from modem and write byte out to debug serial over USB
    // Serial.write(Modem_Serial.read());
    getModemResponse();
  } 
  // Serial.println();
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Modem_Serial.available();
}

/* This function prints the incoming data from the modem to the Serial monitor
 * and builds a volatile string which is cleared whenever a new line is detected
 * @param: none
 * @return: none
*/
void getModemResponse() {
  incomingByte = Modem_Serial.read();
  if(incomingByte != -1) {
    Serial.write(incomingByte); 
  }
  if(incomingByte != '\n') {
    currentString += char(incomingByte);  
  } else {
    currentString = "";
  }
}

/************************************ RESET ***********************************/
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

