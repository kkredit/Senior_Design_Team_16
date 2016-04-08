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
	void insert(int day, scheduleEvent event);
	void deleteDaysSchedule(int day);
	scheduleEvent popFrontStartTime(int day);
private:
	list<scheduleEvent> days[7];
	friend class ScheduleTester;
};

#endif /* SCHEDULE_H_ */
