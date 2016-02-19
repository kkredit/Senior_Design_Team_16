from event import Event
import json

class Schedule:
	def __init__(self):
		self.mySchedule = {'Sunday': [], 'Monday': [], 'Tuesday': [], 'Wednesday': [], 'Thursday': [],
					'Friday': [], 'Saturday': []}

	def __str__(self):
		temp_string = ""
		for key in self.mySchedule:
			temp_string += key + ": "
			i = 0
			for event in self.mySchedule[key]:
				if i  == len(self.mySchedule[key])-1:
					temp_string += str(event)
				else:
					temp_string += str(event) + ", "
				i+=1
			temp_string += '\n'
		return temp_string

	def insert_event(self, myDay, myEvent):
		if myDay in self.mySchedule.keys():
			self.mySchedule[myDay].append(myEvent)
			self.mySchedule[myDay].sort()
		else:
			print("Incorrect day was specified")

	def get_event_at_day(self, day):
		if day in self.mySchedule.keys():
			return self.mySchedule[day]
		else:
			print("Incorrect day was specified")



########################## Testing for the Schedule class ##########################
if __name__ == "__main__":
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
	print(test)

	day = test.get_event_at_day('Monday')
	for event in day:
		print(str(event))

	#json.dumps(test)

####################################################################