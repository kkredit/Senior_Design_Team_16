/*
 * Schedule.cpp
 *
 *  Created on: Feb 9, 2016
 *      Author: charleskingston
 */

#include "Schedule.h"

Schedule::Schedule() {

}

/*
 * Author: Charles A. Kingston
 * Parameters: An int eger for which day to insert to and a
 * scheduleEvent object to insert
 * Purpose: Function that inserts a new item int o the list
 * on the passed in day. The list will order itself, based
 * upon the operators defined in the schedule class, with
 * the earliest start times first
 */
void Schedule::insert(int day, ScheduleEvent event) {
  days[day].push_front(event);
  days[day].sort();
}

/*
 * Author: Charles A. Kingston
 * Parameters: An int eger to select the day to delete the list
 * Purpose: Function that deletes the list for a certain day
 * passed in by the user
 */
void Schedule::deleteDaysSchedule(int day){
  days[day].erase(days[day].begin(),days[day].end());
}

bool Schedule::isEmpty(int day) {
  if(days[day].empty()) {
    return true;
  } else {
    return false;
  }
}

/*
 * Author: Charles A. Kingston
 * Parameters: An int eger to select the day
 * Purpose: Function that returns the first items in the list
 * at the specified day and deletes that item. This function
 * can be compared to pop() for a stack
 */
ScheduleEvent Schedule::popFrontStartTime(int day){
  ScheduleEvent tempEvent = days[day].front();
  days[day].pop_front();
  return tempEvent;
}
