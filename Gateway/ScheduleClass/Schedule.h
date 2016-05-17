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

#include "ScheduleEvent.h"
#include <Arduino.h>

class Schedule {
public:
  Schedule() {};
  void insert(uint8_t day, ScheduleEvent event);
  void deleteDaysSchedule(uint8_t day);
  bool isEmpty(uint8_t day);
  ScheduleEvent popFrontStartTime(uint8_t day);
  bool shouldValveBeOpen(uint8_t dayofweek, uint8_t hour, uint8_t minute, uint8_t node, uint8_t valve);
private:
  list<ScheduleEvent> days[7];
  friend class ScheduleTester;
};

#endif /* SCHEDULE_H_ */

