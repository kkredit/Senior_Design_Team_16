/*
 * SharedFunctions.h
 * A file to write all functions shared between the master and the nodes
 * For the GardeNet system
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last modified: 2/8/16
 */
 
 /*
 * A function to do a mesh.write with automatic error handling
 * @param void* payload: a pointer to the payload
 * @param char header: the type of message
 * @param unsigned datasize: the size of the payload in bytes (use the sizeof() function)
 * @param unsigned timesToTry: the number of times to try to send the message if send fails
 * @return bool: true if send success, false if send fail
 */
bool safeMeshWrite(uint8_t destination, void* payload, char header, uint8_t datasize, uint8_t timesToTry);
