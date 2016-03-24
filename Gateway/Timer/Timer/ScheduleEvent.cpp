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
ScheduleEvent::ScheduleEvent(int userNodeID, int userStartHour, int userStartMin, int userEndHour, int userEndMin){
  nodeID = userNodeID;
  startHour = userStartHour;
  startMin = userStartHour;
  endHour = userEndHour;
  endMin = userEndMin;
}

/*
 * Author: Charles A. Kingston
 * Parameters: None
 * Purpose: Returns the integer nodeID property
 */
int ScheduleEvent::getNodeID(){
  return nodeID;
}

/*
 * Author: Charles A. Kingston
 * Parameters: None
 * Purpose: Getter function that returns the
 * float startTime property
 */
int ScheduleEvent::getStartHour(){
  return startHour;
}

int ScheduleEvent::getStartMin() {
  return startMin;
}

/*
 * Author: Charles A. Kingston
 * Parameters: None
 * Purpose: Getter function that returns the
 * float endTime property
 */
int ScheduleEvent::getEndHour(){
  return endHour;
}

int ScheduleEvent::getEndMin() {
  return endMin;
}

/*
 * Author: Charles A. Kingston
 * Parameters: int
 * Purpose: Setter function that does not return a
 * value but allows the nodeID property to be set
 */
void ScheduleEvent::setNodeID(int id){
  nodeID = id;
}

/*
 * Author: Charles A. Kingston
 * Parameters: float
 * Purpose: Setter function that does not return a
 * value but allows the startTime property to be set
 */
void ScheduleEvent::setStartHour(int time){
  startHour = time;
}

void ScheduleEvent::setStartMin(int time) {
  startMin = time;
}

/*
 * Author: Charles A. Kingston
 * Parameters: float
 * Purpose: Setter function that does not return a
 * value but allows the endTime property to be set
 */
void ScheduleEvent::setEndHour(int time){
  endHour = time;
}

void ScheduleEvent::setEndMin(int time) {
  endMin = time;
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
