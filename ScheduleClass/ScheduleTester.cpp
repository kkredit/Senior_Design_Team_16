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
	scheduleEvent test0;
	cout << "test0 .............................";
	assert (test0.getNodeID() == 0);
	assert (test0.getStartTime() == 0);
	assert (test0.getEndTime() == 0);
	test0.setNodeID(1);
	test0.setStartTime(02.00);
	test0.setEndTime(03.00);
	assert (test0.getNodeID() == 1);
	assert (test0.getStartTime() == 02.00);
	assert (test0.getEndTime() == 03.00);
	cout << "passed" << endl;

	scheduleEvent test1;
	cout << "test1 .............................";
	assert (test1.getNodeID() == 0);
	assert (test1.getStartTime() == 0);
	assert (test1.getEndTime() == 0);
	test1.setNodeID(1);
	test1.setStartTime(12.00);
	test1.setEndTime(15.00);
	assert (test1.getNodeID() == 1);
	assert (test1.getStartTime() == 12.00);
	assert (test1.getEndTime() == 15.00);
	cout << "passed" << endl;

	scheduleEvent test2;
	cout << "test2 .............................";
	assert (test2.getNodeID() == 0);
	assert (test2.getStartTime() == 0);
	assert (test2.getEndTime() == 0);
	test2.setNodeID(5);
	test2.setStartTime(2.00);
	test2.setEndTime(15.00);
	assert (test2.getNodeID() == 5);
	assert (test2.getStartTime() == 2.00);
	assert (test2.getEndTime() == 15.00);
	cout << "passed" << endl;

	cout << "test3 .............................";
	assert (test0 < test1);
	assert (test0 == test2);
	assert (!(test0 == test1));
	assert (!(test0 < test2));
	cout << "passed" << endl;

	cout << "scheduleEvent class passed all tests!" << endl;
}

void ScheduleTester::testInsert(){
	cout << "\nBeginning to test the Schedule class..." << endl;

	cout << "test0 .............................";
	Schedule test0;
	scheduleEvent event0(6,.20,5.00);
	test0.insert(0,event0);
	scheduleEvent event1(1,1.00,2.00);
	test0.insert(0,event1);
	scheduleEvent event2(1,1.00,2.00);
	test0.insert(0,event2);
	scheduleEvent event3(1,.5,1.00);
	test0.insert(0,event3);
	assert(test0.days[0].size() == 4);
	assert(test0.popFrontStartTime(0) == event0);
	assert(test0.days[0].size()  == 3);
	assert(test0.popFrontStartTime(0) == event3);
	assert(test0.days[0].size()  == 2);
	assert(test0.popFrontStartTime(0) == event1);
	assert(test0.days[0].size()  == 1);
	assert(test0.popFrontStartTime(0) == event2);
	assert(test0.days[0].size()  == 0);
	cout << "passed" << endl;

	cout << "test1 .............................";
	Schedule test1;
	scheduleEvent event4(6,5.20,5.00);
	test1.insert(1,event4);
	scheduleEvent event5(1,5.00,2.00);
	test1.insert(1,event5);
	scheduleEvent event6(1,4.45,2.00);
	test1.insert(1,event6);
	scheduleEvent event7(1,4.30,1.00);
	test1.insert(1,event7);
	assert(test1.days[1].size() == 4);
	assert(test1.popFrontStartTime(1) == event7);
	assert(test1.days[1].size() == 3);
	assert(test1.popFrontStartTime(1) == event6);
	assert(test1.days[1].size() == 2);
	assert(test1.popFrontStartTime(1) == event5);
	assert(test1.days[1].size() == 1);
	assert(test1.popFrontStartTime(1) == event4);
	assert(test1.days[1].size() == 0);
	cout << "passed" << endl;

	cout << "test2 .............................";
	Schedule test2;
	scheduleEvent event8(2,1.00,2.00);
	for (unsigned i=0; i<=6; i++){
		test2.insert(i,event8);
	}
	for (unsigned i=0; i<=6; i++){
		assert(test2.days[i].size() == 1);
	}
	scheduleEvent tempEvent;
	cout << endl;
	for (unsigned i = 0; i <= 6; i++){
		tempEvent = test2.popFrontStartTime(i);
		cout << "Schedule for day " << i << " contains a nodeID:" << tempEvent.getNodeID() << " startTime: " << tempEvent.getStartTime() << " endTime: " << tempEvent.getEndTime() << endl;
	}
	cout << "passed" << endl;

	cout << "Schedule Class passed all tests!" << endl;
}
