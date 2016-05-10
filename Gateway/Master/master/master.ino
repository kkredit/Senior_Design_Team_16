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
 * Last Modified: 4/18/16
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

#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/Gateway/SharedDefinitions.h"
//#include "C:/Users/kevin/Documents/Senior_Design_Team_16/Gateway/SharedDefinitions.h"
#include "StandardCplusplus.h"
//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>
#include <Time.h>
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/Schedule.h"
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/Schedule.cpp"
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/ScheduleEvent.h"
#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/ScheduleEvent.cpp"
//#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/Schedule.h"
//#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/Schedule.cpp"
//#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/ScheduleEvent.h"
//#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/ScheduleEvent.cpp"

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

// 3G Modem 
String currentString = "";
String JSONString = "";
String alertSetting = "";
String demoString = "";

///// flags ////
// ISR
volatile bool hadButtonPress = false;
volatile bool updateStatusFlag = false;
// others
bool modemStartReceiving = false;
bool modemReceivingJSON = false;
bool gotAllEvents = false;
bool doDailySchedule = true;
bool sentNewDayAlertToNodes = false;
bool calledIsNewDay = false;

// other
Garden_Status gardenStatus;
Schedule weeklySchedule;
uint16_t statusCounter = 0;
uint16_t eventCount = 0;
uint16_t eventMaxCount = 0;
time_t lastTime = 0;



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////     Demo        ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


struct demoStruct{
  uint8_t mode;// = 0;
  bool v1;// = false;
  bool v2;// = false;
  bool v3;// = false;  
};

demoStruct forDemo;



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
void handleButtonISR() {
  // gets rid of startup false positive by ignoring for a few seconds after startup
  //if(statusCounter > 0) {
    hadButtonPress = true; 
    //Serial.println(F("\n[Detected buttonpress]"));
  //}
}


/* 
 * updateStatusISR()
 * 
 * ISR called when timer1 times out; sets a flag and exits
 * 
 * @preconditions: timer interrupt must be enabled
 * @postconditions: updateNodeStatus flag is set
 */
