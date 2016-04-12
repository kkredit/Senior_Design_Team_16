/*
 * Schedule.h
 *
 *  Created on: Feb 9, 2016
 *      Author: charleskingston
 */

#ifndef SCHEDULE_H_
#define SCHEDULE_H_
#include <List>
using namespace std;

#include "scheduleEvent.h"
#include <Arduino.h>

class Schedule {
public:
  Schedule() {};
  void insert(uint8_t day, ScheduleEvent event);
  void deleteDaysSchedule(uint8_t day);
  bool isEmpty(uint8_t day);
  ScheduleEvent popFrontStartTime(uint8_t day);
  bool shouldValveBeOpen(uint8_t dayofweek, uint8_t hour, uint8_t minute, uint8_t node, uint8_t valve);//{
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
  // TODO fix that^^!
private:
  list<ScheduleEvent> days[7];
  friend class ScheduleTester;
};

#endif /* SCHEDULE_H_ */

