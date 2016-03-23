from garden_net.gn_util.schedule import Schedule
from garden_net.gn_util.event import Event

class FullSchedule:
	def __init__(self):
		self.full_schedule = {'zone1': Schedule(), 'zone2': Schedule(), 'zone3': Schedule()}

	def add_schedule_for_zone(self, zone_number, schedule_for_node):
		if zone_number in self.full_schedule.keys():
			self.full_schedule[zone_number] = schedule_for_node
		else:
			print("Error: Zone does not exist")

	def __str__(self):
		temp_string = ""
		for key in self.full_schedule:
			temp_string += key + ":\n\n" + str(self.full_schedule[key]) + '\n'
		return temp_string

	def get_schedule_for_nodeID(self, nodeID):
		if nodeID in self.full_schedule.keys():
			return self.full_schedule[nodeID]
		else:
			print("Error: Zone does not exist")

	def get_schedule_for_nodeID_and_day(self, nodeID, day):
		if nodeID in self.full_schedule.keys():
			zone_sched = self.full_schedule[nodeID]
			if day in zone1_sched.mySchedule.keys():
				return zone1_sched.get_event_at_day(day)

if __name__ == "__main__":
	schedule = FullSchedule()
	test = Schedule()
	testEvent = Event(1.00, 2.00)
	testEvent2 = Event(5.00, 6.00)
	testEvent3 = Event(8.00,10.50)
	testEvent4 = Event(2.00, 4.00)
	testEvent5 = Event(3.00, 3.15)
	testEvent6 = Event(7.00, 8.00)
	testEvent7 = Event(2.00,10.50)
	testEvent8 = Event(18.00, 19.60)
	testEvent9 = Event(23.00,24.00)
	testEvent10 = Event(22.00, 22.15)
	test.insert_event('Monday', testEvent)
	test.insert_event('Monday', testEvent2)
	test.insert_event('Monday', testEvent3)
	test.insert_event('Tuesday', testEvent4)
	test.insert_event('Tuesday', testEvent5)
	test.insert_event('Friday', testEvent6)
	test.insert_event('Sunday', testEvent7)
	test.insert_event('Saturday', testEvent8)
	test.insert_event('Wednesday', testEvent9)
	test.insert_event('Thursday', testEvent10)

	schedule.add_schedule_for_zone('zone1',test)
	schedule.add_schedule_for_zone('zone2',test)

	zone1_sched = schedule.get_schedule_for_nodeID('zone1')
	zone1_sched_mon = schedule.get_schedule_for_nodeID_and_day('zone1', 'Monday')
	#print(str(zone1_sched_mon))
	# for event in zone1_sched_mon:
	# 	print(str(event))
	print(schedule)