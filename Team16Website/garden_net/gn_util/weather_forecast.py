"""
	Date: 04/26/2016
	Author: Charles A. Kingston

	weather_forecast.py:

	The class Forecast uses a Forecast.io API to get a forecast for the
	upcoming days. The class also has functions to check the temperature
	and rain probability for the upcoming day such that other scripts
	can use it to manipulate the schedule of events.
"""
# Third party imports
from forecastiopy import ForecastIO, FIOHourly, FIODaily
from datetime import datetime, timedelta
from pytz import timezone
from send_email import Alert

class Forecast:
	"""
		Constructor

		The constructor uses the api key and the location of CCG
		to create a ForecastIO object.
	"""
	def __init__(self):
		self._apikey = 'f119a2dd4a7d21069aebce2924af6f9a'
		# CCG location coordinates: 42.8057347, -85.5263732
		self._my_location = [42.8057347, -85.5263732]

		self._fio = ForecastIO.ForecastIO(self._apikey, latitude=self._my_location[0], longitude=self._my_location[1])

	"""
		A function to get the hourly forecast for the next 24 hours

		@:param self, pass ourselves in

		@:return a string of the 24 hour weather forecast
	"""
	def get_forecast(self):
		total_forecast = ""
		if self._fio.has_hourly() is True:
			hourly = FIOHourly.FIOHourly(self._fio)
			for hour in range(0, 24):
				time = self.get_current_time(hour)
				total_forecast += 'Hour' + str(hour+1) + "\n"
				curr = time + timedelta(hours=hour)
				total_forecast += str(curr) + "\n"
				for item in hourly.get_hour(hour).keys():
					total_forecast += item + ' : ' + str(hourly.get_hour(hour)[item]) + "\n"
				total_forecast += "\n"
				# Or access attributes directly for a given minute.
				# hourly.hour_5_time would also work
			return total_forecast
		else:
			return 'No Hourly data'
	"""
		A function to get the hourly forecast for the next 24 hours and determine
		if the probability of rain is great enough to cancel events

		@:param self, pass ourselves in
		@:param threshold, of type int determines how high the probability of rain
			should be per hour

		@:return a bool of true if the probability of rain is greater for 6 hours else false
	"""
	def check_rain_prob(self, threshold):
		sum = 0
		if self._fio.has_hourly() is True:
			hourly = FIOHourly.FIOHourly(self._fio)
			for hour in range(0, 24):
				#print(hourly.get_hour(hour)['precipProbability'])
				if hourly.get_hour(hour)['precipProbability'] >= threshold:
					sum += 1
			if sum >= 3:
				return True
			else:
				return False

	"""
		A function to get the hourly forecast for the next 24 hours and determine
		if the temperature is high enough that watering times should be increased

		@:param self, pass ourselves in
		@:param threshold, of type int determines how high the temperature should be to
			increase the watering time

		@:return a bool of true if the temperature is above the threshold else false
	"""
	def check_temp(self, threshold):
		temp = False
		if self._fio.has_hourly() is True:
			hourly = FIOHourly.FIOHourly(self._fio)
			for hour in range(0, 24):
				if hourly.get_hour(hour)['temperature'] > threshold:
					temp = True
			if temp:
				return True
			else:
				return False

	"""
		A function to get the whole hourly forecast for the next 24 hours

		@:param self, pass ourselves in

		@:return a string of the forecast
	"""
	def get_total_24_rain_forecast(self):
		total = 0
		if self._fio.has_hourly() is True:
			hourly = FIOHourly.FIOHourly(self._fio)
			for hour in range(0, 24):
				if hourly.get_hour(hour)['precipProbability'] > 0:
					total += hourly.get_hour(hour)['precipIntensity']
			return total
		else:
			print('No Hourly data')
			return total

	"""
		A getter function to get the rain intensity for the given hour

		@:param self, pass ourselves in
		@:param hour, of type int to determine which hour to get the intensity for

		@:return the value of the intensity
	"""
	def get_intensity_for_hour(self, hour):
		if self._fio.has_hourly():
			hourly = FIOHourly.FIOHourly(self._fio)
			return hourly.get_hour(hour)['precipIntensity']

	"""
		A getter function to get the rain probability for the given hour

		@:param self, pass ourselves in
		@:param hour, of type int to determine which hour to get the intensity for

		@:return the value of the intensity
	"""
	def get_probability_for_hour(self, hour):
		if self._fio.has_hourly():
			hourly = FIOHourly.FIOHourly(self._fio)
			return hourly.get_hour(hour)['precipProbability']

	"""
		A getter function to get the current time plus the hour

		@:param self, pass ourselves in
		@:param hour, of type int to add to the current time

		@:return the time plus the hour
	"""
	def get_current_time(self, hour):
		eastern = timezone('US/Eastern')
		loc_dt = eastern.localize(datetime.now())
		return (loc_dt + timedelta(hours=hour))

	"""
		A getter function to convert the day number to day string

		@:param self, pass ourselves in

		@:return the string of the day given by the day number
	"""
	def get_current_day(self):
		eastern = timezone('US/Eastern')
		loc_dt = eastern.localize(datetime.now())
		it = datetime.date(loc_dt).isoweekday()
		if it == 1:
			return "Monday"
		elif it == 2:
			return "Tuesday"
		elif it == 3:
			return "Wednesday"
		elif it == 4:
			return "Thursday"
		elif it == 5:
			return "Friday"
		elif it == 6:
			return "Saturday"
		elif it == 7:
			return "Sunday"

	def get_max_temp(self):
		if self._fio.has_daily():
			daily = FIODaily.FIODaily(self._fio)
			return daily.get_day(0)["temperatureMax"]

	def check_for_freeze(self):
		if self._fio.has_daily():
			daily = FIODaily.FIODaily(self._fio)
			for x in range(0, 14):
				try:
					tempMin = daily.get_day(x)["temperatureMin"]
					if float(tempMin) <= 32:
						fmt = "%m-%d-%Y"
						now_time = datetime.now(timezone('US/Eastern')) + timedelta(days=x)
						time = now_time.strftime(fmt)# + datetime.timedelta(days=x)
						#print(time)
						TEXT = "Hello GardeNet User,\n\nThe forecast for " + time + " shows that the temperature" \
								" may reach " + str(tempMin) + " degrees. We suggest preparing your system for the " \
							   "freeze. \n\nRegards,\nGardeNet"
						Alert(TEXT)
						break
				except:
					pass

if __name__ == "__main__":
	forecast = Forecast()
	#days_rainfall = forecast.get_total_24_rain_forecast()
	#forecast_string = forecast.get_forecast()
	#print(days_rainfall)
	#print(forecast_string)
	rain = forecast.check_rain_prob(.6)
	# temp = forecast.check_temp(92)
	# day = forecast.get_current_day()
	# print(rain)
	# print(temp)
	# print(day)
	# print(rain)
	# print(forecast.get_max_temp())
	forecast.check_for_freeze()
