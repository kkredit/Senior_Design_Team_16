class OneHour:
	def __init__(self):
		self._current_time = 0
		self._precipProbability = 0
		self._precipIntensity = 0

	def __str__(self):
		return "current_time: " + str(self._current_time) + " set_precipProbability: " \
			   + str(self._precipProbability) + " precipIntensity: " + str(self._precipIntensity)

	def set_current_time(self, value):
		self._current_time = value

	def set_precipProbability(self, value):
		self._precipProbability = value

	def set_precipIntensity(self, value):
		self._precipIntensity = value

	def get_current_time(self):
		return self._current_time

	def get_precipProbability(self):
		return self._precipProbability

	def get_precipIntensity(self):
		return self._precipIntensity

	def get_current_hour(self):
		temp = str(self._current_time).split(" ")
		value = temp[1]
		return value[0:2]
