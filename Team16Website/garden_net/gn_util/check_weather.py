import socket

from weather_forecast import Forecast
from database import Database
from JSON import JSON_Interface

forecast = Forecast()
#db = Database(False)
ipc_file = "ipc_file.txt"
rain_threshold = .80
temp_threshold = 10
json_convert = JSON_Interface()

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5538


#########TESTING
db = Database(True)
from event import Event
from zone import Zone
z = Zone(1)
z2 = Zone(2)
z3 = Zone(3)

db.add_zone(z)
db.add_zone(z2)

test_event0 = Event(3.0, 4.0, 'Tuesday', 1, 1)
test_event1 = Event(1.0, 2.0, 'Tuesday', 1, 1)
test_event2 = Event(2.0, 3.0, 'Tuesday', 1, 1)
test_event3 = Event(1.5, 2.5, 'Tuesday', 1, 1)
test_event4 = Event(1.0, 2.0, 'Tuesday', 1, 2)
test_event5 = Event(1.0, 2.0, 'Tuesday', 1, 3)
test_event6 = Event(1.5, 3.0, 'Tuesday', 1, 3)
test_event7 = Event(1.0, 2.0, 'Tuesday', 1, 3)

test_event8 = Event(3.0, 4.0, 'Tuesday', 2, 1)
test_event9 = Event(1.0, 2.0, 'Tuesday', 2, 1)
test_event10 = Event(2.0, 3.0, 'Tuesday', 2, 1)
test_event11 = Event(1.5, 2.5, 'Tuesday', 2, 1)
test_event12 = Event(1.0, 2.0, 'Tuesday', 2, 2)
test_event13 = Event(1.0, 2.0, 'Tuesday', 2, 3)
test_event14 = Event(1.5, 3.0, 'Tuesday', 2, 3)
test_event15 = Event(1.0, 2.0, 'Tuesday', 2, 3)

db.add_event(test_event0)
db.add_event(test_event1)
db.add_event(test_event2)
db.add_event(test_event3)
db.add_event(test_event4)
db.add_event(test_event5)
db.add_event(test_event6)
db.add_event(test_event7)
db.add_event(test_event8)
db.add_event(test_event9)
db.add_event(test_event10)
db.add_event(test_event11)
db.add_event(test_event12)
db.add_event(test_event13)
db.add_event(test_event14)
db.add_event(test_event15)

#########testing




if forecast.check_rain_prob(rain_threshold):
	f = open(ipc_file, 'w')
	f.write("NoEvents")
	f.close()
	try:
		soc = socket.create_connection(('localhost', port))
	except:
		print("Unable to connect")
elif forecast.check_temp(temp_threshold):
	day = forecast.get_current_day()
	zone = 1
	it = "{"
	while True:
		try:
			event_list = db.get_events_on_day_for_zone(day, zone)
			zone_obj = Zone(zone)
			for event in event_list:
				current_hour = int(event.stop_time)
				current_minute = int((event.stop_time - current_hour) * 100)
				if (int(current_minute) + 15) == 60:
					new_hour = int(current_hour) + 1
					new_minute = 0
					temp = str(new_hour) + '.' + str(new_minute)
					new_time = float(temp)
					event.set_stop_time(new_time)
				elif (int(current_minute) + 15) > 60:
					new_hour = current_hour + 1
					new_minute = (current_minute + 15) - 60
					if new_minute < 10:
						temp = str(new_hour) + ".0" + str(new_minute)
						new_time = float(temp)
						event.set_stop_time(new_time)
					else:
						temp = str(new_hour) + "." + str(new_minute)
						new_time = float(temp)
						event.set_stop_time(new_time)
				else:
					new_minute = current_minute + 15
					temp = str(current_hour) + "." + str(new_minute)
					new_time = float(temp)
					event.set_stop_time(new_time)
			temp_zone = Zone(zone+1)
			try:
				db.get_events_on_day_for_zone(day, zone+1)
				it += json_convert.to_JSON(event_list, zone_obj, True)
			except:
				it += json_convert.to_JSON(event_list, zone_obj, False)
		except:
			break
		zone += 1
	it += "}"
	print(it)