/*
 * settings.h
 * A file to put all settings definitions into; shared between master and node code
 * For the GardeNet system
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last modified: 2/8/16
 */
 
// mesh settings
#define MASTER_ADDRESS    0             // Address (nodeID) of the master
#define COMMCHANNEL       12            // Channel to use for communication
#define DATARATE          RF24_250KBPS  // RF24 mesh communication data rate
#define CONNECTTIMEOUT    20000         // time in ms before mesh.begin() times out
#define DEFAULTSENDTRIES  5             // number of tries to send a message if send fails but are connected  
#define RETRY_PERIOD      1000          // number of ms to wait before retrying to send a message

// protocol settings
#define GET_VALVE_STATE_H 'V'
// and so on...
