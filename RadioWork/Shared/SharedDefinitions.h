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
};
 
// Valve_Status struct
struct Valve_Status{
  bool isConnected;
  bool state;
};

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
  Valve_Status valveState1;
  Valve_Status valveState2;
  Valve_Status valveState3;
  Valve_Status valveState4;
  uint8_t meshState;
  uint8_t nodeID;
  int16_t nodeMeshAddress;
};
 
// mesh settings
#define MASTER_ADDRESS      0             // Address (and nodeID) of the master
#define COMM_CHANNEL        12            // Channel to use for communication
#define DATA_RATE           RF24_250KBPS  // RF24 mesh communication data rate
#define CONNECT_TIMEOUT     15000         // time in ms before mesh.begin() times out
#define DEFAULT_SEND_TRIES  5             // number of tries to send a message if send fails but are
                                          // connected  
#define RETRY_PERIOD        1000          // number of ms to wait before retrying to send a message
#define RENEWAL_TIMEOUT     15000         // time in ms before mesh.renewAddress() times out
#define CONNECTION_TRIES    5             // number of tries to connect before waiting going to 
                                          // sleep
#define DISCONNECTED_SLEEP  (15*60000)    // (15 minutes) -- time to wait before trying to connect 
                                          // again


// communication protocol settings -- _H stands for message Headers, _P stands for message Payloads
// master to nodes
#define SET_VALVE_H         'V'
//#define INFO_REQUEST_H      'R'     DEPRICATED
//#define GET_VALVE_P         'v'     DEPRICATED
//#define GET_FLOW_RATE_P     'r'     DEPRICATED
//#define GET_NODE_STATUS_P   'n'     DEPRICATED
#define GET_NODE_STATUS_H   'S'
#define FORCE_RESET_H       'R'
#define IS_NEW_DAY_H        'D'
// nodes to master
#define SEND_VALVE_H        'v'
//#define SEND_FLOW_RATE_H    'r'     DEPRICATED
#define SEND_NODE_STATUS_H  's'
#define SEND_JUST_RESET_H   'r'
#define SEND_NODE_SLEEP_H   'l'
#define SEND_NEW_DAY_H      'd'


// node states -- apply equally to "node" proper and to master
// voltage states
#define GOOD_VOLTAGE      0
#define LOW_VOLTAGE       1
#define HIGH_VOLTAGE      2
// mesh states
#define MESH_CONNECTED    0
#define MESH_DISCONNECTED 1
// flow states
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
#define NODE_IS_ASLEEP_ERROR  -2      // when node is asleep


// other settings
#define ON                    1
#define OFF                   0
#define ALL_VALVES            5
#define BAUD_RATE             9600    // serial communication baud rate
#define TIMER1_PERIOD         4000000 /* 10s */ // timer period in microseconds (1000000 = 1 sec)
#define RATE_MEASURING_PERIOD 5000    // number of ms over which to collect flowrate data
#define VIN_EEPROM_ADDR       0       // address of the input voltage reading in EEPROM
#define RESET_EEPROM_ADDR     4       // address of the toldToReset bool in EEPROM
#define ACC_FLOW_EEPROM_ADDR  8       // address of accumulatedFlow float in EEPROM
#define OK_VIN_RANGE          0.15    // amount that VIN can vary without generating an error
#define MIN_MEASUREABLE_GPM   0.2642  // is 1 LPM, minimum measuring threshold for our meter
#define MAX_MEASUREABLE_GPM   7.92516 // is 29 LPM, maximumm measureing threshold for our meter
#define FLOW_RATE_SAMPLE_SIZE 100
