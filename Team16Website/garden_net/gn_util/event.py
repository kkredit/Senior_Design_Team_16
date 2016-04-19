"""
	Date: 04/12/2016
	Author: Charles A. Kingston

	event.py:

	Script is defines a class that is used throughout the GardeNet system. The event class
	extends the BASE that is defined. This means that it will get correctly linked to the
	Database and any other classes that are linked to it as well.

	The event class also creates a new row in the 'events' table. It adds an id to each row.

	@_start_time: Class variable to track the start_time of an event as well as add a column in
				the 'events' table of type float
	@_stop_time: Class variable to track the stop_time of an event as well as add a column in
				the 'events' table of type float
	@_day: Class variable to track the day of an event as well as add a column in the 'events'
				table of type string
	@_valve_num: Class variable to track the start_time of an event as well as add a column in
				the 'events' table of type Integer
	@_owner: Class variable to track which zone the event belongs to as well as add a column
	 			in the 'events' table of type Integer. This also adds a foreign key that links
	 			each event to a 'zone' in the 'zones' table.
"""

# Third Party Imports
from sqlalchemy import Column, Float, String, Integer, ForeignKey

# Local Imports
from base import Base

class Event(Base):
	__tablename__ = 'events'

	id = Column(Integer, primary_key=True)
	_start_time = Column(Float)
	_stop_time = Column(Float)
	_day = Column(String(10))
	_valve_num = Column(Integer)
	_owner = Column(Integer, ForeignKey('zones._zone_id'))

	"""
		A explicit constructor for the event class

		@:param self: pass ourselves
		@:param start: a int to be the self._start_time
		@:param stop: a int to be the self._stop_time
		@:param myDay: a string to be set to self._day
		@:param zone_owner: a int that sets the self._owner to the zone it belongs to
		@:param valve_id: a int that sets the self._valve_num to the valve it belongs to
	"""
	def __init__(self, start, stop, myDay, zone_owner, valve_id):
		if start > stop:
			raise ValueError("Start time should be less than stop time")
		if start > 23.59:
			raise ValueError("Start time should be less than 24.00")
		if stop > 23.59:
			raise ValueError("Stop time should be less than 24.00")
		else:
			self._start_time = start
			self._stop_time = stop
		if myDay == 'Sunday' or myDay == 'Monday' or myDay == 'Tuesday' \
				or myDay == 'Wednesday'or myDay == 'Thursday' or myDay == 'Friday' \
				or myDay =='Saturday' or myDay == 'Everyday':
			self._day = myDay
		else:
			raise ValueError("Specify a day of the week")
		self._owner = zone_owner

		if int(valve_id) >= 1 and int(valve_id) <= 4:
			self._valve_num = int(valve_id)
		else:
			raise ValueError("Legal valve numbers are 1 through 3")
	"""
		A string method for the class

		@:param self: passes self as an object to the function

		@:return: returns the JSON form of the Event

	"""
	def __str__(self):
		#return "\"start_time\": " + str(self.start_time) + " \"stop_time\": : " + str(self.stop_time)\
			   #+"\"day\": " + self.day + "\"zoneID\" :   " + str(self.owner)
		return self.to_JSON()

	"""
		A less than method for the class that allows for comparisons

		@:param self: passes self as an object to the function
		@:param other: a Event object to be compared with self

		@:return: returns True if self's start_time is less then other's start_time; else returns False
	"""
	def __lt__(self, other):
		if self.start_time < other.start_time and self.owner == other.owner:
			return True
		else:
			return False

	"""
		A greater than method for the class that allows for comparisons

		@:param self: passes self as an object to the function
		@:param other: a Event object to be compared with self

		@:return: returns True if self's start_time is greater than other's start_time; else returns False
	"""
	def __gt__(self, other):
		if self.start_time > other.start_time and self.owner == other.owner:
			return True
		else:
			return False

	"""
		A is equal method for the class that allows for comparisons

		@:param self: passes self as an object to the function
		@:param other: a Event object to be compared with self

		@:return: returns True if self's start_time is equal to other's start_time; else returns False
	"""
	def __eq__(self, other):
		if self.start_time == other.start_time and self.owner == other.owner:
			return True
		else:
			return False

	@property
	def start_time(self):
		return self._start_time

	@start_time.setter
	def start_time(self, value):
		if value < self.stop_time:
			self._start_time = value
		elif value > self.stop_time:
			raise ValueError("start_time should be less than  stop_time")
		if value > 24.00:
			raise ValueError("start_time should be less than 24.00")

	@property
	def stop_time(self):
		return self._stop_time

	@stop_time.setter
	def stop_time(self, value):
		if value > self.start_time:
			self._stop_time = value
		if value > 24.00:
			raise ValueError("stop_time should be less than 24.00")
		else:
			raise ValueError("stop_time should be greater than start_time")

	def set_stop_time(self, value: float):
		self._stop_time = value

	@property
	def day(self):
		return self._day

	@day.setter
	def day(self,value):
		if value == 'Sunday' or value == 'Monday' or value == 'Tuesday' \
				or value == 'Wednesday'or value == 'Thursday' or value == 'Friday' \
				or value =='Saturday':
			self._day = value
		else:
			raise ValueError("Specify a day of the week")

	@property
	def owner(self):
		return self._owner

	@owner.setter
	def owner(self, value):
		if isinstance(value, int):
			self._owner = value
		else:
			raise ValueError("The zone ownership should be and int")

	@property
	def valve_num(self):
		return self._valve_num

	"""
		A function to convert the Event object into a JSON formatted string

		@:param self: passes self as an object to the function

		@:return: returns the JSON string format of the Event object
	"""
	def to_JSON(self):
		return "{\"start_time\" : \"" + str(self.start_time) + "\"" + ", \"stop_time\" : \"" + str(self.stop_time) + \
				"\", \"day\" : \"" + str(self.day) + "\", \"zone_ID\" : \"" + str(self.owner) + "\" , \"valve_num\" : " \
				"\"" + str(self.valve_num) + "\" }"

if __name__ == "__main__":
	e = Event(1.15, 2.0, 'Monday', 1, 1)

	print(str(e))
	current_hour = int(e.stop_time)
	current_minute = int((e.stop_time - current_hour) * 100)
	print(current_hour)
	print(current_minute)
	new_minute = int(current_minute) + 15
	if (int(current_minute) + 15) > 60:
		new_hour = current_hour + 1
		new_minute = (current_minute + 15) - 60
		if new_minute < 10:
			temp = str(new_hour) + ".0" + str(new_minute)
			new_time = float(temp)
		else:
			temp = str(new_hour) + "." + str(new_minute)
			new_time = float(temp)
	else:
		new_minute = current_minute + 15
		temp = str(current_hour) + "." + str(new_minute)
		new_time = float(temp)
	print(new_minute)
	print(new_time)