void updateStatusISR() {
  updateStatusFlag = true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////    3G Modem     ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/* 
 * setupModem()
 *  
 * This function sets up the 3G modem and ensures that it connects to the network
 * 
 * @preconditions: modem is not connected to the network
 * @postconditions: modem is connected to the network
*/
void setupModem() {
  Serial.println("Starting Cellular Modem"); 
  // initialize serial port to communicate with modem
  Serial.println("Initializing modem COM port...");
  // delay(1000);
  Modem_Serial.begin(115200);
  while(!Modem_Serial);

  // Reset modem
  Serial.println("Reseting modem");
  
  Modem_Serial.println("AT#MODE=RESET");
  delay(1000);
  while(PrintModemResponse() > 0);
  
  Modem_Serial.println("ATZ");
  delay(1000);
  while(PrintModemResponse() > 0);

  // Connect to 3G cellular network
  Serial.println("Waiting for network connection...");
  boolean connectionGood = false;
  while(!connectionGood) {
    Modem_Serial.println("AT+CREG?");
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


/*
 * checkDataUsae()
 * 
 * This function checks the modem's internal data counter
 * 
 * @postconditions: the modem reports the total bytes of data it used since last reset of the counter,
 *                  the total bytes sent, and the total bytes received
*/
void checkDataUsage() {
  // monitor data usage since last reset: <cid>, <tot>, <sent>, <received>
  // mode 0: reset data counter
  // mode 1: report data from last session
  // mode 2: report data from last reset (of the data counter) 
  Modem_Serial.println("AT#GDATAVOL=2");
  delay(250);
  while(PrintModemResponse() > 0);
}


/*
 * socketConfigs()
 * 
 * This function configurates three of the modem's sockets to be used as TCP sockets
 * 
 * @preconditions: the PDP context of the modem is activated
 * @postconditions: socket 1 and 2 of the modem are configured to be used for connection with the
 *                  GardeNet server and the NIST time server
*/
void socketConfigs() {
  // TCP socket 1: used for connection with server
  Modem_Serial.println("AT#SCFG=1,1,0,0,600,2");
  delay(1000);
  while(PrintModemResponse() > 0);

  // choose data view mode and set the keepalive timeout period to 5 minutes
  // COMMENT: the modem seems to be inactive if the keepalive timeout period is greater than 5 minutes
  Modem_Serial.println("AT#SCFGEXT=1,2,0,5,0,0");
  delay(1000);
  while(PrintModemResponse() > 0);

  // TCP socket 2: used for time initiation with NIST time server
  Modem_Serial.println("AT#SCFG=2,1,0,10,600,2");
  delay(1000);
  while(PrintModemResponse() > 0);
}


/* 
 * getModemIP()
 *  
 * This function gets an IP address for the 3G modem by resetting the modem's
 * PDP context once
 * 
 * @preconditions: the modem is connected to the 3G cell network, but does not have an IP
 * @postconditions: the modem retrieves its IP address or throws an error message if it
 * already has one
*/
void getModemIP() {
  // retrieve IP address
  Modem_Serial.println("at#sgact=1,1");
  delay(500);
  boolean IPGood = false;
  String queryString;
  while(!IPGood) {
    while(Modem_Serial.available() > 0) {
      getModemResponse();
      if(currentString == "#SGACT: \"") {
        bool isIP = false;
        while(!isIP) {
          int myByte = Modem_Serial.read();
          if(myByte != -1) {
            Serial.write(myByte);
            if(myByte == '\n') {
              isIP = true;
            } else if (myByte != '\"') {
              queryString += char(myByte);
            }
          }
        }
        queryString.toCharArray(gardenStatus.modemIP, 20);
        currentString = "";
      }
      // if the modem already has an IP or retrieved IP 
      if(currentString == "ERROR" || currentString == "OK") {
        IPGood = true;
      }
    }
  }
//  Serial.println();
//  Serial.print("The modem's IP address is: ");
//  Serial.println((String) gardenStatus.modemIP);
  while(PrintModemResponse() > 0);
}


/* 
 * openSocket()
 *  
 * This function opens a TCP socket to the GardeNet server and send a message to the server
 * 
 * @preconditions: the modem is connected to the network and has retrieved its IP address
 * @postconditions: the modem is connected to the GardeNet server via a TCP socket
*/
void openSocket() {
  // initiate TCP socket dial in command mode
  Modem_Serial.println("AT#SD=1,0,5530,\"gardenet.ddns.net\",255,0,1");
}


/*
 * decodeModemResponse()
 * 
 * This function decodes the current modem response and decide what action to take
 * 
 * @preconditions: currentString contains complete response
 * @postconditions: the corresponding action instruction is sent to handleModemOperation()
 * 
 * @return: an operation code (uint8_t) defined in SharedDefinitions.h
 */
uint8_t decodeModemResponse() {
  
  // connection error
  if (currentString == "ERROR") {
    return TR_G_ERROR;
  }

  // connection is good
  if (currentString == "OK") {
    return TR_G_CONNECTED;
  }

  // disable the interrupt
  if (currentString.substring(11, 16) == "START") {
    String maxCount = currentString.substring(16, currentString.length());
    eventMaxCount = maxCount.toInt();
    Serial.print("In this schedule, I should get "); Serial.print(maxCount); Serial.println(" events.");
    return TR_G_DISABLE_INT;
  }

  // change alert settings
  if (currentString.substring(12, 15) == "SMS") {
    alertSetting = currentString.substring(15, currentString.length());
    return TR_G_ALERT_SETTING;
  }

  // parse JSON when available
  if (currentString.substring(13,14) == "{") {
    // second comma, since first comma's index is also 9
    int idx = currentString.indexOf(',',10);
    int JSONLength = currentString.substring(9, idx).toInt();
    Serial.print("This JSON String needs to be ");
    Serial.print(JSONLength);
    Serial.println(" bytes long...");

    // buffer the available JSON string for error checking
    String myString = currentString.substring(currentString.indexOf('{'),currentString.length());
    
    // error checking: incomplete JSON would have an incorrect length
    if(JSONLength != myString.length()) {
      Serial.print("The actual JSON string is ");
      Serial.print(myString.length());
      Serial.println(" bytes long...");
      return TR_G_INCOMPLETE_JSON;
      
    // else the JSON string is good
    } else {
      JSONString = currentString.substring(currentString.indexOf('{'), currentString.indexOf('}')+1);
      return TR_G_JSON;
    }
  }

  // done receiving JSON strings
  if (currentString.substring(currentString.length()-4, currentString.length()) == "DONE") {
    return TR_G_ENABLE_INT;
  }

  // wake up garden
  if (currentString == "SRING: 1,4,true") {
    currentString = "";
    return TR_G_GARDEN_ON;
  }

  // make garden go to sleep
  if (currentString == "SRING: 1,5,false") {
    currentString = "";
    return TR_G_GARDEN_OFF;
  }

  // disable today's schedule
  if (currentString == "SRING: 1,8,NoEvents") {
    currentString = "";
    return TR_G_NO_EVENTS;
  }
  
  // socket connection lost
  if (currentString == "NO CARRIER") {
    return TR_G_DISCONNECTED;
  }

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////// DEMO ////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if (currentString.substring(12, 17) == "DEMO1") {
    demoString = currentString.substring(17, currentString.length());
    return DEMO1;
  } 

  // return no response message by default
  return TR_G_NO_RESPONSE; // else modem has no response
}


/*
 * handleModemOperation(uint8_t modemMode)
 * 
 * This function execute specific tasks based on the result of decodeModemResponse()
 * 
 * @preconditions: decodeModemResponse() returns a meaningful modem mode
 * @postconditions: the corresponding action is executed
*/
void handleModemOperation(uint8_t modemMode) {
  switch(modemMode) {

    case TR_G_DISABLE_INT:
      forDemo.mode = 0; // turn off demo 1 mode
      Timer1.detachInterrupt();
      updateStatusFlag = false;
      Serial.println("");
      Serial.println("Detected schedules!");
      Serial.println("Delete the old schedule...");
      deleteSchedule();
      modemReceivingJSON = true;
    break;

    case TR_G_ENABLE_INT:
      if(eventCount == eventMaxCount) {
        Serial.println("");
        Serial.println("Schedules are all set!");
        gotAllEvents = true;
        // check schedule now once
        if(gardenStatus.isAwake){
          checkSchedule();
        }
      } 
      else {
        Serial.println("");
        Serial.println("I did not get all schedule events... Now request another schedule from the server...");
        Serial.print("I'm supposed to get "); Serial.print((String) eventMaxCount); Serial.print(" events; ");
        Serial.print("but got "); Serial.print((String) eventCount); Serial.println(" events.");        
        gotAllEvents = false;
        // send a request to the server to get a new schedule: RESENDSCHEDULE
        checkAlerts(INCOMPLETE_JSON, 0);
      }
      // reset counter
      eventMaxCount = 0;
      eventCount = 0;
      // reenable interrupt
      Timer1.initialize(TIMER1_PERIOD);
      Timer1.attachInterrupt(updateStatusISR);
      modemReceivingJSON = false;
    break;

    case TR_G_JSON:
      Serial.print("The event I got is: ");Serial.println(JSONString);
      createEvent();
    break;

    case TR_G_GARDEN_ON:
      Serial.println("\nReceived 'Turn on' message from the server");
      gardenStatus.isAwake = true;
    break;

    case TR_G_GARDEN_OFF:
      Serial.println("\nReceived 'Go to sleep' message from the server");
      gardenStatus.isAwake = false;
      // TODO set light sequence
      // for each node, shut off all valves
      uint8_t node;
      for(node=1; node<=16; node++) {
        // if registered
        if(gardenStatus.nodeStatusPtrs[node] != NULL) {
          // if connected
          if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED) {
            // send command to turn off all valves
            Valve_Command vc;
            vc.whichValve = ALL_VALVES;
            vc.onOrOff = OFF;
            vc.timeToLive = VALVE_COMMAND_TTL;
            safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
          }
        }
      }    
    break;

    case TR_G_NO_EVENTS:
      Serial.println("\nDisable the next day's schedule events.");
      doDailySchedule = false;
    break;

    case TR_G_DISCONNECTED:
      gardenStatus.threeGState = TR_G_DISCONNECTED;
    break;

    case TR_G_CONNECTED:
      gardenStatus.threeGState = TR_G_CONNECTED;
    break;

    case TR_G_ALERT_SETTING:
      parseAlertSetting();
    break;

    case TR_G_ERROR:
      gardenStatus.threeGState = TR_G_ERROR;
    break;

    case TR_G_INCOMPLETE_JSON:
      // discard the corrupted JSON by the end of this funtion
    break;

    //////////////////////////////////////////////////////////////////////
    //////////////////////////////// DEMO ////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    case DEMO1:
      forDemo.mode = 1;
      parseDemoOneSetting();
      checkSchedule();
    break;

    // DEMO2 is the same as a normal schedule

    default:
      // UNUSED
    break;
  }
  // clear currentString after modem message is parsed and processed
  currentString = "";
}


/* 
 * PrintModemResponse()
 *  
 * This function prints the modem response and monitors the availability of
 * TX1/RX1, which are used by the modem
 * 
 * @return: Modem_Serial.available() -- 1 if more bytes are still avaialble
 *  in the UART buffer, 0 if false
 * @preconditions: there might be incoming bytes buffered in the modem
 * @postconditions: the modem writes out the incoming byte(s) to the serial debug port
 *  and returns a 0 or 1
*/
int PrintModemResponse() {
  while(Modem_Serial.available() > 0) {
    //read incoming byte from modem and write byte out to debug serial over USB
    Serial.write(Modem_Serial.read());
  } 
  Serial.println("");
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Modem_Serial.available();
}


/* 
 * getModemResponse()
 *  
 * This function prints the incoming data from the modem to the serial monitor
 * and builds a  string which is processed and then cleared whenever a new line is detected
 * 
 * @preconditions: there is a global string variable, currentString, waiting to be processed
 * @postconditions: currentString is concatenated with the incoming byte or cleared when a new
 *  line character is detected and the current message is not an exception
*/
void getModemResponse() {
  char incomingByte = Modem_Serial.read();
  Serial.print(incomingByte);
  // reset currentString at the end of a line if it is not a special case
  if(incomingByte == '\n') {
    currentString = "";
  } else if (incomingByte != -1) {
    currentString += incomingByte;
  } 
}



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////  JSON Parsing   ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


// {"start_time" : "1.10", "stop_time" : "2.30", "day" : "Monday", "zone_ID" : "3", "valve_num" : "1" }


/*
 * createEvent()
 * 
 * This function parse a JSON string and create a ScheduleEvent and insert it into the weekly schedule
 * 
 * @preconditions: currentString is a JSON string containing a schedule event
 * @postconditions: the schedule event is parsed and inserted into the weekly schedule
*/
void createEvent() {
  int myDay;
  int beginIdx = 0;
  ScheduleEvent tempEvent;
  // used to process start time and end time further
  // split the string by double quote
  int idx = JSONString.indexOf("\"");
  char charBuffer[16];
  String arg;

  for(int i = 0; i <= 99; i++) {
    arg = JSONString.substring(beginIdx, idx);
    arg.toCharArray(charBuffer, 16);
          
    beginIdx = idx + 1;
    idx = JSONString.indexOf("\"", beginIdx);

    // only four pieces of information are needed
    // start time
    if(i == 3) {
      if(charBuffer[4] == '\0'){
        charBuffer[4] = '0';
        charBuffer[5] = '\0';
      }
      uint8_t myHour = atoi(&charBuffer[0]);
      uint8_t myMin = atoi(&charBuffer[3]);
      tempEvent.setStartHour(myHour);
      tempEvent.setStartMin(myMin);
      
    // end time
    } else if (i == 7) {
      if(charBuffer[4] == '\0'){
        charBuffer[4] = '0';
        charBuffer[5] = '\0';
      }
      uint8_t myHour = atoi(&charBuffer[0]);
      uint8_t myMin = atoi(&charBuffer[3]);
      tempEvent.setEndHour(myHour);
      tempEvent.setEndMin(myMin);

    // weekday
    } else if (i == 11) {
      myDay = dayDecoder(arg);
    // node ID
    } else if (i == 15) {
      int myID = atoi(charBuffer);
      // tempEvent.setNodeID(myID);
      tempEvent.setNodeID(1);
    // valve #
    } else if (i == 19) {
      int myValve = atoi(charBuffer);
      if(myValve > 0) {
        tempEvent.setValveNum(myValve);
        eventCount += 1;
      }
      break;
    }
  }
  
  // insert this event to the weekly schedule
  if(myDay == 7) {
    for(int i = 0; i <= 6; i++) {
      weeklySchedule.insert(i, tempEvent);
    }
  } else {
    weeklySchedule.insert(myDay, tempEvent);
  }

  Serial.print("At node "); Serial.print(tempEvent.getNodeID());
  Serial.print(", valve "); Serial.print(tempEvent.getValveNum()); Serial.print(" is set to have a start time of ");
  Serial.print(tempEvent.getStartHour()); Serial.print(":"); Serial.print(tempEvent.getStartMin());
  Serial.print(" and an end time of "); Serial.print(tempEvent.getEndHour()); Serial.print(":"); Serial.print(tempEvent.getEndMin());
  Serial.println(".");
  JSONString = "";  // clear JSON string for the next event
}


/*
 * deleteSchedule()
 * 
 * This function deletes all schedule events currently stored in the weekly schedule
 * -- use with caution!
 * 
 * @preconditions: weeklySchedule contains schedule events in it
 * @postconditions: weeklySchedule is empty
*/
void deleteSchedule() {
  // Erase all schedule events in the current weekly schedule
  for(uint8_t i = 0; i <= 6; i++) {
    weeklySchedule.deleteDaysSchedule(i);
  }
}


/*
 * dayDecoder(String myDay)
 * 
 * This function translates the string name of a weekday into a number
 * 
 * @param String myDay: string of the day being decoded
 * @return: a number from 0 - 6 representing a weekday with 0 being Sunday
 *          or 7 meaning Everyday or -1 if unrecognized input
*/
int8_t dayDecoder(String myDay) {
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
  } else if(myDay == "Everyday") {
    return 7;
  } else { 
    // then something is not right and we need to do something about it?
    return -1;
  }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Helper Functions///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/* 
 * handleButtonPress()
 *
 * Called when the button is pressed; toggles between garden awake and garden asleep
 * 
 * @preconditions: garden is initializes
 * @postconditions: garden state is toggled between asleep/awake; if turned asleep,
 *                  turns off all valves
 */ 
void handleButtonPress(){

  // TODO known issue: sometimes crashes when button is pressed, but most of the time functions properly
  //                   when this occurs, it does reset :)

  // Ignore if are in between getting final status and sending the report to the server
  // (it is in a psuedo-sleep, which we do not want to mess with)
  if(!(hour() == 0 && calledIsNewDay == false)){
  
    Serial.println(F("\n[Detected buttonpress]"));
    
    // toggle states between asleep and awake
    gardenStatus.isAwake = !gardenStatus.isAwake;
  
    // if asleep, tell valves to shut
    if(gardenStatus.isAwake == false){
      // TODO set light sequence
      // for each node, shut off all valves
      uint8_t node;
      for(node=1; node<=16; node++){
        // if registered
        if(gardenStatus.nodeStatusPtrs[node] != NULL){
          // if connected
          if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED){
            // send command to turn off all valves
            Valve_Command vc;
            vc.whichValve = ALL_VALVES;
            vc.onOrOff = OFF;
            vc.timeToLive = VALVE_COMMAND_TTL;
            safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
          }
        }
      }
    }
    else{
      // TODO set light sequence
    }
  
    // report state to the server
    checkAlerts(GARDEN_TOGGLE, 0);
  }
  
  // reset flag
  hadButtonPress = false;
}


