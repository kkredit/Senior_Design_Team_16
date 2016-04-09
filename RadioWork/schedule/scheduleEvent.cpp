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
ScheduleEvent::ScheduleEvent() {
	nodeID = 0;
  valveNum = 0;
	startHour = 0;
	startMin = 0;
	endHour = 0;
	endMin = 0;
}

/*
 * Author: Charles A. Kingston
 * Parameters: integer called userNodeID, float called userStartTime, and
 * float called userEndTime
 * Purpose: Constructor for the scheduleEvent class in
 * which all of its properties are initialized to 0
 */
ScheduleEvent::ScheduleEvent(uint8_t userNodeID, uint8_t userValveNum, uint8_t userStartHour, uint8_t userStartMin, uint8_t userEndHour, uint8_t userEndMin){
	nodeID = userNodeID;
  valveNum = userValveNum;
	startHour = userStartHour;
	startMin = userStartHour;
	endHour = userEndHour;
	endMin = userEndMin;
}

/*
 * Author: Charles A. Kingston
 * Parameters:
 * Purpose: Function that defines the less than operator
 * between to schedule events by comparing their starTime
 * properties
 */
bool ScheduleEvent::operator<(const ScheduleEvent& scheduleEvent2) const {
	// comparing hour
	if (this->startHour < scheduleEvent2.startHour){
		return true;
	}
	// comparing minute
	else if (this->startHour == scheduleEvent2.startHour) {
		if (this->startMin < scheduleEvent2.startMin) {
			return true;
		} else {
			return false;
		}
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
bool ScheduleEvent::operator>(const ScheduleEvent& scheduleEvent2) const{
	// comparing hour
	if (this->startHour > scheduleEvent2.startHour){
		return true;
	} 
	// comparing minute
	else if (this->startHour == scheduleEvent2.startHour) {
		if (this->startMin > scheduleEvent2.startMin) {
			return true;
		}
		else {
			return false;
		}
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
bool ScheduleEvent::operator==(const ScheduleEvent& scheduleEvent2) const{
	if (this->startHour == scheduleEvent2.startHour){
		if (this->startMin == scheduleEvent2.startMin) {
			return true;
		}
		else {
			return false;
		}
	} else {
		return false;
	}
}
