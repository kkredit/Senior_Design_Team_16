/*
 * ScheduleTester.h
 *
 *  Created on: Feb 9, 2016
 *      Author: charleskingston
 */

#ifndef SCHEDULETESTER_H_
#define SCHEDULETESTER_H_

#include "Schedule.h"
#include "scheduleEvent.h"

#include <iostream>
#include <cassert>
using namespace std;

class ScheduleTester {
public:
	ScheduleTester();
	void runTests();
	void testScheduleEvents();
	void testInsert();
};

#endif /* SCHEDULETESTER_H_ */
