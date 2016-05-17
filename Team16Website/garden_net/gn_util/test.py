#from historical_database import Historical_Database

#from historical_database_interface import Historical_Database_Interface
#from gateway_report import Report
#Report("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
# to = Historical_Database_Interface("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
# db = Historical_Database(True)
#
# db.add_row(it)
# db.add_row(to)
#
# the = db.get_all()
# for item in the:
# 	print(item)

# from database import Database
# from event import Event
# from zone import Zone
#
# db = Database(True)
#
# z = Zone(1)
# z2 = Zone(2)
# z3 = Zone(3)
#
# db.add_zone(z)
# db.add_zone(z2)
# db.add_zone(z3)
#
# test_event0 = Event(3.0, 4.0, 'Monday', 1, 1)
# test_event1 = Event(1.0, 2.0, 'Monday', 1, 1)
# test_event2 = Event(2.0, 3.0, 'Monday', 1, 1)
# test_event3 = Event(1.5, 2.5, 'Monday', 1, 1)
# test_event4 = Event(1.0, 2.0, 'Monday', 1, 2)
# test_event5 = Event(1.0, 2.0, 'Monday', 1, 3)
# test_event6 = Event(1.5, 3.0, 'Monday', 1, 3)
# test_event7 = Event(1.0, 2.0, 'Monday', 1, 3)
#
# db.add_event(test_event0)
# db.add_event(test_event1)
# db.add_event(test_event2)
# db.add_event(test_event3)
# db.add_event(test_event4)
# db.add_event(test_event5)
# db.add_event(test_event6)
# db.add_event(test_event7)
#
# it = db.get_events_on_day_for_zone_and_valve('Monday', 1, 1)
#
# for item in it:
# 	print(item)

""" Testing the alert engine """
"""
from gateway_report import Report
#it = Report("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
#print(it)
Report("01%2%5%0.00")
# Report("02")
# Report("03")
# Report("04%1%2")
"""

'''
user_info_file = "user_info.txt"
alerts_file = "alerts.txt"
user_info = []
f = open(user_info_file, 'r')
for line in f:
	user_info.append(line)
phone_number = str(user_info[1]).split(" ")[1].split("\n")[0]
print(phone_number)

f2 = open(alerts_file, 'r')
alerts = []
for line in f2:
	alerts.append(line)
opcode_01 = str(alerts[0]).split('\t')[1]
if opcode_01.upper() == "TRUE":
	opcode_01 = "1"
else:
	opcode_01 = "0"
opcode_02 = str(alerts[3]).split('\t')[1]
if opcode_02.upper() == "TRUE":
	opcode_02 = "1"
else:
	opcode_02 = "0"
opcode_03 = str(alerts[2]).split('\t')[1]
if opcode_03.upper() == "TRUE":
	opcode_03 = "1"
else:
	opcode_03 = "0"
opcode_04 = str(alerts[1]).split('\t')[1]
if opcode_04.upper() == "TRUE":
	opcode_04 = "1"
else:
	opcode_04 = "0"

send_string = phone_number + "%" + opcode_01 + "%" + opcode_02 + "%" + opcode_03 + "%" + opcode_04

print(send_string)
'''
# import json
# from JSON import JSON_Interface
# from event import Event
# EVENT_LIST = []
# def create_event_list(json_str: str):
# 	# temp = json_str[2:7]
# 	# print(temp)
# 	i = 1
# 	j = 0
# 	parsed = json.loads(json_str)
# 	temp = True
# 	while (temp):
# 		zone_string = "Zone" + str(i)
# 		try:
# 			parsed[zone_string]
# 			# Anything after this line will not execute if a KeyError is raised
# 			temp2 = True
# 			while (temp2):
# 				event_string = "event" + str(j)
# 				try:
# 					day = parsed[zone_string][event_string]['day']
# 					zone_id = parsed[zone_string][event_string]['zone_ID']
# 					temp_start = parsed[zone_string][event_string]['start_time']
# 					start_time = float(temp_start.replace(":", "."))
# 					temp_stop = parsed[zone_string][event_string]['stop_time']
# 					stop_time = float(temp_stop.replace(":", "."))
# 					valve = parsed[zone_string][event_string]['valve_num']
# 					event = Event(start_time, stop_time, day, zone_id, valve)
# 					EVENT_LIST.append(event)
#
# 					# print(start_time)
# 					# print(parsed[zone_string][event_string])
# 					# print(event)
# 					j += 1
# 				except(KeyError):
# 					temp2 = False
# 					j = 0
# 		except(KeyError):
# 			temp = False
#
# 		i += 1
# json_convert = JSON_Interface()
# demo2_file = open("demo2_file.txt", "r")
# string = ""
# for line in demo2_file:
# 	string += line
# print(string)
# events = create_event_list(string)
#
# for item in EVENT_LIST:
# 	print(item)

# import json
# f = open("garden_info.txt")
# json_string = ""
# for line in f:
# 	json_string += line
# converted = json.loads(json_string)
# print(converted["Zone1"]["weather"])