/* 
 * initPins()
 *
 * Sets the usage of each pin.
 * 
 * @preconditions: none
 * @postconditions: pins are ready to be used for radio, 3G modem, buttons, and LEDs
 */ 
void initPins(){  
  // BUTTON
  pinMode(BUTTON, INPUT_PULLUP);
  // attach interrupt at the end of setup() in initPins2()
  
  // LEDR and LEDG
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);  

  // ThreeG
  digitalWrite(ThreeG, LOW);
  pinMode(ThreeG, OUTPUT);
  digitalWrite(ThreeG, LOW);
  delay(1100); // modem requires >1s pulse
  // return 3G I/O pin to input/hi-Z state  
  pinMode(ThreeG, INPUT);

  // RESET_GND
  // initialize at the end of setup() in initPins2()

  // RESET_PIN
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
}


/* 
 * initPins2()
 *
 * Called at the end of setup(), inits some additional pins
 * 
 * @preconditions: initPins() has already been called
 * @postconditions: button attached to interrupt and reset circuit is enabled
 */ 
void initPins2() {
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
void flashLED(uint8_t whichLED, uint8_t myNum, uint8_t myTime) {
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
void hardReset() {
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
void refreshReset() {
  pinMode(RESET_PIN, OUTPUT);  
  digitalWrite(RESET_PIN, LOW);
  delay(50); // 95% charged: 3*tau = 3*RC = 3*200*100*10^-6 = 60 ms
  pinMode(RESET_PIN, INPUT);
}


/* 
 * checkSchedule()
 *
 * For each connected node, looks through the schedule to see if there are
 * any relevant events. If it detects that a valve is in a state that it 
 * should not be, it sends a command to open or close it.
 * 
 * @preconditions: mesh established and schedule available
 * @postconditions: schedule applied to nodes
 */ 
void checkSchedule() {

  ///////////////////////////////////////////////////////////////////
  ////////////////////////// FOR THE DEMO ///////////////////////////
  ///////////////////////////////////////////////////////////////////
  
  if(forDemo.mode == 1) {
    // valve 1
    if(gardenStatus.nodeStatusPtrs[1]->valveStates[1].state != forDemo.v1){
      Serial.print(F("Valve 1 is ")); Serial.print(gardenStatus.nodeStatusPtrs[1]->valveStates[1].state);
      Serial.print(F(" and should be ")); Serial.println(forDemo.v1);
      // send open/close signal
      Valve_Command vc;
      vc.whichValve = 1;
      vc.onOrOff = forDemo.v1;
      vc.timeToLive = VALVE_COMMAND_TTL;
      safeMeshWrite(mesh.getAddress(1), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
    }
    // valve 2
    if(gardenStatus.nodeStatusPtrs[1]->valveStates[2].state != forDemo.v2) {
      Serial.print(F("Valve 2 is ")); Serial.print(gardenStatus.nodeStatusPtrs[1]->valveStates[2].state);
      Serial.print(F(" and should be ")); Serial.println(forDemo.v2);
      // send open/close signal
      Valve_Command vc;
      vc.whichValve = 2;
      vc.onOrOff = forDemo.v2;
      vc.timeToLive = VALVE_COMMAND_TTL;
      safeMeshWrite(mesh.getAddress(1), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
    }
    // valve 3
    if(gardenStatus.nodeStatusPtrs[1]->valveStates[3].state != forDemo.v3) {
      Serial.print(F("Valve 3 is ")); Serial.print(gardenStatus.nodeStatusPtrs[1]->valveStates[3].state);
      Serial.print(F(" and should be ")); Serial.println(forDemo.v3);
      // send open/close signal
      Valve_Command vc;
      vc.whichValve = 3;
      vc.onOrOff = forDemo.v3;
      vc.timeToLive = VALVE_COMMAND_TTL;
      safeMeshWrite(mesh.getAddress(1), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
    }
    return;
  }
  
  ///////////////////////////////////////////////////////////////////
  ////////////////// END OF DEMO SECTION ////////////////////////////
  ///////////////////////////////////////////////////////////////////

  else if(doDailySchedule) {
    // for each node
    uint8_t node;
    for(node=1; node<=16; node++) {
      // if registered
      if(gardenStatus.nodeStatusPtrs[node] != NULL) {
        // if connected
        if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED) {
          Serial.print(F("Checking node: ")); Serial.println(node);
          // for each valve
          uint8_t valve;
          for(valve=1; valve<=4; valve++) {
            if(gardenStatus.nodeStatusPtrs[node]->valveStates[valve].isConnected){
              Serial.print(F("Checking valve: ")); Serial.println(valve);
              bool shouldBeOn;
              shouldBeOn = weeklySchedule.shouldValveBeOpen(weekday()-1, hour(), minute(), node, valve);
              Serial.print(F("Valve is ")); Serial.print(gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state);
              Serial.print(F(" and should be ")); Serial.println(shouldBeOn);
              
              
              // if schedule says should be open and is closed
              if(shouldBeOn && gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state == OFF) {
                // send open signal
                Valve_Command vc;
                vc.whichValve = valve;
                vc.onOrOff = ON;
                vc.timeToLive = VALVE_COMMAND_TTL;
                safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
              }
              // else if schedule says should be closed and is open
              else if(shouldBeOn == false && gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state == ON) {
                // send close signal
                Valve_Command vc;
                vc.whichValve = valve;
                vc.onOrOff = OFF;
                vc.timeToLive = VALVE_COMMAND_TTL;
                safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
              }
              // else the state is as it should be
            }
          }
        }
        // else node is not connected to mesh
      }
      // else unregistered
    }    
  }

  // else make sure all valves are off
  else {
    // for each node
    uint8_t node;
    for(node=1; node<=16; node++) {
      // if registered
      if(gardenStatus.nodeStatusPtrs[node] != NULL) {
        // if connected
        if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED) {
          Serial.print(F("Checking node: ")); Serial.println(node);
          // for each valve
          uint8_t valve;
          for(valve=1; valve<=4; valve++){
            if(gardenStatus.nodeStatusPtrs[node]->valveStates[valve].isConnected) {
              Serial.print(F("Checking valve: ")); Serial.println(valve);
              Serial.print(F("Valve is ")); Serial.print(gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state);
              Serial.print(F(" and should be ")); Serial.println(doDailySchedule);
              
              // if valve is open
              if( gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state == ON) {
                // send close signal
                Valve_Command vc;
                vc.whichValve = valve;
                vc.onOrOff = OFF;
                vc.timeToLive = VALVE_COMMAND_TTL;
                safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
              }
            }
          }
        }
      }
    }
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
bool safeMeshWrite(uint8_t destination, void* payload, char header, uint8_t datasize, uint8_t timesToTry) {
  // putting these here to see if it helps with mesh stability
  mesh.update();
  mesh.DHCP();
  
  // perform write
  if (!mesh.write(destination, payload, header, datasize)) {
    // if a write fails, check if have more tries
    if(timesToTry > 0){
      delay(RETRY_PERIOD);
      return safeMeshWrite(destination, payload, header, datasize, --timesToTry);
    }
    else{
      // write failed
      if(mesh.getNodeID(destination) != -1 && gardenStatus.nodeStatusPtrs[mesh.getNodeID(destination)] != NULL) {
        gardenStatus.nodeStatusPtrs[mesh.getNodeID(destination)]->meshState = MESH_DISCONNECTED; // TODO is this the only way to tell?
        //gardenStatus.numConnectedNodes--;
      }
      Serial.print(F("[Send of type ")); Serial.print(header); Serial.print(F(" failure; node "));
      Serial.print(mesh.getNodeID(destination)); Serial.println(F(" is DISCONNECTED]"));
      return false;
    }
  }
  else {
    // write succeeded
    Serial.print(F("[Send of type ")); Serial.print(header); Serial.print(F(" to node "));
    Serial.print(mesh.getNodeID(destination)); Serial.println(F(" success]"));
    return true;
  }
}


/* 
 * checkNodeRegistered()
 *
 * If a node is detected in the mesh but unregistered in the list of nodes,
 * this function adds it.
 * 
 * @preconditions: mesh established
 * @postconditions: new node is registered, if hasn't been already
 * 
 * @param uint8_t nodeID: the ID of the node (not the address)
 * 
 * @return bool: true if was already registered, false if was not
 */ 
bool checkNodeRegistered(uint8_t nodeID) {
  if(gardenStatus.nodeStatusPtrs[nodeID] == NULL){
    gardenStatus.nodeStatusPtrs[nodeID] = new Node_Status;
    gardenStatus.numRegisteredNodes++;
    gardenStatus.numConnectedNodes++;
    return false;
  }
  return true;
}


/* 
 * readMeshMessages()
 *
 * Checks if the mesh has any available messages on it; if it does, it reads
 * and responds to them as appropriate.
 * 
 * @preconditions: mesh established
 * @postconditions: incoming messages read and handled
 */ 
void readMeshMessages() {
  mesh.update();
  mesh.DHCP();
  
  while(network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
    Serial.print(F("\n[Received ")); Serial.print(char(header.type));
    Serial.print(F(" type message from node ")); Serial.print(mesh.getNodeID(header.from_node));
    Serial.println(F("]"));

    // check that it's registered
    bool wasRegistered;
    wasRegistered = checkNodeRegistered(mesh.getNodeID(header.from_node));

    switch(header.type) {
    case SEND_NODE_STATUS_H:
      // A node is reporting its status. Update its status in gardenStatus
      uint8_t node;
      node = mesh.getNodeID(header.from_node);
      
      // but first, store its old status to see if there are any new alerts
      // if is first message, then prefill with good values
      Node_Status oldNS;
      if(wasRegistered == false) {
        oldNS.voltageState = GOOD_VOLTAGE;
        oldNS.flowState = NO_FLOW_GOOD;        
      }
      else {
        oldNS = *gardenStatus.nodeStatusPtrs[node];
      }
      
      // read in the new status
      network.read(header, gardenStatus.nodeStatusPtrs[node], sizeof(Node_Status));

      // compare and look for new items of concern
      // check for new voltage error
      if(gardenStatus.nodeStatusPtrs[node]->voltageState != GOOD_VOLTAGE &&
         gardenStatus.nodeStatusPtrs[node]->voltageState != oldNS.voltageState) {
        gardenStatus.gardenState = GARDEN_NODE_ERROR;
        // report this to server
        checkAlerts(BAD_VOLTAGE_STATE, node);
        if (gardenStatus.voltage_alert){
          checkSMSAlerts(BAD_VOLTAGE_STATE, node);
        }
      }
      // check for voltage error resolved
      else if(gardenStatus.nodeStatusPtrs[node]->voltageState == GOOD_VOLTAGE &&
              gardenStatus.nodeStatusPtrs[node]->voltageState != oldNS.voltageState) {
        // report this to server
        checkAlerts(BAD_VOLTAGE_STATE, node);
        if (gardenStatus.voltage_alert){
          checkSMSAlerts(BAD_VOLTAGE_STATE, node);
        }
      }

      // check for new flow rate error
      if(gardenStatus.nodeStatusPtrs[node]->flowState != HAS_NO_METER &&
              gardenStatus.nodeStatusPtrs[node]->flowState != NO_FLOW_GOOD &&
              gardenStatus.nodeStatusPtrs[node]->flowState != FLOWING_GOOD &&
              gardenStatus.nodeStatusPtrs[node]->flowState != oldNS.flowState) {
        gardenStatus.gardenState = GARDEN_NODE_ERROR;
        // report this to server
        checkAlerts(BAD_FLOW_RATE, node);
        if(gardenStatus.valve_alert) {
          checkSMSAlerts(BAD_FLOW_RATE, node);
        }
      }
      // check for flow rate error resolved
      else if((gardenStatus.nodeStatusPtrs[node]->flowState == NO_FLOW_GOOD ||
              gardenStatus.nodeStatusPtrs[node]->flowState == FLOWING_GOOD) &&
              (oldNS.flowState != HAS_NO_METER &&
               oldNS.flowState != NO_FLOW_GOOD &&
               oldNS.flowState != FLOWING_GOOD)) {
        // report this to server
        checkAlerts(BAD_FLOW_RATE, node);
        if(gardenStatus.valve_alert) {
          checkSMSAlerts(BAD_FLOW_RATE, node);
        }
      }
      
      break;
    case SEND_JUST_RESET_H:
      // means that a node has recovered from reset; true means it was told to do so, false means a crash

      bool toldToReset;      
      network.read(header, &toldToReset, sizeof(toldToReset));
      
      // what to do with this information? ask for status update? -> will get this automatically...
      
    break;
    
    case SEND_NODE_SLEEP_H:
      // node had its button pressed and toggled sleep
      bool nodeIsAwake;
      network.read(header, &nodeIsAwake, sizeof(nodeIsAwake));
      gardenStatus.nodeStatusPtrs[mesh.getNodeID(header.from_node)]->isAwake = nodeIsAwake;
      
    break;
    
    default:
      char placeholder;
      network.read(header, &placeholder, sizeof(placeholder));
      Serial.println(F("Unknown message type."));
    break;
    }

    // if have a new node, check if it should be on
    if(wasRegistered == false){
      checkSchedule();
    }
  }
}


/* 
 * isNewDay()
 *
 * Handles activities that must occur once per day (at midnight). Includes reseting 
 * some variables and sending a message to the nodes. 
 * 
 * @preconditions: 3G connection and mesh are established
 * @postconditions: variables and daily stats are reset
 */ 
void isNewDay() {
  gardenStatus.isAwake = true;
  doDailySchedule = true;
  checkAlerts(GARDEN_TOGGLE, 0);
  checkAlerts(DAILY_REPORT, 0);

  // reset garden status
  gardenStatus.percentAwake = 100;
  gardenStatus.percent3GUptime = (gardenStatus.threeGState != TR_G_DISCONNECTED) ? 100 : 0;
  gardenStatus.percentMeshUptime = (gardenStatus.meshState == MESH_ALL_NODES_GOOD) ? 100 : 0;
  statusCounter = 0;
  // for each node
  uint8_t node;
  for(node=1; node<=16; node++) {
    // if registered
    if(gardenStatus.nodeStatusPtrs[node] != NULL) {
      // if connected
      if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED) {
          // this is handled a few minutes beforehand, elsewhere
//        char placeholder = '0';
//        safeMeshWrite(mesh.getAddress(node), &placeholder, IS_NEW_DAY_H, sizeof(placeholder), DEFAULT_SEND_TRIES);
      }
      else{
        // if not connected, must manually reset some of its properties
        gardenStatus.nodeStatusPtrs[node]->isAwake = true;
        gardenStatus.nodeStatusPtrs[node]->accumulatedFlow = 0;
        gardenStatus.nodeStatusPtrs[node]->maxedOutFlowMeter = false;
        gardenStatus.nodeStatusPtrs[node]->percentAwake = 100;
        gardenStatus.nodeStatusPtrs[node]->percentMeshUptime = 0;
        for(uint8_t valve = 1; valve<=4; valve++) {
          gardenStatus.nodeStatusPtrs[node]->valveStates[valve].timeSpentWatering = 0;
        }
      }
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
void initGardenStatus() {  
  // isAwake
  gardenStatus.isAwake = true;
  
  // threeGState
  gardenStatus.threeGState = TR_G_DISCONNECTED;

  // meshState
  gardenStatus.meshState = MESH_NOT_BEGAN;

  // gardenState
  gardenStatus.gardenState = GARDEN_ALL_IS_WELL;

  // numRegisteredNodes
  gardenStatus.numRegisteredNodes = 0;

  // numConnectedNodes
  gardenStatus.numConnectedNodes = 0;

  // nodeStatusPrts -- initiate all to NULL
  *gardenStatus.nodeStatusPtrs = {0};

  // percentAwake
  gardenStatus.percentAwake = 100;

  // percent3GUptime
  gardenStatus.percent3GUptime = 100;

  // percentMeshUptime
  gardenStatus.percentMeshUptime = 100;

  // alert setting for SMS message
  // do we need to preset the phone number?
  memset(&gardenStatus.phoneNum[0], '0', sizeof(gardenStatus.phoneNum)-1);
  gardenStatus.phoneNum[11] = '\0';
  gardenStatus.valve_alert = true;
  gardenStatus.mesh_alert = true;
  gardenStatus.reset_alert = true;
  gardenStatus.voltage_alert = true;
}


/* 
 * updateGardenStatus()
 *
 * Performs diagnostics on the 3G connection, mesh connection, and nodes
 * 
 * @preconditions: gardenStatus is initialized
 * @postconditions: gardenStatus is updated
 */
void updateGardenStatus() {
  statusCounter++;

  //////////// CHECK 3G CONNECTION //////////// 
   
  // gardenStatus.threeGState is updated elsewhere based on modem response

  // check if has been in TR_G_ERROR state for TIME_TILL_3G_ERR consecutive minutes
  static time_t threeGDisconnectedTimer = 0;
  if(gardenStatus.threeGState == TR_G_ERROR) {
    // if first time have seen this issue, record the time
    if(threeGDisconnectedTimer == 0){
      threeGDisconnectedTimer = now();
    }
    // else check if is late enough to send the alert
    // resends the alert every 24 hours
    else if((now() - (threeGDisconnectedTimer + TIME_TILL_3G_ERR*60))%(24*60*60) < TIMER1_PERIOD/1000000) {
      checkSMSAlerts(SERVER_COMM_DOWN, 0);
    }
  }
  else {
    // if already sent an alert, send a "fixed" alert
    if(threeGDisconnectedTimer != 0 &&
       now() > threeGDisconnectedTimer + TIME_TILL_3G_ERR*60){
      checkSMSAlerts(SERVER_COMM_DOWN, 0);
    }
    threeGDisconnectedTimer = 0;
  }
  
  
  //////////// CHECK NODE_STATUSES ////////////

  // manually check for unregistered nodes
//  Serial.println("");
//  Serial.print("Num addresses in list: "); Serial.println(mesh.addrListTop);
  if(mesh.addrListTop-1 != gardenStatus.numRegisteredNodes) {
    // if numbers don't match, check each registered address
    for(uint8_t i=1; i<mesh.addrListTop; i++) {
      // if have a node registered in mesh but not in struct, request a status
      //    (will add it after the status is responded to)
      if(gardenStatus.nodeStatusPtrs[mesh.addrList[i].nodeID] == NULL) {
        safeMeshWrite(mesh.addrList[i].address, NULL, GET_NODE_STATUS_H, sizeof(NULL), DEFAULT_SEND_TRIES);
      }
    }
  }
  
  
  //////////// CHECK GARDEN STATE ////////////

  gardenStatus.numRegisteredNodes = 0;
  gardenStatus.numConnectedNodes = 0;
  
  gardenStatus.gardenState = GARDEN_ALL_IS_WELL;
  // cycle through CONNECTED nodes, and if any have an error, report
  uint8_t node;
  for(node=1; node<=16; node++) {
    // if registered
    if(gardenStatus.nodeStatusPtrs[node] != NULL) {
      gardenStatus.numRegisteredNodes++;
      
      // every PERIOD_ASKS_STATUS times, manually ask the nodes for status updates
      char placeHolder = '0';
      bool success = false;
      if(statusCounter % PERIOD_ASKS_STATUS == 0) {
        success = safeMeshWrite(mesh.getAddress(node), &placeHolder, GET_NODE_STATUS_H, sizeof(placeHolder), DEFAULT_SEND_TRIES);
      }
      // otherwise, just do a send check
      else {
        success = safeMeshWrite(mesh.getAddress(node), &placeHolder, CONNECTION_TEST_H, sizeof(placeHolder), DEFAULT_SEND_TRIES);
      }
      gardenStatus.nodeStatusPtrs[node]->meshState = success ? MESH_CONNECTED : MESH_DISCONNECTED;
      
      // if connected
      //if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED){
      if(success) {
        gardenStatus.numConnectedNodes++;
        
        // check if it has any errors; NOTE: reporting handled elsewhere
        if(gardenStatus.nodeStatusPtrs[node]->voltageState != GOOD_VOLTAGE) {
          gardenStatus.gardenState = GARDEN_NODE_ERROR;
          break;
        }
        else if(gardenStatus.nodeStatusPtrs[node]->flowState != HAS_NO_METER &&
                gardenStatus.nodeStatusPtrs[node]->flowState != NO_FLOW_GOOD &&
                gardenStatus.nodeStatusPtrs[node]->flowState != FLOWING_GOOD) {
          gardenStatus.gardenState = GARDEN_NODE_ERROR;
          break;
        }
      }
      else {
        // have disconnected node; report is handled elsewhere
      }
    }
  }
  
  
  //////////// CHECK OVERALL MESH CONNECTION ////////////

  // assume began, so just check ratio of connected nodes
  static time_t disconnectedCounter = 0; // NOTE only set to 0 first time; else keeps old value
  if(gardenStatus.numConnectedNodes == gardenStatus.numRegisteredNodes &&
      gardenStatus.meshState != MESH_ALL_NODES_GOOD) {
    gardenStatus.meshState = MESH_ALL_NODES_GOOD;
    disconnectedCounter = 0;
    checkAlerts(MESH_DOWN, 0);
    if(gardenStatus.mesh_alert) {
       checkSMSAlerts(MESH_DOWN, 0);
    }
  }
  else if(gardenStatus.numConnectedNodes == 0 &&
          gardenStatus.numRegisteredNodes > 0) {
    // check if is new error, but only call it error if has been so for TIME_TILL_MESH_ERR
    if(disconnectedCounter == 0) {
      disconnectedCounter = now();
    }
    else if(gardenStatus.meshState != MESH_ALL_NODES_DOWN && now() > disconnectedCounter + TIME_TILL_MESH_ERR) {
      gardenStatus.meshState = MESH_ALL_NODES_DOWN;
      checkAlerts(MESH_DOWN, 0);
      if(gardenStatus.mesh_alert){
         checkSMSAlerts(MESH_DOWN, 0);
      }
    }
  }
  else if(gardenStatus.numConnectedNodes < gardenStatus.numRegisteredNodes) {
    // check if is new error, but only call it error if has been so for TIME_TILL_MESH_ERR
    if(disconnectedCounter == 0){
      disconnectedCounter = now();
    }
    else if(gardenStatus.meshState != MESH_SOME_NODES_DOWN && now() > disconnectedCounter + TIME_TILL_MESH_ERR) {
      gardenStatus.meshState = MESH_SOME_NODES_DOWN;
      checkAlerts(MESH_DOWN, 0);
      if(gardenStatus.mesh_alert) {
         checkSMSAlerts(MESH_DOWN, 0);
      }
    }
  }  


  //////////// STAT TRACKING ////////////

  // Don't count if are in between getting final status and sending the report to the server
  // (it is in a psuedo-sleep, which we do not want to report)
  if(hour() > 0 || calledIsNewDay == true) {
    gardenStatus.percentAwake = (gardenStatus.percentAwake * (statusCounter-1) + (gardenStatus.isAwake ? 100 : 0))/statusCounter;
  }
  bool threeGGood = (gardenStatus.threeGState != TR_G_ERROR);
  gardenStatus.percent3GUptime = (gardenStatus.percent3GUptime * (statusCounter-1) + (threeGGood ? 100 : 0))/statusCounter;
  bool meshGood = (gardenStatus.meshState == MESH_ALL_NODES_GOOD);
  gardenStatus.percentMeshUptime = (gardenStatus.percentMeshUptime * (statusCounter-1) + (meshGood ? 100 : 0))/statusCounter;
}


/* 
 * printGardenStatus()
 *
 * Prints gardenStatus to Serial port in a user-friendly way.
 * 
 * @preconditions: myStatus is initialized, Serial port is active
 * @postconditions: none
 */ 
void printGardenStatus() {
  // print number of times executed
//  Serial.println(); Serial.println(statusCounter);
//
//  // print time
//  Serial.print(F("Time         : ")); digitalClockDisplay();
//  Serial.print(F("\nDate         : ")); dateDisplay();
  Serial.println(); Serial.println(); digitalClockDisplay();
  Serial.println(); dateDisplay(); Serial.println();

  if(gardenStatus.isAwake == false) Serial.println(F("GARDEN IS IN STANDBY"));
  if(forDemo.mode == 1) Serial.println(F("Demo 1 mode"));
  Serial.print(F("Time Awake   : ")); Serial.print(gardenStatus.percentAwake); 
  Serial.println(F("%"));

  // print mesh stuff
  Serial.print(F("Mesh status  : "));
  Serial.print(gardenStatus.numConnectedNodes); Serial.print(F("/"));
  Serial.print(gardenStatus.numRegisteredNodes); Serial.print(F("            : "));
  if(gardenStatus.meshState == MESH_ALL_NODES_GOOD) {
    Serial.println(F("good"));
  }
  else if(gardenStatus.meshState == MESH_NOT_BEGAN) {
    Serial.println(F("not began"));
  }
  else if(gardenStatus.meshState == MESH_SOME_NODES_DOWN) {
    Serial.println(F("some nodes down!"));
  }
  else if(gardenStatus.meshState == MESH_ALL_NODES_DOWN) {
    Serial.println(F("MESH IS DOWN!"));
  }
  Serial.print(F("Mesh uptime  : ")); 
  Serial.print(gardenStatus.percentMeshUptime); Serial.println(F("%"));

  // print 3G stuff
  Serial.print(F("3G status    : ")); 
  if(gardenStatus.threeGState != TR_G_ERROR) {
    Serial.print(F("connected (")); Serial.print(gardenStatus.threeGState); 
    Serial.println(F(")  : good"));
  }
  else {
    Serial.println(F("CANNOT CONNECT"));
  }
  Serial.print(F("3G uptime    : ")); 
  Serial.print(gardenStatus.percent3GUptime); Serial.println(F("%"));

  // print node stuff
  Serial.print(F("Node states  : "));
  if(gardenStatus.gardenState == GARDEN_ALL_IS_WELL) {
    Serial.println(F("no issues      : good\n"));
  }
  else if(gardenStatus.gardenState == GARDEN_NODE_ERROR) {
    Serial.println(F("at least one connected node has an issue!\n"));
  }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////     Timer       ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/* 
 * timeInit()
 *
 * This function initializes the internal timer on the gateway and 
 * synchronizes it with the NIST server time
 * 
 * @preconditions: 3G established
 * @postconditions: internal timer is set to EST
 */
void timeInit() {
  int timeArray[6];
  Modem_Serial.println(F("AT#SD=2,0,13,\"time.nist.gov\",0"));
  delay(250);

  boolean timeGood = false;
  int setTimeFlag = 0;
  while(!timeGood) {
    while(Modem_Serial.available() > 0) {
      getModemResponse();
      if(currentString.substring(currentString.length()-11,currentString.length()) == "UTC(NIST) *") {
      // when the time string is received from NIST server
        // grab the substring containing time information
        String dateString = currentString.substring(6,14);
        String timeString = currentString.substring(15,23);
        
        Serial.println();
        Serial.println("Detected time string!");
        Serial.println(dateString);
        Serial.println(timeString);

        // assign time info to time array    
        int beginIdx = 0;
        int idx0 = dateString.indexOf("-");
        int idx1 = timeString.indexOf(":");
        char charBuffer[16];
        String arg;

        for(int i = 0; i <= 2; i++) {
          arg = dateString.substring(beginIdx, idx0);
          arg.toCharArray(charBuffer, 16);
      
          // add error handling for atoi:
          timeArray[i] = atoi(charBuffer);
          beginIdx = idx0 + 1;
          idx0 = dateString.indexOf("-", beginIdx);
        }

        beginIdx = 0;
        for(int i = 3; i <= 5; i++) {
          arg = timeString.substring(beginIdx, idx1);
          arg.toCharArray(charBuffer, 16);
      
          // add error handling for atoi:
          timeArray[i] = atoi(charBuffer);
          beginIdx = idx1 + 1;
          idx1 = timeString.indexOf(":", beginIdx);
        }
        timeGood = true;
      // reprovision if socket dial fails
      } else if (currentString == "ERROR") {
        delay(1000);
        Modem_Serial.println(F("AT#SD=2,0,13,\"time.nist.gov\",0"));
      // reprovision if socket dial did not get proper response
      } else if (currentString == "NO CARRIER" && timeGood == false) {
        delay(1000);
        Modem_Serial.println(F("AT#SD=2,0,13,\"time.nist.gov\",0"));
      }
    } 
  }
  setTime(timeArray[3], timeArray[4], timeArray[5], timeArray[2], timeArray[1], timeArray[0]);
  setTime(now() - 60*60*4); // convert to EST time
}


/* 
 * digitalClockDisplay()
 *
 * This function prints out the current time recorded by the internal timer
 * in a readable way
 * 
 * @preconditions: internal timer is set
 * @postconditions: time is displayed in nice format through Serial port
 */
void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour()); printDigits(minute()); printDigits(second());
}


/* 
 * dateDisplay()
 *
 * This function prints out the current date recorded by the internal timer
 * in a readable way
 * 
 * @preconditions: internal timer is set
 * @postconditions: date is displayed in nice format through Serial port
 */
void dateDisplay() {
  switch(weekday()-1){
    case 0:
      Serial.print(F("Sunday, "));
      break;
    case 1:
      Serial.print(F("Monday, "));
      break;
    case 2:
      Serial.print(F("Tuesday, "));
      break;
    case 3:
      Serial.print(F("Wednesday, "));
      break;
    case 4:
      Serial.print(F("Thursday, "));
      break;
    case 5:
      Serial.print(F("Friday, "));
      break;
    case 6:
      Serial.print(F("Saturday, "));
      break;
    default: 
      break;
  }
  Serial.print(month()); Serial.print(F("/")); Serial.print(day());
  Serial.print(F("/")); Serial.print(year());
}


/* 
 * printDigits()
 *
 * This function process the display of the minute and second of the internal timer
 * 
 * @preconditions: called from digitalClockDisplay()
 * @postconditions: minutes or seconds are printed to Serial port
 * 
 * @param uint8_t digits: a digit between 0 and 60 which represents the current 
 *         minute or second recorded by the internal timer
 */
void printDigits(uint8_t digits) {
  // utility function for digital clock display: pruint8_ts preceding colon and leading 0
  Serial.print(F(":"));
  if(digits < 10)
    Serial.print(F("0"));
  Serial.print(digits);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////     Alert Engine       ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * checkAlerts(uint8_t opcode, uint8_t nodeNum, uint8_t type);
 * 
 * This function sends a message to the server based an opcode + nodeNum combination.
 * Refer to alert_engine_protocol for the meaning of different alerts.
 * 
 * @preconditions: a function call is initiated by an exception case in the main loop
 * @postconditions: an alert message is sent to the server
 * 
 * @param uint8_t opcode: an opcode befined in the protocol
 * @param uint8_t nodeNum: the node number, only used for specific opcode
 * @param unit8_t type: whether the alert is new (0) or resolved (1);
 *                      for daily report, means nothing
 *                      for gateway reset, means nothing
*/
void checkAlerts(uint8_t opcode, uint8_t nodeNum) {
  String myAlert;
  // daily report
  switch(opcode) {
    case DAILY_REPORT:
      myAlert = "0" + (String) opcode + "%" + (String) gardenStatus.percentAwake + "%"
      + (String) gardenStatus.percentMeshUptime + "%" + (String) gardenStatus.percent3GUptime;
  
      // use for loop to create information about each registered node in {}
      for(uint8_t node = 1; node<=16; node++) {
        String myNodeInfo;
        // if registered
        if(gardenStatus.nodeStatusPtrs[node] != NULL) {
          myNodeInfo += "{%" + (String) node + "%";
          myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->meshState;
          myNodeInfo += "%";
          myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->percentAwake;
          myNodeInfo += "%";
          myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->accumulatedFlow;
          myNodeInfo += "%";
          myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->maxedOutFlowMeter;
       
          for(uint8_t valve = 1; valve <= 4; valve++) {
            String myValveInfo;
            if(gardenStatus.nodeStatusPtrs[node]->valveStates[valve].isConnected) {
              myValveInfo += "%[%" + (String) valve + "%";
              float myValveTime = gardenStatus.nodeStatusPtrs[node]->valveStates[valve].timeSpentWatering;
              myValveTime = myValveTime / 60;   // convert to minutes
              myValveInfo += (String) myValveTime + "]" + myValveInfo;
            }
          }
          myNodeInfo += "%}" + myNodeInfo;
        }
      }
    break;

    // bad flow rate
    case BAD_FLOW_RATE:
      myAlert = "0" + (String) opcode + "%" + nodeNum + "%";
      myAlert += (String) gardenStatus.nodeStatusPtrs[nodeNum]->flowState;
      myAlert += "%";
      myAlert += (String) gardenStatus.nodeStatusPtrs[nodeNum]->currentFlowRate;
    break;
      
    // mesh down
    case MESH_DOWN:
      myAlert = "0" + (String) opcode + "%" + (String) gardenStatus.meshState;
    break;
      
    // gateway self-reset
    case GATEWAY_RESET:
      myAlert = "0" + (String) opcode;
    break;
      
    // bad voltage state
    case BAD_VOLTAGE_STATE:
      myAlert = "0" + (String) opcode + "%" + nodeNum + "%" + (String) gardenStatus.nodeStatusPtrs[nodeNum]->voltageState;
    break;
  
    case GARDEN_TOGGLE:
      myAlert = "0" + (String) opcode + "%" + (String) gardenStatus.isAwake;
    break;
  
    case INCOMPLETE_JSON:
      myAlert = "RESENDSCHEDULE";
    break;
  
    default:
      // UNUSED
    break;
  }
  // debug
  Serial.println();
  Serial.print(myAlert);
  Serial.println("");

  sendAlertMessage(myAlert);

  delay(250);
  while(PrintModemResponse() > 0);
}


/*
 * sendAlertMessage(String myMessage)
 * 
 * This function send an alert message to the GardeNet server
 * 
 * @preconditions: the modem is connected to the GardeNet server
 * @postconditions: the modem has sent a string to the GardeNet server
 * 
 * @param String myMessage : a string containing the message that we need to send to the server
*/
void sendAlertMessage(String myMessage) {
  // put modem into sending mode
  Modem_Serial.print("AT#SSENDEXT=1,");
  Modem_Serial.println((String) myMessage.length());
  delay(500);
  while(PrintModemResponse() > 0);

  // send the message
  Modem_Serial.print(myMessage);
  Modem_Serial.write(26);
  Modem_Serial.write('\r');
}


/*
 * parseAlertSetting()
 * 
 * @preconditions: the modem receives an alert setting command from the server in
 *                 the form as SMS2485048891%1%0%1%1
 * @postconditions: the SMS alert setting is updated in the Garden_Status struct
*/
void parseAlertSetting() {
  int beginIdx = 0;
  // split the string by %
  int idx = alertSetting.indexOf('%');
  char charBuffer[16];
  String arg;

  // parse alert setting
  Serial.println("The current alert setting is...");
  for(int i = 0; i <= 4; i++) {
    arg = alertSetting.substring(beginIdx, idx);
    arg.toCharArray(charBuffer, 16); 
    beginIdx = idx + 1;
    idx = alertSetting.indexOf('%', beginIdx);

    // phone number
    if(i == 0) {
      Serial.print("Phone number:     "); Serial.println(arg);
      arg.toCharArray(gardenStatus.phoneNum, 11);

    // valve alert
    } else if (i == 1) {
      uint8_t myValue = atoi(charBuffer);
      if(myValue == 1) {
        gardenStatus.valve_alert = true;
      } else {
        gardenStatus.valve_alert = false;
      }
      Serial.print("Valve alert:      "); Serial.println(gardenStatus.valve_alert);

    // mesh alert
    } else if (i == 2) {
      uint8_t myValue = atoi(charBuffer);
      if(myValue == 1) {
        gardenStatus.mesh_alert = true;
      } else {
        gardenStatus.mesh_alert = false;
      }
      Serial.print("Mesh alert:       "); Serial.println(gardenStatus.mesh_alert);

    // restart alert
    } else if (i == 3) {
      uint8_t myValue = atoi(charBuffer);
      if(myValue == 1) {
        gardenStatus.reset_alert = true;
      } else {
        gardenStatus.reset_alert = false;
      }
      Serial.print("Reset alert:      "); Serial.println(gardenStatus.reset_alert);

    // voltage alert
    } else if (i == 4) {
      uint8_t myValue = atoi(charBuffer);
      if(myValue == 1) {
        gardenStatus.voltage_alert = true;
      } else {
        gardenStatus.voltage_alert = false;
      }
      Serial.print("Voltage alert:    "); Serial.println(gardenStatus.voltage_alert);
    }
  }
}


/*
 * parseDemoOneSetting()
 * 
 * This function parses currentString if it is a demo instruction
 * 
 * @preconditions: currentString contains a setting for the demo starting with DEMO1
 * @postconditions: the node(s) enter the demo mode. The scheduling algorithm is disabled, and
 *                  each valve is either on or off depends on the demo setting
*/
void parseDemoOneSetting() {
  int beginIdx = 0;
  // split the string by %
  int idx = demoString.indexOf('%');
  char charBuffer[16];
  String arg;

  // parse demo setting for each valve
  for(int i = 0; i <= 3; i++) {
    arg = demoString.substring(beginIdx, idx);

    arg.toCharArray(charBuffer, 16);
          
    beginIdx = idx + 1;
    idx = demoString.indexOf('%', beginIdx);

    // valve 1
    if (i == 1) {
      uint8_t myValue = atoi(charBuffer);
      if(myValue == 1) {
        forDemo.v1 = true;
      } else {
        forDemo.v1 = false;
      }
      Serial.print("\nV1 is ");
      Serial.println(forDemo.v1);

    // valve 2
    } else if (i == 2) {
      uint8_t myValue = atoi(charBuffer);
      if(myValue == 1) {
        forDemo.v2 = true;
      } else {
        forDemo.v2 = false;
      }
      Serial.print("V2 is ");
      Serial.println(forDemo.v2);

    // valve 3
    } else if (i == 3) {
      uint8_t myValue = atoi(charBuffer);
      if(myValue == 1) {
        forDemo.v3 = true;
      } else {
        forDemo.v3 = false;
      }
      Serial.print("V3 is ");
      Serial.println(forDemo.v3);
    }
  }
}


/*
 * checkSMSAlerts(uint8_t opcode, uint8_t nodeNum, uint8_t type)
 * 
 * This function constructs an SMS message based on the different alert opcode
 * and forwards it to a designated phone number stored in the Garden_Status struct
 * 
 * @preconditions: there is an alert message available from the nodes
 * @postconditions: the alert message is sent in the form of an SMS message
 * 
 * @param uint8_t opcode: an opcode defined for the alert engine protocol
 * @param uint8_t nodeNum: the node ID to be reported on
 * @param unit8_t type: whether the alert is new (0) or resolved (1); 
 *                      for gardentoggle, is ON or OFF
 *                      for daily report, means nothing
 *                      for gateway reset, means nothing
 */
void checkSMSAlerts(uint8_t opcode, uint8_t nodeNum) {
  String myAlert = "GardeNet user: ";
  // daily report
  switch(opcode) {
    case BAD_FLOW_RATE:
      if(gardenStatus.nodeStatusPtrs[nodeNum]->flowState == HAS_NO_METER ||
          gardenStatus.nodeStatusPtrs[nodeNum]->flowState == NO_FLOW_GOOD ||
          gardenStatus.nodeStatusPtrs[nodeNum]->flowState == FLOWING_GOOD) {
        myAlert += "the flow rate error on node ";
        myAlert += nodeNum;
        myAlert += " has been resolved.";
      }
      else if(gardenStatus.nodeStatusPtrs[nodeNum]->flowState == STUCK_AT_OFF) {
        myAlert += "node ";
        myAlert += nodeNum;
        myAlert += " is stuck off!";
      }
      else if(gardenStatus.nodeStatusPtrs[nodeNum]->flowState == STUCK_AT_ON) {
        myAlert += "node ";
        myAlert += nodeNum;
        myAlert += " is stuck on!";
      }
      else {
        myAlert += "node ";
        myAlert += nodeNum;
        myAlert += " is experiencing a flow rate issue!";
      }
    break;
    
    // mesh down
    case MESH_DOWN:
      if(gardenStatus.meshState == MESH_SOME_NODES_DOWN) {
        myAlert += "the onsite radio network is partially down!";
      }
      else if(gardenStatus.meshState == MESH_ALL_NODES_DOWN) {
        myAlert += "the onsite radio network is entirely down!";
      }
      else {
        myAlert += "the onsite radio network issue has been resolved.";
      }
    break;
    
    // gateway self-reset
    case GATEWAY_RESET:
      myAlert += "the gateway has reset itself.";
    break;
    
    // bad voltage state
    case BAD_VOLTAGE_STATE:
      if(gardenStatus.nodeStatusPtrs[nodeNum]->voltageState == LOW_VOLTAGE) {
        myAlert += "node ";
        myAlert += nodeNum;
        myAlert += " has a low voltage.";
      }
      else if(gardenStatus.nodeStatusPtrs[nodeNum]->voltageState == HIGH_VOLTAGE) {
        myAlert += "node ";
        myAlert += nodeNum;
        myAlert += " has a high voltage.";
      }
      else {
        myAlert += "the voltage issue on node ";
        myAlert += nodeNum;
        myAlert += " has been resolved.";
      }
    break;

    case SERVER_COMM_DOWN:
      if(gardenStatus.threeGState == TR_G_ERROR) {
        myAlert += "the gateway cannot connect to the server. If this issue persists, ";
        myAlert += "please cycle power on the server.";
      }
      else{
        myAlert += "the gateway-server connection issue has been resolved.";
      }
    break;
      
    default:
      // UNUSED
    break;
  }
  // debug
  Serial.println();
  Serial.print(myAlert);
  Serial.println();

  // send SMS message via modem if have a number
  if(strcmp(gardenStatus.phoneNum, "0000000000") != 0) {  
    // put modem into text mode
    Modem_Serial.println("AT+CMGF=1");
    delay(250);
    while(PrintModemResponse() > 0);
  
    // assemble SMS message and send
    Modem_Serial.print("AT+CMGS=\"+1");
    Modem_Serial.print((String) gardenStatus.phoneNum);
    Modem_Serial.print("\"\r");
    delay(250);
    while(PrintModemResponse() > 0);

    // format alert message
    Modem_Serial.print(myAlert);
    Modem_Serial.write(26);
    Modem_Serial.write("\r");
    delay(1000);
    while(PrintModemResponse() > 0);
  
    Serial.println("SMS alert sent.");
  
    // put modem back into PDU mode
    Modem_Serial.println("AT+CMGF=0");
    delay(250);
    while(PrintModemResponse() > 0);
  }
  else {
    Serial.println(F("[Would send an SMS alert, but do not have a number]"));
  }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////     Unsued Functions     ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * queryServerIP()
 * 
 * This function query the server's DNS domain name into an IP address
 * 
 * @preconditions: the modem's PDP context is activated
 * @postconditions: the server's IP address is stored in the gardenStatus struct
*/
void queryServerIP() {
  // executes DNS query to solve host name 
  Modem_Serial.println("AT#QDNS=\"gardenet.ddns.net\"");
  bool gotHostIP = false;
  
  String queryString;
  while(!gotHostIP) {
    while(Modem_Serial.available() > 0) {
      getModemResponse();
      if(currentString == "#QDNS: \"gardenet.ddns.net\",") {
        bool isIP = false;
        while(!isIP) {
          int myByte = Modem_Serial.read();
          // Serial.write(incomingByte);
          if(myByte == '\n') {
            isIP = true;
          } else if (myByte != '\"' && myByte != -1) {
            queryString += char(myByte);
          }
        }
        gotHostIP = true;
        currentString = "";
      }
    }
  }
  queryString.toCharArray(gardenStatus.serverIP, 20);
  Serial.println();
  Serial.print("The server's IP address is: ");
  Serial.println((String) gardenStatus.serverIP);
}


/*
 *  openFirewall()
 *  
 *  This function opens the modem's firewall for the server's IP address
 *  
 *  @preconditions: queryServerIP() returned an IP adress
 *  @postconditions: the modem can accept inbound connection from the server
*/
void openFirewall() {
  Serial.print("AT#FRWL=1,\"");
  Serial.print((String) gardenStatus.serverIP);
  Serial.println("\",\"255.255.0.0\"");

//  Modem_Serial.print("AT#FRWL=1,\"");
//  Modem_Serial.print((String) gardenStatus.serverIP);
//  Modem_Serial.println("\",\"255.255.0.0\"");
//  delay(500);
//  while(PrintModemResponse() > 0);
}


/*
 * suspendSocket()
 * 
 * This function suspends a socket connection by sending an escape sequence
 * 
 * @preconditions: there is an active TCP socket connection
 * @postconditions: the connection is suspended but can be reactivated
*/
void suspendSocket() {
  Modem_Serial.println("+++");
  delay(500);
  while(PrintModemResponse() > 0);
}


/*
 * restoreSocket()
 * 
 * This function restores a previously initiated socket dial
 * 
 * @preconditions: a socket connection was initiated and has been suspended
 * @postconditions: the socket connection is back online
*/
void restoreSocket() {
  Modem_Serial.println("AT#SO=1");
  delay(500);
  while(PrintModemResponse() > 0);
}


/* 
 * disconnectModem()
 *  
 * This function disconnects the 3G modem from the network and reports the data usage from 
 * the previous session as well as total data usage through the serial port
 * 
 * @preconditions: the modem is connected to the 3G cellular network (its PDP context is on)
 * @postconditions: the modem is no longer connected to the 3G network (its PDP context is turned off)
*/
void disconnectModem() {
  Modem_Serial.println("AT#SGACT=1,0");
  delay(500);
  boolean disconnected = false;
  while(!disconnected) {
    while(Modem_Serial.available() > 0) {
      getModemResponse();
      // if the modem already has an IP or retrieved IP 
      if(currentString == "ERROR" || currentString == "OK") {
        disconnected = true;
      } 
    } 
  }
  while(PrintModemResponse() > 0);
}


/*
 * socketListen()
 * 
 * This function listens for an inbound connection based on the firewall's filtered
 * IP address accepted range
 * 
 * @preconditions: the modem's firewall has a filtered range of IP address
 * @postconditions: if an inbound connection request is present, it is accepted
*/
void socketListen() {
  Modem_Serial.println("AT#SL=2,1,3500,0");
}


/*
 * socketAccept()
 * 
 * This function accept an socket connection when it is available
 * 
 * @preconditions: there is an inbound connection request
 * @postconditions: the connection request is accepted
*/
void socketAccept() {
  Modem_Serial.println("AT#SA=2,1");
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////     Setup       ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * setupGarden()
 * 
 * This sequence requests the available schedule, alert settings, and garden state from the server
 * 
 * @preconditions: the modem just experienced a reset
 * @postconditions: the modem received all necessary information fro mthe server to
 * control the garden
*/
void setupGarden() {
  currentString = "";
  // request alert setting from the server
  sendAlertMessage("ALERT?");
  delay(250);
  while(PrintModemResponse() > 0);
  bool alertGood = false;
  while(!alertGood) {
    while(Modem_Serial.available() > 0) {
      char incomingByte = Modem_Serial.read();
      Serial.print(incomingByte);
      if(incomingByte == '\n') {
        uint8_t tr_g_opmode = decodeModemResponse();
        if(tr_g_opmode == TR_G_ALERT_SETTING) {
          handleModemOperation(tr_g_opmode);
          alertGood = true;
        }
        currentString = "";
      } else if (incomingByte != '\r' && incomingByte != '\n') {
        currentString += incomingByte;
      }
    }
  }
  
  // request a schedule from the server
  sendAlertMessage("SCHEDULE?");
  bool scheduleGood = false;
  while(!scheduleGood) {
    while(Modem_Serial.available() > 0) {
      char incomingByte = Modem_Serial.read();
      Serial.print(incomingByte);
      if(incomingByte == '\n') {
        uint8_t tr_g_opmode = decodeModemResponse();
        if(tr_g_opmode != TR_G_NO_RESPONSE) {
          handleModemOperation(tr_g_opmode);
        }
        // only get out of the loop if schedule is good
        if(tr_g_opmode == TR_G_ENABLE_INT && gotAllEvents) {
          scheduleGood = true;
        } else if (tr_g_opmode == TR_G_NO_EVENTS) {
          scheduleGood = true;
        }
        currentString = "";
      } else if (incomingByte != '\r' && incomingByte != '\n') {
        currentString += incomingByte;
      }
    }
  }

  // request garden state from the server (awake / asleep)
  sendAlertMessage("STATE?");
  delay(250);
  while(PrintModemResponse() > 0);
  bool stateGood = false;
  while(!stateGood) {
    while(Modem_Serial.available() > 0) {
      char incomingByte = Modem_Serial.read();
      Serial.print(incomingByte);
      if(incomingByte == '\n') {
        uint8_t tr_g_opmode = decodeModemResponse();
        if(tr_g_opmode == TR_G_GARDEN_ON || tr_g_opmode == TR_G_GARDEN_OFF) {
          handleModemOperation(tr_g_opmode);
          stateGood = true;
        }
        currentString = "";
      } else if (incomingByte != '\r' && incomingByte != '\n') {
        currentString += incomingByte;
      }
    }
  }

//  // send modem IP address to server
//  String myIPAddr = "IP:";
//  myIPAddr += (String) gardenStatus.modemIP;
//  sendAlertMessage(myIPAddr);
//  delay(1000);
//  while(PrintModemResponse() > 0);
}


/* 
 * setup()
 *
 * The beginning of execution when the board boots; uses helper functions to initialize the pins, 
 * initialized myStatus, begin mesh communication, and more; see internal comments for more detail.
 * 
 * @preconditions: board just booted
 * @postconditions: all initializaion is complete, ready for normal operation
 */ 
void setup() {

  // initaialize all the pins
  initPins();

  // flash LEDR once to indicate the beginning of the setup process
  flashLED(LEDR, 1, 500);
  
  // begin serial communication
  Serial.begin(BAUD_RATE);
  while (!Serial);
  
  // check if button is being pressed; if so, do special startup
  if(digitalRead(BUTTON) == 0) {
    flashLED(LEDG, 3, 125);

    // do special stuff
  }

  // initialize gardenStatus
  initGardenStatus();

  // Setup 3G Modem
  setupModem();
  checkDataUsage();
  socketConfigs();
  getModemIP();

  // Setup mesh
  mesh.setNodeID(MASTER_NODE_ID);
  while(!mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT)) {
    Serial.println(F("Trouble setting up the mesh, trying again..."));
    delay(1000);
  }
  Serial.println();
  Serial.println(F("Mesh created"));
  gardenStatus.meshState = MESH_ALL_NODES_GOOD;
  mesh.setAddress(MASTER_NODE_ID, MASTER_ADDRESS);

  // init timer for regular system checks
  Timer1.initialize(TIMER1_PERIOD);
  Timer1.attachInterrupt(updateStatusISR);

  // setup time
  timeInit();
  
  // Hard-coded time for testing purpose
  // setTime(hr,min,sec,day,mnth,yr)
  // remember to fix line 168 in Time.cpp
  // (0, 1, 25, 12, 4, 16);
  // setTime(21, 0, 0, 3, 5, 16);

  // enable self-reset
  initPins2();
  
  openSocket();
  boolean setupDone = false;
  uint8_t connectTries = 1;
  while(!setupDone) {
    while(Modem_Serial.available()) {
      getModemResponse();
      if (currentString == "OK") {
        gardenStatus.threeGState = TR_G_CONNECTED;
        Serial.println("");
        setupDone = true;
      } 
      else if (currentString == "ERROR") {
        gardenStatus.threeGState = TR_G_ERROR;
        if(connectTries < THREE_G_COMM_TRIES){
          connectTries++;
          openSocket();
        }
//        else{
//          connectTries = 1;
//          Serial.print(F("\nCould not connect. Trying again in "));
//          Serial.print(THREE_G_SLEEP_PERIOD/60000); Serial.println(F(" minutes...\n"));
//          delay(THREE_G_SLEEP_PERIOD);
//        }
      } else if (currentString == "NO CARRIER") {
        gardenStatus.threeGState = TR_G_DISCONNECTED;
        if(connectTries < THREE_G_COMM_TRIES) {
          connectTries++;
          openSocket();
        }
//        else{
//          connectTries = 1;
//          Serial.print(F("\nCould not connect. Trying again in "));
//          Serial.print(THREE_G_SLEEP_PERIOD/60000); Serial.println(F(" minutes...\n"));
//          delay(THREE_G_SLEEP_PERIOD);
//        }
      }
    }
  }

  // request schedule, alert setting, and garden state from server
  setupGarden();
  
  // notify the server about modem reset
  checkAlerts(GATEWAY_RESET, 0);

  if (gardenStatus.reset_alert) {
    checkSMSAlerts(GATEWAY_RESET, 0);
  }


  ///////////////////////////////////////////////////////////////////
  //////////////////////// FOR THE DEMO /////////////////////////////
  ///////////////////////////////////////////////////////////////////
  forDemo.mode = 0;
  forDemo.v1 = false;
  forDemo.v2 = false;
  forDemo.v3 = false;
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

  //////////////////////// Time acceleration for testing /////////////////////////////
  // jump to 50 seconds
//  if(second() > 0 && second() < 15 ) {
//    setTime(now()+50-second());
//  }
  ////////////////////////////////////////////////////////////////////////////////////

  // Communication with server via 3G
  while(Modem_Serial.available() > 0) {
    char incomingByte = Modem_Serial.read();
    Serial.print(incomingByte);
    if(incomingByte == '\n') {
      uint8_t tr_g_opmode = decodeModemResponse();
      if(tr_g_opmode != TR_G_NO_RESPONSE) {
        handleModemOperation(tr_g_opmode);
      }
    } else if (incomingByte != '\r' && incomingByte != '\n') {
      currentString += incomingByte;
    }
  }

  // update node status if necessary
  if(updateStatusFlag) {
    // reprovision/retest socket dial if 3G is cannot connect
    if(gardenStatus.threeGState == TR_G_ERROR || gardenStatus.threeGState == TR_G_DISCONNECTED) {
      openSocket();
      Serial.println();
    }
    
    updateGardenStatus();
    printGardenStatus();
    
    // reset the flag
    updateStatusFlag = false;
  }

  // check if need to open/close valves according to schedule
  // to occur at the beginning of each new minute
  if(now() >= lastTime + 60 && second() == 0 && modemReceivingJSON == false) {
    if(gardenStatus.isAwake){
      checkSchedule();
    }
    refreshReset();
    lastTime = now();
  }

  // if had buttonpress, toggle between awake and asleep
  if(hadButtonPress) {
    handleButtonPress();
  }

  // at midnight, let the nodes know it is a new day and request status in preparation for daily report
  // also go into standby so don't turn on nodes and request statuses until the report is sent
  if(hour() == 0 && minute() == 0 && sentNewDayAlertToNodes == false) {
    Serial.println(F("\nIt is midnight; alerting the nodes and asking for final statuses"));
    uint8_t node;
    for(node=1; node<=16; node++) {
      // if registered
      if(gardenStatus.nodeStatusPtrs[node] != NULL) {
        // if connected
        if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED) {
          char placeholder = '0';
          safeMeshWrite(mesh.getAddress(node), &placeholder, IS_NEW_DAY_H, sizeof(placeholder), DEFAULT_SEND_TRIES);
        }
      }
    }
    sentNewDayAlertToNodes = true;
    gardenStatus.isAwake = false;
  }

  // at 12:01, after statuses are all collected, call isNewDay to send report to the serverr
  if(hour() == 0 && minute() == 2 && second() == 0 && calledIsNewDay == false) {
    Serial.println(F("\nIt is 12:02 AM; sending the daily report to the server"));
    isNewDay();
    setupGarden();
    timeInit();
    calledIsNewDay = true;
  }

  // reset the flags
  if(hour() > 0) {
    sentNewDayAlertToNodes = false;
    calledIsNewDay = false;    
  }

  // read in and respond to mesh messages
  readMeshMessages();
}

