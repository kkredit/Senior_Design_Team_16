from garden_net.database.base import Base

from sqlalchemy import Column, Float, String, Integer, ForeignKey

class Event(Base):
	__tablename__ = 'events'

	id = Column(Integer, primary_key=True)
	_start_time = Column(Float)
	_stop_time = Column(Float)
	_day = Column(String(10))
	_owner = Column(Integer, ForeignKey('zones._zone_id'))

	def __init__(self, start, stop, myDay, zone_owner):
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
				or myDay =='Saturday':
			self._day = myDay
		else:
			raise ValueError("Specify a day of the week")
		self._owner = zone_owner

	def __str__(self):
		return self.day + ": start_time: " + str(self.start_time) + " stop_time: " + str(self.stop_time)\
			   + " zoneID: " + str(self.owner)

	def __lt__(self, other):
		if self.start_time < other.start_time and self.owner == other.owner:
			return True
		else:
			return False

	def __gt__(self, other):
		if self.start_time > other.start_time and self.owner == other.owner:
			return True
		else:
			return False

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

	def to_JSON(self):
		return "{\"start_time\" : \"" + str(self.start_time) + "\"" + ", \"stop_time\" : \"" + str(self.stop_time) + \
			   ", \"day\" : \"" + str(self.day) + "\", \"zone_ID\" : \"" + str(self.owner) + "\"}"

if __name__ == "__main__":
	e = Event(1.0, 2.0, 'Monday', 1)

	e_JSON = e.to_JSON()
	print(e_JSON)
