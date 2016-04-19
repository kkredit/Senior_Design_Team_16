from forecastiopy import ForecastIO, FIOHourly, FIODaily
from datetime import datetime, timedelta
from pytz import timezone
import pytz

class Forecast:
	def __init__(self):
		self._apikey = 'f119a2dd4a7d21069aebce2924af6f9a'
		# CCG location coordinates: 42.8057347, -85.5263732
		self._my_location = [42.8057347, -85.5263732]

		self._fio = ForecastIO.ForecastIO(self._apikey, latitude=self._my_location[0], longitude=self._my_location[1])

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

	def check_rain_prob(self, threshold):
		sum = 0
		if self._fio.has_hourly() is True:
			hourly = FIOHourly.FIOHourly(self._fio)
			for hour in range(0, 24):
				if hourly.get_hour(hour)['precipProbability'] > threshold:
					sum += 1
			if sum >= 6:
				return True
			else:
				return False

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

	def get_intensity_for_hour(self, hour):
		if self._fio.has_hourly():
			hourly = FIOHourly.FIOHourly(self._fio)
			return hourly.get_hour(hour)['precipIntensity']

	def get_probability_for_hour(self, hour):
		if self._fio.has_hourly():
			hourly = FIOHourly.FIOHourly(self._fio)
			return hourly.get_hour(hour)['precipProbability']

	def get_current_time(self, hour):
		eastern = timezone('US/Eastern')
		loc_dt = eastern.localize(datetime.now())
		return (loc_dt + timedelta(hours=hour))

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

if __name__ == "__main__":
	forecast = Forecast()
	#days_rainfall = forecast.get_total_24_rain_forecast()
	#forecast_string = forecast.get_forecast()
	#print(days_rainfall)
	#print(forecast_string)
	rain = forecast.check_rain_prob(.80)
	temp = forecast.check_temp(92)
	day = forecast.get_current_day()
	print(rain)
	print(temp)
	print(day)
