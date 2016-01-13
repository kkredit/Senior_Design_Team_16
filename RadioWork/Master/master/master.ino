/*
 * Copyright: Charles Kingston and Kevin Kredit
 * 
 * Arduino Leonardo code that sets up a mesh network. After setting up the 
 * mesh network it becomes the master on the network. This sketch also 
 * contains code to read data coming in from a flow sensors as pulses.
 */
  
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
//Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>
#include <TimerOne.h>

/***** Configure the chosen CE,CS pins *****/
RF24 radio(9,10);
RF24Network network(radio);
RF24Mesh mesh(radio,network);
/*******************************************/

/********** MESH network vars **************/
uint32_t displayTimer = 0;
int printcount = 0;
bool hadButtonPress = false;
bool LEDYstate = false;
bool defaultNetworkBlink = false;
/*******************************************/

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

/***** Configure I/O ports with #define ****/
#define BUTTON  3
#define LEDR    4
#define LEDG    5
#define LEDY    6
/*******************************************/

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDY, OUTPUT);
  //while(BUTTON); /////////// wait for a button press to continue, allowing for user to setup terminal
  delay(5000);
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButton, FALLING);  
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDY, LOW);

  digitalWrite(LEDR, HIGH);
  delay(500);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, HIGH);
  
  Serial.begin(9600);
  //radio.setPALevel(RF24_PA_LOW);
  // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());
  // Connect to the mesh
  Serial.print("Output of mesh.begin(): ");
  Serial.println(mesh.begin(12, RF24_250KBPS, 2000));
  //mesh.requestAddress(0);
  mesh.setAddress('0'-48, 1);

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
}

/*
 * Regardless of current state, send message to other to turn on LEDY
 */
void handleButton(){
    hadButtonPress = true;
    Serial.println(F("Deteceted Button Press"));
}

void printStatus(){
  Serial.println("");
  Serial.println(printcount++);
  Serial.print("\nMy ID:      ");
  Serial.println(mesh.getNodeID());
  Serial.print("My address: ");
  Serial.println(mesh.getAddress(0));
  Serial.println(" ");
  Serial.println(F("********Assigned Addresses********"));
  for(int i=0; i<mesh.addrListTop; i++){
    Serial.print("NodeID: ");
    Serial.print(mesh.addrList[i].nodeID);
    Serial.print(" RF24Network Address: 0");
    Serial.println(mesh.addrList[i].address,OCT);
 }
  Serial.println(F("**********************************"));
}

void loop() {    

  // Call mesh.update to keep the network updated
  mesh.update();
  
  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
  
  
  // Check for incoming data from the sensors
  if(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    
    uint32_t dat=0;
    bool boolMessage;
    float flowSensorData;
    switch(header.type){
      // Display the incoming millis() values from the sensor nodes
//      case 'M': 
//        Serial.println(dat); 
//        break;
      case 'B':
        network.read(header,&flowSensorData,sizeof(flowSensorData));
        Serial.print(flowSensorData); 
        Serial.println(F(" liters"));
        break;
      default: 
        network.read(header,0,0); 
        Serial.println("Message on network, but not reading it");
        defaultNetworkBlink = !defaultNetworkBlink;
        digitalWrite(LEDR, defaultNetworkBlink);
        network.read(header,&dat,sizeof(dat)); 
        break;
    }
  }
  
  if(millis() - displayTimer > 5000){
    displayTimer = millis();
    Serial.println(" ");
    Serial.println(F("********Assigned Addresses********"));
     for(int i=0; i<mesh.addrListTop; i++){
       Serial.print("NodeID: ");
       Serial.print(mesh.addrList[i].nodeID);
       Serial.print(" RF24Network Address: 0");
       Serial.println(mesh.addrList[i].address,OCT);
     }
    Serial.println(F("**********************************"));
  }
}
