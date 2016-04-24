/*
 * SharedDefinitions.h
 *
 * A file to put all settings definitions into; shared between master and node code
 * For the GardeNet system
 *
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last modified: 4/3/16
 */
 
 
// Valve_Command struct
struct Valve_Command{
  uint8_t whichValve;
  bool onOrOff;
  uint8_t timeToLive;
};
 
// Valve_Status struct
struct Valve_Status{
  bool isConnected;
  bool state;
  uint32_t timeSpentWatering; // in seconds
};

// Valve_Response struct
// essentially a combination of a command and a status
// struct Valve_Response{
  // bool nodeIsAwake;
  // uint8_t whichValve;
  // bool isConnected;
  // bool commandedOnOrOff;
  // int8_t actualState;
  // uint8_t timeToLive;
// };

// Node_Status struct
struct Node_Status{
  bool isAwake;
  uint16_t storedVIN;
  uint8_t voltageState;
  bool hasFlowRateMeter;
  float currentFlowRate;
  uint8_t flowState;
  float accumulatedFlow;
  bool maxedOutFlowMeter;
  uint8_t numConnectedValves;
  uint8_t numOpenValves;
  Valve_Status valveStates[5];
  uint8_t meshState;
  uint8_t nodeID;
  int16_t nodeMeshAddress;
  float percentAwake;
  float percentMeshUptime;
};

// Garden_Status struct
struct Garden_Status{
  bool isAwake;
  uint8_t threeGState;
  uint8_t meshState;
  uint8_t gardenState;
  uint8_t numRegisteredNodes;
  uint8_t numConnectedNodes;
  Node_Status* nodeStatusPtrs[17];
  float percentAwake;
  float percent3GUptime;
  float percentMeshUptime;
};
 
// mesh settings
#define MASTER_ADDRESS      0             // Address of the master
#define MASTER_NODE_ID      0             // Node ID of the master
#define COMM_CHANNEL        12            // Channel to use for communication
#define DATA_RATE           RF24_250KBPS  // RF24 mesh communication data rate
#define CONNECT_TIMEOUT     15000         // time in ms before mesh.begin() times out
#define DEFAULT_SEND_TRIES  5             // number of tries to send a message if send fails but are
                                          // connected  
#define RETRY_PERIOD        1000          // number of ms to wait before retrying to send a message
#define RENEWAL_TIMEOUT     15000         // time in ms before mesh.renewAddress() times out
#define CONNECTION_TRIES    5             // number of tries to connect before waiting going to 
                                          // sleep
#define DISCONNECTED_SLEEP  (5*60000)     // (5 minutes) -- time to wait before trying to connect 
#define PERIOD_ASKS_STATUS  50
                                          // again


// communication protocol settings
// master to nodes
#define SET_VALVE_H         'V'
#define GET_NODE_STATUS_H   'S'
#define FORCE_RESET_H       'R'
#define IS_NEW_DAY_H        'D'
#define CONNECTION_TEST_H   'T'
// nodes to master
//#define SEND_VALVE_H        'v'       // DEPRICATED
#define SEND_NODE_STATUS_H  's'
#define SEND_JUST_RESET_H   'r'
#define SEND_NODE_SLEEP_H   'l'
//#define SEND_NEW_DAY_H      'd'       // DEPRICATED


// master states
// threeGStates
#define TR_G_CONNECTED        0
#define TR_G_DISCONNECTED     1
// meshStates
#define MESH_ALL_NODES_GOOD   0
#define MESH_NOT_BEGAN        1
#define MESH_SOME_NODES_DOWN  2
#define MESH_ALL_NODES_DOWN   3
// gardenStates
#define GARDEN_ALL_IS_WELL    0
#define GARDEN_NODE_ERROR     1


// node states
// voltageStates
#define GOOD_VOLTAGE      0
#define LOW_VOLTAGE       1
#define HIGH_VOLTAGE      2
// meshStates
#define MESH_CONNECTED    0
#define MESH_DISCONNECTED 1
// flowStates
#define HAS_NO_METER      0
#define NO_FLOW_GOOD      1
#define FLOWING_GOOD      2
#define LOW_FLOW_RATE     3
#define HIGH_FLOW_RATE    4
#define STUCK_AT_OFF      5
#define STUCK_AT_ON       6


// LED settings
#define LED_BRIGHTNESS        255     // 0-255; brightness of LEDR_BRIGHTNESS
#define LED_OFF               0
#define LED_ON                1
#define TURN_ON_SEQUENCE      2
#define CONNECTED_SEQUENCE    3
#define DISCONNECTED_SEQUENCE 4
#define SPECIAL_BOOT_SEQUENCE 5
#define GO_TO_SLEEP_SEQUENCE  6
#define AWAKE_SEQUENCE        7


// valve command errors
#define NO_VALVE_ERROR        -1      // when no valve connected in given slot


// alert engine op code 
//////// TEST RESULTS //////////////    Sends       Sends at correct time        Sends without duplicates
#define DAILY_REPORT          00  //
#define BAD_FLOW_RATE         01  //
#define MESH_DOWN             02  //    yes               yes                         yes
#define GATEWAY_RESET         03  //    yes               yes                         yes
#define BAD_VOLTAGE_STATE     04  //    yes               yes                         yes
#define GARDEN_TOGGLE         05  //    yes               yes                         yes  
                                  // TODO -- GARDEN_TOGGLE should move the "full garden sut off" switch to reflect the state


// other settings
#define ON                    true
#define OFF                   false
#define ALL_VALVES            5
#define BAUD_RATE             9600    // serial communication baud rate
#define TIMER1_PERIOD         5000000 /* 5s */ // timer period in microseconds (1000000 = 1 sec)
#define RATE_MEASURING_PERIOD 5000    // number of ms over which to collect flowrate data
#define VIN_EEPROM_ADDR       0       // address of the input voltage reading in EEPROM
#define RESET_EEPROM_ADDR     4       // address of the toldToReset bool in EEPROM
#define ACC_FLOW_EEPROM_ADDR  8       // address of accumulatedFlow float in EEPROM
#define OK_VIN_RANGE          0.15    // amount that VIN can vary without generating an error
#define MIN_MEASUREABLE_GPM   0.2642  // is 1 LPM, minimum measuring threshold for our meter 
                                      //  also serves as max ok leakage rate
#define MAX_MEASUREABLE_GPM   7.92516 // is 29 LPM, maximumm measureing threshold for our meter
#define FLOW_RATE_SAMPLE_SIZE 100
#define VALVE_COMMAND_TTL     5       // number of tries to execute a valve command