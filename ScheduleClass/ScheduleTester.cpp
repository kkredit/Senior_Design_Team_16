/*
 * ScheduleTester.cpp
 *
 *  Created on: Feb 9, 2016
 *      Author: charleskingston
 *
 *      Tester that tests the functionality of the both the
 *      scheduleEvent class as well as the Schedule class that
 *      uses the scheduleEventClass
 */

#include "ScheduleTester.h"

ScheduleTester::ScheduleTester() {
	// TODO Auto-generated constructor stub

}

void ScheduleTester::runTests(){
	testScheduleEvents();
	testInsert();
}

void ScheduleTester::testScheduleEvents () {
	cout << "\nBeginning to test the ScheduleEvents..." << endl;
	ScheduleEvent test0;
	cout << "test0 .............................";
	assert (test0.getNodeID() == 0);
	assert (test0.getStartHour() == 0);
	assert (test0.getStartMin() == 0);
	assert (test0.getEndHour() == 0);
	assert (test0.getEndMin() == 0);
	test0.setNodeID(1);
	test0.setStartHour(2);
	test0.setEndHour(3);
	assert (test0.getNodeID() == 1);
	assert (test0.getStartHour() == 2);
	assert (test0.getStartMin() == 0);
	assert (test0.getEndHour() == 3);
	assert (test0.getEndMin() == 0);
	cout << "passed" << endl;

	ScheduleEvent test1;
	cout << "test1 .............................";
	assert (test1.getNodeID() == 0);
	assert (test1.getStartHour() == 0);
	assert (test1.getStartMin() == 0);
	assert (test1.getEndHour() == 0);
	assert (test1.getEndMin() == 0);
	test1.setNodeID(1);
	test1.setStartHour(2);
	test1.setStartMin(12);
	test1.setEndHour(3);
	test1.setEndMin(15);
	assert (test1.getNodeID() == 1);
	assert (test1.getStartHour() == 2);
	assert (test1.getStartMin() == 12);
	assert (test1.getEndHour() == 3);
	assert (test1.getEndMin() == 15);
	cout << "passed" << endl;

	ScheduleEvent test2;
	cout << "test2 .............................";
	assert (test2.getNodeID() == 0);
	assert(test2.getStartHour() == 0);
	assert(test2.getStartMin() == 0);
	assert(test2.getEndHour() == 0);
	assert(test2.getEndMin() == 0);
	test2.setNodeID(5);
	test2.setStartHour(2);
	test2.setStartMin(12);
	test2.setEndHour(3);
	test2.setEndMin(15);
	assert (test2.getNodeID() == 5);
	assert(test2.getStartHour() == 2);
	assert(test2.getStartMin() == 12);
	assert(test2.getEndHour() == 3);
	assert(test2.getEndMin() == 15);
	cout << "passed" << endl;

	cout << "test3 .............................";
	assert (test0 < test1);
	assert (test2 == test1);
	assert (!(test0 == test2));
	assert (!(test0 > test2));
	cout << "passed" << endl; 

	cout << "scheduleEvent class passed all tests!" << endl;
}

void ScheduleTester::testInsert(){
	cout << "\nBeginning to test the Schedule class..." << endl;

	cout << "test0 .............................";
	Schedule test0;
	ScheduleEvent event0(0, 3, 10, 5, 20);
	test0.insert(0,event0);
	ScheduleEvent event1(1, 12, 30, 15, 45);
	test0.insert(0,event1);
	ScheduleEvent event2(2, 3, 23, 6, 45);
	test0.insert(0,event2);
	ScheduleEvent event3(3, 16, 55, 21, 45);
	test0.insert(0,event3);
	assert(test0.days[0].size() == 4);
	assert(test0.popFrontStartTime(0) == event0);
	assert(test0.days[0].size()  == 3);
	assert(test0.popFrontStartTime(0) == event2);
	assert(test0.days[0].size()  == 2);
	assert(test0.popFrontStartTime(0) == event1);
	assert(test0.days[0].size()  == 1);
	assert(test0.popFrontStartTime(0) == event3);
	assert(test0.days[0].size()  == 0);
	cout << "passed" << endl;

	cout << "test1 .............................";
	Schedule test1;
	ScheduleEvent event4(3, 0, 0, 2, 25);
	test1.insert(1,event4);
	ScheduleEvent event5(1, 23, 0, 23, 45);
	test1.insert(1,event5);
	ScheduleEvent event6(1, 4, 15, 8, 20);
	test1.insert(1,event6);
	ScheduleEvent event7(1, 4, 0, 8, 56);
	test1.insert(1,event7);
	assert(test1.days[1].size() == 4);
	assert(test1.popFrontStartTime(1) == event4);
	assert(test1.days[1].size() == 3);
	assert(test1.popFrontStartTime(1) == event7);
	assert(test1.days[1].size() == 2);
	assert(test1.popFrontStartTime(1) == event6);
	assert(test1.days[1].size() == 1);
	assert(test1.popFrontStartTime(1) == event5);
	assert(test1.days[1].size() == 0);
	cout << "passed" << endl;

	cout << "test2 .............................";
	Schedule test2;
	ScheduleEvent event8(2, 1, 0, 2, 0);
	for (unsigned i=0; i<=6; i++){
		test2.insert(i,event8);
	}
	for (unsigned i=0; i<=6; i++){
		assert(test2.days[i].size() == 1);
	}
	ScheduleEvent tempEvent;
	cout << endl;
	for (unsigned i = 0; i <= 6; i++){
		tempEvent = test2.popFrontStartTime(i);
		cout << "Schedule for day " << i << " contains a nodeID:" << tempEvent.getNodeID() << " startTime: " \
			 << tempEvent.getStartHour() << ":" << tempEvent.getStartMin() <<" endTime: " << tempEvent.getEndHour() \
			 << ":" << tempEvent.getEndMin() << endl;
	}
 	cout << "passed" << endl; 

	cout << "Schedule Class passed all tests!" << endl; 
}
