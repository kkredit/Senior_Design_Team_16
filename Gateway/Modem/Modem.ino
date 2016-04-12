/*
 * Copyright: Charles Kingston, Kevin Kredit and Anthony Jin
 * 
 * Arduino Leonardo code that sets up a mesh network. After setting up the 
 * mesh network it becomes the master on the network. This sketch also 
 * contains code to read data coming in from a flow sensors as pulses.
 */

#include <StandardCplusplus.h>
// Schedule Class
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/Schedule.h"
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/Schedule.cpp"
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/ScheduleEvent.h"
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/ScheduleEvent.cpp"

#define ThreeG        12
#define Modem_Serial  Serial1

/********************************* Modem ***************************************/
String currentString = "";
volatile int incomingByte = 0;

/******************************** JSON Parsing **********************************/
ScheduleEvent tempEvent;
Schedule weeklySchedule;

/* the setup function configures and initializes the I/O devices: LEDs, push button, serial port, 
 * and 3G Modem
*/
void setup() {
  Serial.begin(9600);
  while (!Serial);
  setupModem();
  getModemIP();
  openSocket();
}

void loop() {
  // while(PrintModemResponse() > 0);
  while(Modem_Serial.available() > 0) {
    getModemResponse();
    parseJSON();
  }

    if(currentString == "DONE") {
      while(!weeklySchedule.isEmpty(1)) {
        checkCurrentSchedule(1);
      }
    }
    
    // checkCurrentSchedule();

  // exceptions
  if(currentString == "NO CARRIER" || currentString == "ERROR") {
    openSocket();
    currentString = "";
  }
}


/*******************************************************************************************
****************************** Function Declarations ***************************************
*******************************************************************************************/



/********************************* Setups ************************************/

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
//  Serial.println("Reseting modem");
//  Modem_Serial.println("ATZ");
//  delay(500);
//  while(PrintModemResponse() > 0);

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
        currentString = "";
      }
    }
  }
  while(PrintModemResponse() > 0);
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

//  Modem_Serial.println("AT#TCPMAXDAT=1420");
//  delay(500);
//  while(PrintModemResponse() > 0);

  Modem_Serial.println("at#sgact=1,1");
  delay(500);
  while(PrintModemResponse() > 0); 
  
  // wait for 10s for the modem to retrieve IP address
  delay(10000);
  while(PrintModemResponse() > 0);  
}

/* This function opens a TCP socket to the GardeNet server and send a message to the server
 * @param: none
 * @return: none
*/
void openSocket() {
  // initiate TCP socket dial
  Modem_Serial.println("AT#SD=1,0,5530,\"gardenet.ddns.net\",0,0");
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
    getModemResponse();
  } 
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
  // NO CARRIER is an exception in which we need to reopen the socket
  if(incomingByte == '\n' && currentString != "NO CARRIER") {
    currentString = "";
  } else if(currentString != "NO CARRIER") {
    currentString += char(incomingByte);  
  } else {
    currentString = currentString;
  }
}

/***************************************** JSON ******************************************/

/* JSON format: {"start_time" : "1.10", "stop_time" : "2.30", "day" : "Monday", "zone_ID" : "3"}
 *  interested in the 3, 7, 11, 15th items that we split
 *  3: start time
 *  7: stop time
 *  11: day of the week
 *  15: zone/node ID
   Full garden operation: true, false
*/
void parseJSON() {
  if ((currentString.substring(currentString.length()-15,currentString.length()-8) == "zone_ID")) {
    Serial.println("");
    Serial.println("I got a scheduling event!");
    createEvent();
  } else if (currentString == "true") {
    Serial.println("");
    Serial.println("I need to turn on the full garden.");
  } else if (currentString == "false") {
    Serial.println("");
    Serial.println("I need to shut down the full garden.");
  }
}

void createEvent() {
  int myDay = 0;
  int beginIdx = 0;
  // used to process start time and end time further
  // split the string by double quote
  int idx = currentString.indexOf("\"");
  char charBuffer[16];
  String arg;

  for(int i = 0; i <= 15; i++) {
    arg = currentString.substring(beginIdx, idx);
    arg.toCharArray(charBuffer, 16);
          
    // add error handling for atoi:
    // eventArray[i] = atoi(charBuffer);
    beginIdx = idx + 1;
    idx = currentString.indexOf("\"", beginIdx);
    // Serial.println(charBuffer);

    // only four pieces of information are needed
    if(i == 3) {
      char* separator = strchr(charBuffer, '.');
      *separator = 0;
      int myHour = atoi(charBuffer);
      separator = separator + 1;
      int myMin = atoi(separator);
      tempEvent.setStartHour(myHour);
      tempEvent.setStartMin(myMin);
    } else if (i == 7) {
      char* separator = strchr(charBuffer, '.');
      *separator = 0;
      int myHour = atoi(charBuffer);
      separator = separator + 1;
      int myMin = atoi(separator);
      tempEvent.setEndHour(myHour);
      tempEvent.setEndMin(myMin);
    } else if (i == 11) {
      myDay = dayDecoder(arg);
      Serial.print("This event is to be inserted into "); Serial.println(myDay);
    } else if (i == 15) {
      int myID = atoi(charBuffer);
      tempEvent.setNodeID(myID);
    }
  }
//  Serial.print("Valeve "); Serial.print(tempEvent.getNodeID()); Serial.print(" is set to have a start time of ");
//  Serial.print(tempEvent.getStartHour()); Serial.print(":"); Serial.print(tempEvent.getStartMin());
//  Serial.print(" and an end time of "); Serial.print(tempEvent.getEndHour()); Serial.print(":"); Serial.print(tempEvent.getEndMin());
//  Serial.println(".");

  // insert this event to the weekly schedule
  weeklySchedule.insert(myDay, tempEvent);

  // checkCurrentSchedule(myDay);
}

void checkCurrentSchedule( int myDay) {
  ScheduleEvent myEvent = weeklySchedule.popFrontStartTime(myDay);
  Serial.println("");
  Serial.print("Valve "); Serial.print(myEvent.getNodeID()); Serial.print(" is set to have a start time of ");
  Serial.print(myEvent.getStartHour()); Serial.print(":"); Serial.print(myEvent.getStartMin());
  Serial.print(" and an end time of "); Serial.print(myEvent.getEndHour()); Serial.print(":"); Serial.print(myEvent.getEndMin());
  Serial.println(".");
}

int dayDecoder(String myDay) {
  if(myDay == "Monday") {
    return 1;
  } else if (myDay == "Tuesday") {
    return 2;
  } else if (myDay == "Wednesday") {
    return 3;
  } else if (myDay == "Thursday") {
    return 4;
  } else if (myDay == "Friday") {
    return 5;
  } else if (myDay == "Saturday") {
    return 6;
  } else if (myDay == "Sunday") {
    return 0;
  } else { // then somethign is not right...
    return 11;
  }
}










