/*
 * settings.h
 * A file to put all settings definitions into; shared between master and node code
 * For the GardeNet system
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last modified: 2/8/16
 */
 
// mesh settings
#define MASTER_ADDRESS      0             // Address (and nodeID) of the master
#define COMM_CHANNEL        12            // Channel to use for communication
#define DATA_RATE           RF24_250KBPS  // RF24 mesh communication data rate
#define CONNECT_TIMEOUT     15000         // time in ms before mesh.begin() times out
#define DEFAULT_SEND_TRIES  5             // number of tries to send a message if send fails but are connected  
#define RETRY_PERIOD        1000          // number of ms to wait before retrying to send a message
#define RENEWAL_TIMEOUT     15000         // time in ms before mesh.renewAddress() times out
#define CONNECTION_TRIES    5             // number of tries to connect before waiting going to sleep
#define DISCONNECTED_SLEEP  (15*60000)    // (15 minutes) -- time to wait before trying to connect again


// communication protocol settings -- _H stands for message Headers, _P stands for message Payloads
// master to nodes
#define SET_VALVE_H         'V'
#define INFO_REQUEST_H      'R'
#define GET_VALVE_P         'v'
#define GET_FLOW_RATE_P     'r'
#define GET_NODE_STATUS_P   'n'
#define FORCE_RESET_H       'F'
// nodes to master
#define SEND_VALVE_H        'v'
#define SEND_FLOW_RATE_H    'r'
#define SEND_NODE_STATUS_H  'n'


// node states
#define NODE_OK                 0
#define NODE_DISCONNECTED       1
#define NODE_VALVE_ERROR        2
#define NODE_LOW_BATTERY        3
#define NODE_IS_MASTER          4


// LEDR settings
#define LEDR_BRIGHTNESS       255     // 0-255; brightness of LEDR_BRIGHTNESS
#define LEDR_OFF              0       // LEDR light pattern
#define LEDR_ON               1       // LEDR light pattern
#define TURN_ON_SEQUENCE      2       // LEDR light pattern
#define CONNECTED_SEQUENCE    3       // LEDR light pattern
#define DISCONNECTED          4       // LEDR light pattern


// other settings
#define BAUD_RATE             9600    // serial communication baud rate
#define TIMER1_PERIOD  /*60*/ 60000000 // timer period in microseconds (1000000 = 1 sec)
#define RATE_MEASURING_PERIOD 5000    // number of ms over which to collect flowrate data
