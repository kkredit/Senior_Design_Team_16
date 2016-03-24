/*
 * scheduleEvent.h
 *
 *  Created on: Feb 11, 2016
 *      Author: charleskingston
 */

#ifndef SCHEDULEEVENT_H_
#define SCHEDULEEVENT_H_

class ScheduleEvent {
public:
  ScheduleEvent();
  ScheduleEvent(int userNodeID, int userStartHour, int userStartMin, int userEndHour, int userEndMin);
  int getNodeID();
  int getStartHour();
  int getStartMin();
  int getEndHour();
  int getEndMin();
  void setNodeID(int id);
  void setStartHour(int time);
  void setStartMin(int time);
  void setEndHour(int time);
  void setEndMin(int time);
  bool operator<(const ScheduleEvent& ScheduleEvent2) const;
  bool operator>(const ScheduleEvent& ScheduleEvent2) const;
  bool operator==(const ScheduleEvent& ScheduleEvent2) const;
private:
  int nodeID;
  int startHour;
  int startMin;
  int endHour;
  int endMin;
};

#endif /* SCHEDULEEVENT_H_ */
