/*
 * scheduleEvent.cpp
 *
 *  Created on: Feb 11, 2016
 *      Author: charleskingston
 */

#include "scheduleEvent.h"

/*
 * Author: Charles A. Kingston
 * Parameters: None
 * Purpose: Constructor for the scheduleEvent class in
 * which all of its properties are initialized to 0
 */
scheduleEvent::scheduleEvent() {
	nodeID = 0;
	startTime = 0.00;
	endTime = 0.00;
}

/*
 * Author: Charles A. Kingston
 * Parameters: Integer called userNodeID, float called userStartTime, and
 * float called userEndTime
 * Purpose: Constructor for the scheduleEvent class in
 * which all of its properties are initialized to 0
 */
scheduleEvent::scheduleEvent(int userNodeID, float userStartTime, float userEndTime){
	nodeID = userNodeID;
	startTime = userStartTime;
	endTime = userEndTime;
}

/*
 * Author: Charles A. Kingston
 * Parameters: None
 * Purpose: Returns the integer nodeID property
 */
int scheduleEvent::getNodeID(){
	return nodeID;
}

/*
 * Author: Charles A. Kingston
 * Parameters: None
 * Purpose: Getter function that returns the
 * float startTime property
 */
float scheduleEvent::getStartTime(){
	return startTime;
}

/*
 * Author: Charles A. Kingston
 * Parameters: None
 * Purpose: Getter function that returns the
 * float endTime property
 */
float scheduleEvent::getEndTime(){
	return endTime;
}

/*
 * Author: Charles A. Kingston
 * Parameters: int
 * Purpose: Setter function that does not return a
 * value but allows the nodeID property to be set
 */
void scheduleEvent::setNodeID(int id){
	nodeID = id;
}

/*
 * Author: Charles A. Kingston
 * Parameters: float
 * Purpose: Setter function that does not return a
 * value but allows the startTime property to be set
 */
void scheduleEvent::setStartTime(float time){
	startTime = time;
}

/*
 * Author: Charles A. Kingston
 * Parameters: float
 * Purpose: Setter function that does not return a
 * value but allows the endTime property to be set
 */
void scheduleEvent::setEndTime(float time){
	endTime = time;
}

/*
 * Author: Charles A. Kingston
 * Parameters:
 * Purpose: Function that defines the less than operator
 * between to schedule events by comparing their starTime
 * properties
 */
bool scheduleEvent::operator<(const scheduleEvent& scheduleEvent2) const{
	if (this->startTime < scheduleEvent2.startTime){
		return true;
	} else {
		return false;
	}
}

/*
 * Author: Charles A. Kingston
 * Parameters:
 * Purpose: Function that defines the greater than operator
 * between to schedule events by comparing their starTime
 * properties
 */
bool scheduleEvent::operator>(const scheduleEvent& scheduleEvent2) const{
	if (this->startTime > scheduleEvent2.startTime){
		return true;
	} else {
		return false;
	}
}

/*
 * Author: Charles A. Kingston
 * Parameters:
 * Purpose: Function that defines the equals to operator
 * between to schedule events by comparing their starTime
 * properties
 */
bool scheduleEvent::operator==(const scheduleEvent& scheduleEvent2) const{
	if (this->startTime == scheduleEvent2.startTime){
		return true;
	} else {
		return false;
	}
}
