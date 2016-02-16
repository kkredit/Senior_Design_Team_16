/*
 * scheduleEvent.h
 *
 *  Created on: Feb 11, 2016
 *      Author: charleskingston
 */

#ifndef SCHEDULEEVENT_H_
#define SCHEDULEEVENT_H_

class scheduleEvent {
public:
	scheduleEvent();
	scheduleEvent(int userNodeID, float userStartTime, float userEndTime);
	int getNodeID();
	float getStartTime();
	float getEndTime();
	void setNodeID(int id);
	void setStartTime(float time);
	void setEndTime(float time);
	bool operator<(const scheduleEvent& scheduleEvent2) const;
	bool operator>(const scheduleEvent& scheduleEvent2) const;
	bool operator==(const scheduleEvent& scheduleEvent2) const;
private:
	int nodeID;
	float startTime;
	float endTime;
};

#endif /* SCHEDULEEVENT_H_ */
