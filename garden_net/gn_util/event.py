class Event:
	def __init__(self, start, stop):
		if start > stop:
			raise ValueError("Start time should be less than stop time")
		if start > 23.59:
			raise ValueError("Start time should be less than 24.00")
		if stop > 23.59:
			raise ValueError("Stop time should be less than 24.00")
		else:
			self._start_time = start
			self._stop_time = stop

	def __str__(self):
		return "start_time: " + str(self.start_time) + " stop_time: " + str(self.stop_time)

	def __lt__(self, other):
		if self.start_time < other.start_time:
			return True
		else:
			return False

	def __gt__(self, other):
		if self.start_time > other.start_time:
			return True
		else:
			return False

	def __eq__(self, other):
		if self.start_time == other.start_time:
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
		if value > 24.00:
			raise ValueError("start_time should be less than 24.00")
		else:
			raise ValueError("start_time should be less than  stop_time")

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


########################## Testing for the event class ##########################
if __name__ == "__main__":
	event = Event(1.0,2.0)
	event2 = Event(2.0,3.0)
	assert (event2 > event)
	assert (not(event2 < event))
	try:
		event2.start_time = 5.0
		print("not passed")
		exit()
	except ValueError:
		pass

	print("passed")