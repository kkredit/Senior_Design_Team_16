from weather_forecast import Forecast
from hour import OneHour
from zone import Zone
from database import Database
from event import Event

FORECAST_LIST = []
forecast = Forecast()
precip_probability_threshold = .4


#create a list of one hour events

for i in range(0,24):
	current_foecast = OneHour()
	time = forecast.get_current_time(i)
	current_foecast.set_current_time(time)
	prob = forecast.get_probability_for_hour(i)
	current_foecast.set_precipProbability(prob)
	intense = forecast.get_intensity_for_hour(i)
	current_foecast.set_precipIntensity(intense)
	FORECAST_LIST.append(current_foecast)

# for hour in FORECAST_LIST:
# 	print(hour)

it = FORECAST_LIST[0]

print(it.get_current_hour())

if __name__ == "__main__":
	db = Database(True)

	z = Zone(1)
	z2 = Zone(2)
	z3 = Zone(3)

	db.add_zone(z)
	db.add_zone(z2)
	db.add_zone(z3)

	test_event0 = Event(9.0, 12.0, 'Thursday', 3)
	test_event1 = Event(10.0, 11.0, 'Thursday', 1)
	test_event2 = Event(9.0, 12.0, 'Thursday', 2)
	test_event3 = Event(13.3, 15.30, 'Thursday', 3)
	test_event4 = Event(13.0, 18.0, 'Thursday', 2)
	test_event5 = Event(1.0, 2.0, 'Tuesday', 1)
	test_event6 = Event(1.0, 3.0, 'Tuesday', 3)
	test_event7 = Event(1.0, 2.0, 'Saturday', 1)

	db.add_event(test_event0)
	db.add_event(test_event1)
	db.add_event(test_event2)
	db.add_event(test_event3)
	db.add_event(test_event4)
	db.add_event(test_event5)
	db.add_event(test_event6)
	db.add_event(test_event7)

	print(forecast.get_current_day())

	events = db.get_all_events_on_day(forecast.get_current_day())

	for hour in range(0,24):
		if FORECAST_LIST[i] > precip_probability_threshold:



	# for item in events:
	#	print(item)
