/*
 * scheduleEvent.h
 *
 *  Created on: Feb 11, 2016
 *      Author: charleskingston
 */

#ifndef SCHEDULEEVENT_H_
#define SCHEDULEEVENT_H_

#include <Arduino.h>

class ScheduleEvent {
public:
  ScheduleEvent();
  ScheduleEvent(uint8_t userNodeID, uint8_t userValveNum, uint8_t userStartHour, uint8_t userStartMin, uint8_t userEndHour, uint8_t userEndMin);
  uint8_t getNodeID()     { return nodeID;    };
  uint8_t getValveNum()   { return valveNum;  };
  uint8_t getStartHour()  { return startHour; };
  uint8_t getStartMin()   { return startMin;  };
  uint8_t getEndHour()    { return endHour;   };
  uint8_t getEndMin()     { return endMin;    };
  void setNodeID(uint8_t id)      { nodeID = id;      };
  void setValveNum(uint8_t valve) { valveNum = valve; };
  void setStartHour(uint8_t time) { startHour = time; };
  void setStartMin(uint8_t time)  { startMin = time;  };
  void setEndHour(uint8_t time)   { endHour = time;   };
  void setEndMin(uint8_t time)    { endMin = time;    };
  bool operator<(const ScheduleEvent& ScheduleEvent2) const;
  bool operator>(const ScheduleEvent& ScheduleEvent2) const;
  bool operator==(const ScheduleEvent& ScheduleEvent2) const;
private:
  uint8_t nodeID;
  uint8_t valveNum;
  uint8_t startHour;
  uint8_t startMin;
  uint8_t endHour;
  uint8_t endMin;
};

#endif /* SCHEDULEEVENT_H_ */

