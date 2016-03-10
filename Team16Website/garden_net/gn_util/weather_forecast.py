from forecastiopy import ForecastIO, FIODaily
from datetime import datetime

class Forecast:
	def __init__(self):
		self._apikey = 'f119a2dd4a7d21069aebce2924af6f9a'
		# CCG location coordinates: 42.8057347, -85.5263732
		self._my_location = [42.8057347, -85.5263732]

		self._fio = ForecastIO.ForecastIO(self._apikey, latitude=self._my_location[0], longitude=self._my_location[1])

	def get_forecast(self):
		if self._fio.has_daily() is True:
			daily = FIODaily.FIODaily(self._fio)
			#print('Daily')
			#print('Summary:', daily.summary)
			#print('Icon:', daily.icon)
			print()
			for day in range(2, 9):
				print('Day', day)
				rise_time = daily.get_day(day)['sunriseTime']
				sunriseTime = datetime.datetime.fromtimestamp(int(rise_time)).strftime('%m-%d-%Y %H:%M:%S')
				print("sunriseTime: " + sunriseTime)
				down_time = daily.get_day(day)['sunsetTime']
				sunsetTime = datetime.datetime.fromtimestamp(int(down_time)).strftime('%H:%M:%S')
				print("sunsetTime: " + sunsetTime)
				print("humidity: " + str(daily.get_day(day)['humidity']))
				print("temperatureMax: " + str(daily.get_day(day)['temperatureMax']))
				print("temperatureMin: " + str(daily.get_day(day)['temperatureMin']))
				print("cloudCover: " + str(daily.get_day(day)['cloudCover']))
				print("precipType: " + str(daily.get_day(day)['precipType']))
				print("precipProbability: " + str(daily.get_day(day)['precipProbability']))
				# for item in daily.get_day(day).keys():
				# 	print(item + ' : ' + str(daily.get_day(day)[item]))
				print
				# Or access attributes directly for a given minute.
				# daily.day_7_time would also work
				#print(daily.day_5_time)
				print()
				print()
		else:
			print('No Daily data')

if __name__ == "__main__":
	forecast = Forecast()
	forecast.get_forecast()