# demo_weather_report = open("demo_weather_reports.txt", "r")
# for line in demo_weather_report:
# 	print(line.split('\t')[1].upper())
# 	if line.split('\t')[0] == "rain":
# 		if line.split('\t')[1].upper().split('\n')[0] == "TRUE":
# 			rain = True
# 			print("rain")
# 	elif line.split('\t')[0] == "hot":
# 		if line.split('\t')[1].upper() == "TRUE":
# 			print("hot as balls")
# 			hot = True
# demo_weather_report.close()

import json

from database import Database
from event import Event
from zone import Zone
from weather_forecast import Forecast

db = Database(True)
forecast = Forecast()
z = Zone(1)
z2 = Zone(2)
z3 = Zone(3)

db.add_zone(z)
db.add_zone(z2)
db.add_zone(z3)

test_event0 = Event(3.0, 5.0, 'Everyday', 1, 1)
test_event1 = Event(1.0, 3.0, 'Everyday', 1, 1)
test_event2 = Event(2.0, 4.0, 'Everyday', 1, 1)
test_event3 = Event(1.5, 3.5, 'Everyday', 1, 1)
test_event4 = Event(1.0, 3.0, 'Everyday', 1, 2)
test_event5 = Event(1.0, 3.0, 'Everyday', 1, 3)
test_event6 = Event(1.5, 4.0, 'Everyday', 1, 3)
test_event7 = Event(1.0, 3.0, 'Everyday', 1, 3)

db.add_event(test_event0)
db.add_event(test_event1)
db.add_event(test_event2)
db.add_event(test_event3)
db.add_event(test_event4)
db.add_event(test_event5)
db.add_event(test_event6)
db.add_event(test_event7)

temp_threshold = 54

def increase_times(event: Event, increase_percentage: float, threshold: int, forecast: Forecast):
	# print(event)
	degrees_above = int(forecast.get_max_temp()) - threshold
	total_increase_percentage = degrees_above * increase_percentage
	#print("The total increase percentage: " + str(total_increase_percentage))
	total_watering_time = event.stop_time - event.start_time
	total_watering_time_hour = int(total_watering_time)
	total_watering_time_minute = int((total_watering_time - total_watering_time) * 100)
	total_watering_time_in_minutes = (total_watering_time_hour * 60) + total_watering_time_minute
	#print("total_watering_time: " + str(total_watering_time_in_minutes))
	increase_in_minutes = total_watering_time_in_minutes*total_increase_percentage
	#print(increase_in_minutes)
	#print("Total increase time: " + str(increase))
	increase_hour = int(increase_in_minutes/60)
	increase_minute = int(increase_in_minutes - increase_hour*60)
	#print("Increase hour: " + str(increase_hour) + " Increase minute: " + str(increase_minute))
	current_hour = int(event.stop_time)
	current_minute = int((event.stop_time - current_hour) * 100)
	if (int(current_minute) + increase_minute) == 60:
		new_hour = int(current_hour) + 1 + increase_hour
		new_minute = 0
		temp = str(new_hour) + '.' + str(new_minute)
		new_time = float(temp)
		event.set_stop_time(new_time)
	elif (int(current_minute) + increase_minute) > 60:
		new_hour = current_hour + 1 + increase_hour
		new_minute = (current_minute + increase_minute) - 60
		if new_minute < 10:
			temp = str(new_hour) + ".0" + str(new_minute)
			new_time = float(temp)
			event.set_stop_time(new_time)
		else:
			temp = str(new_hour) + "." + str(new_minute)
			new_time = float(temp)
			event.set_stop_time(new_time)
	else:
		new_hour = current_hour + increase_hour
		new_minute = current_minute + increase_minute
		temp = str(new_hour) + "." + str(new_minute)
		new_time = float(temp)
		event.set_stop_time(new_time)
	return event

day = forecast.get_current_day()
event_list = db.get_all_events_on_day(day)
# for event in event_list:
# 	print(event)
#weather_setting_file = open("/var/www/Team16Website/garden_net/gn_util/garden_info.txt", "r")
weather_setting_file = open("garden_info.txt", "r")
weather_settings = ""
for line in weather_setting_file:
	weather_settings += line
parsed = json.loads(weather_settings)
increase_amount = 0
for event in event_list:
	i = 1
	while True:
		zone_string = "Zone" + str(i)
		if int(parsed[zone_string]["node"]) == event.owner and \
					int(parsed[zone_string]["valve"]) == event.valve_num and \
					str(parsed[zone_string]["weather"]).upper() == "FULL":
			#print(str(event) + " Needs to FULLY increase water times")
			increase_times(event, .04, temp_threshold, forecast)
			break
		elif int(parsed[zone_string]["node"]) == event.owner and \
					int(parsed[zone_string]["valve"]) == event.valve_num and \
					str(parsed[zone_string]["weather"]).upper() == "PARTIAL":
			#print(str(event) + " Needs to PARTIALLY increase water times")
			increase_times(event, .02, temp_threshold, forecast)
			break
		elif int(parsed[zone_string]["node"]) == event.owner and \
					int(parsed[zone_string]["valve"]) == event.valve_num and \
					str(parsed[zone_string]["weather"]).upper() == "NONE":
			#print(str(event) + " does NOT need to increase")
			increase_amount = 0
			increase_times(event, 0, temp_threshold, forecast)
			break
		i += 1

#print(sensitivity_setting)