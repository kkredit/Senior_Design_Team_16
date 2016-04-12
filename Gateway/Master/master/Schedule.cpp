/*
 * Schedule.cpp
 *
 *  Created on: Feb 9, 2016
 *      Author: charleskingston
 */

#include "Schedule.h"

/*
Schedule::Schedule(){

}
*/

/*
 * Author: Charles A. Kingston
 * Parameters: An integer for which day to insert to and a
 * scheduleEvent object to insert
 * Purpose: Function that inserts a new item uint8_t o the list
 * on the passed in day. The list will order itself, based
 * upon the operators defined in the schedule class, with
 * the earliest start times first
 */
void Schedule::insert(uint8_t day, ScheduleEvent event){
  days[day].push_front(event);
  days[day].sort();
}

/*
 * Author: Charles A. Kingston
 * Parameters: An integer to select the day to delete the list
 * Purpose: Function that deletes the list for a certain day
 * passed in by the user
 */
void Schedule::deleteDaysSchedule(uint8_t day){
  days[day].erase(days[day].begin(),days[day].end());
}

bool Schedule::isEmpty(uint8_t day) {
  if(days[day].empty()) {
    return true;
  } else {
    return false;
  }
}

/*
 * Author: Charles A. Kingston
 * Parameters: An integer to select the day
 * Purpose: Function that returns the first items in the list
 * at the specified day and deletes that item. This function
 * can be compared to pop() for a stack
 */
ScheduleEvent Schedule::popFrontStartTime(uint8_t day){
  ScheduleEvent tempEvent = days[day].front();
  days[day].pop_front();
  return tempEvent;
}

bool Schedule::shouldValveBeOpen(uint8_t dayofweek, uint8_t hour, uint8_t minute, uint8_t node, uint8_t valve){
  // check for empty schedule...
  if(days[dayofweek].empty()) return false;
  
  // else, check each event  
  bool result = false;
  // for each event in given day
  list<ScheduleEvent>::const_iterator iter;
  for(iter=days[dayofweek].begin(); iter != days[dayofweek].end(); iter++){
    // if given event is for this node and valve
    if(iter->getNodeID() == node && iter->getValveNum() == valve){
      // if event is active, then return true
      // convert time to absolute minutes to simplify comparisons
      uint16_t currentTime = 60*hour+minute;
      uint16_t startTime = 60*iter->getStartHour()+iter->getStartMin();
      uint16_t endTime = 60*iter->getEndHour()+iter->getEndMin();      
      if(currentTime >= startTime && currentTime < endTime){
        result = true;
        break;
      }
    }
  }
  return result;
  
}
//  // check for empty schedule...
//  if(days[dayofweek].empty()) return false;
//  
//  // else, check each event  
//  bool result = false;
//  // for each event in given day
//  list<ScheduleEvent>::const_iterator iter;
//  for(iter=days[dayofweek].begin(); iter != days[dayofweek].end(); iter++){
//    // if given event is for this node and valve
//    if(iter->getNodeID() == node && iter->getValveNum() == valve){
//      // if event is active, then return true
//      if(iter->getStartHour() <= hour && iter->getStartMin() <= minute && iter->getEndHour() <= hour && iter->getEndMin() <= minute){
//        result = true;
//        break;
//      }
//    }
//  }
//  return result;
//}


