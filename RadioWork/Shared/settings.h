/*
 * settings.h
 * A file to write all settings definitions; shared between master and node code
 * For the GardeNet system
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last modified: 2/8/16
 */
 
// mesh settings
#define MASTER_ADDRESS    0     // Address (nodeID) of the master
#define DEFAULTSENDTRIES  5     // number of tries to send a message if send fails but are connected  
#define RETRY_PERIOD      1000  // number of ms to wait before retrying to send a 