/*
 * SharedFunctions.cpp
 * A file to write all functions shared between the master and the nodes
 * For the GardeNet system
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last modified: 2/8/16
 */

 //#define uint8_t unsigned char

/*
 * A function to do a mesh.write with automatic error handling
 * @param void* payload: a pointer to the payload
 * @param char header: the type of message
 * @param unsigned datasize: the size of the payload in bytes (use the sizeof() function)
 * @param unsigned timesToTry: the number of times to try to send the message if send fails
 * @return bool: true if send success, false if send fail
 */
//bool safeMeshWrite(uint8_t destination, void* payload, char header, uint8_t datasize, uint8_t timesToTry){
//  // perform write
//  if (!mesh.write(destination, payload, header, datasize)) {
//    // if a write fails, check connectivity to the mesh network
//    Serial.print(F("Send fail... "));
//    if (!mesh.checkConnection() ) {
//      //refresh the network address
//      Serial.println(F("renewing Address... "));
//      if (!mesh.renewAddress()){
//        // if failed, connection is down
//        Serial.println(F("MESH CONNECTION DOWN"));
//        return false;
//      }
//      else{
//        if(timesToTry){
//          // if succeeded, are reconnected and try again
//          Serial.println(F("reconnected, trying again"));
//          delay(RETRY_PERIOD);
//          return safeMeshWrite(destination, payload, header, datasize, --timesToTry);
//        }
//        else{
//          // out of tries; send failed
//          Serial.println(F("reconnected, but out of send tries: SEND FAIL"));
//          return false;
//        }
//        
//      }
//    } 
//    else {      
//      if(timesToTry){
//        // if send failed but are connected and have more tries, try again after 50 ms
//        Serial.println(F("mesh connected, trying again"));
//        delay(RETRY_PERIOD);
//        return safeMeshWrite(destination, payload, header, datasize, --timesToTry);
//      }
//      else{
//        // out of tries; send failed
//        Serial.println(F("out of send tries: SEND FAIL"));
//        return false;
//      }
//    }    
//    //return false;
//  }
//  else {
//    // write succeeded
//    Serial.print(F("Send of type ")); Serial.print(header); Serial.println(F(" success"));
//    //Serial.println(*payload); // cannot dereference void*, and do not know type...
//    return true;
//  }
//}
