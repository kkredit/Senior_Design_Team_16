
/** RF24Mesh_Example.ino by TMRh20
 *
 * This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
 * master node.
 * The nodes will refresh their network address as soon as a single write fails. This allows the
 * nodes to change position in relation to each other and the master node.
 */


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
//#include <printf.h>
#include <TimerOne.h>

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
 * User Configuration: nodeID - A unique identifier for each radio. Allows addressing
 * to change dynamically with physical changes to the mesh.
 *
 * In this example, configuration takes place below, prior to uploading the sketch to the device
 * A unique value from 1-255 must be configuLEDR for each node.
 * This will be stoLEDR in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.
 *
 **/
#define nodeID  04
#define BUTTON  3
#define LEDY    6
#define LEDR    5

uint32_t displayTimer = 0;
int printcount = 0;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDY, OUTPUT);
  //while(BUTTON); /////////// wait for a button press to continue, allowing for user to setup terminal
  delay(5000);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, LOW);

  digitalWrite(LEDR, HIGH);
  delay(500);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDY, HIGH);

  Serial.begin(9600);
  radio.setPALevel(RF24_PA_LOW);
  //printf_begin();
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println("Connecting to the mesh...");
  Serial.print("Output of mesh.begin(): ");
  Serial.println(mesh.begin(12, RF24_250KBPS, 2000));
  //mesh.setAddress(nodeID, nodeID+1); 

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

void printStatus(){
  Serial.println("\n");
  Serial.println(printcount++);
  Serial.print("\nMy ID:      ");
  Serial.println(mesh.getNodeID());
  Serial.print("My address: ");
  Serial.println(mesh.getAddress(0));
}

void loop() {

  mesh.update();

  // Send to the master node every second
  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();

    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&displayTimer, 'M', sizeof(displayTimer))) {

      // If a write fails, check connectivity to the mesh network
      if ( ! mesh.checkConnection() ) {
        //refresh the network address
        Serial.println("Renewing Address");
        mesh.renewAddress();
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK: "); Serial.println(displayTimer);
    }
  }

  while (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
}






