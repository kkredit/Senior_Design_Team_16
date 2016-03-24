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

class Schedule {
public:
  Schedule();
  void insert(int day, ScheduleEvent event);
  void deleteDaysSchedule(int day);
  ScheduleEvent popFrontStartTime(int day);
private:
  list<ScheduleEvent> days[7];
  friend class ScheduleTester;
};

#endif /* SCHEDULE_H_ */
