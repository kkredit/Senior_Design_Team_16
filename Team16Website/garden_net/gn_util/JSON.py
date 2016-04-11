import json

from event import Event
from zone import Zone
from database import Database

class JSON_Interface:
	def __init__(self):
		pass

	def convert_to_JSON(self, event: Event, zone: Zone):
		return "{" + str(zone.to_JSON()) + "[\n\t" + event.to_JSON() + "\n]}"

	def to_JSON(self, event_list: list, zone: Zone, comma: bool):
		temp = str(zone.to_JSON()) + ""
		i = 0
		for event in event_list:
			if i == 0:
				temp += "\n\t{\"event" + str(i) + "\":"
			else:
				temp += "\n\t\"event" + str(i) + "\":"
			temp += "\n\t\t" + event.to_JSON()
			if (i == (len(event_list)-1)):
				temp += "\n\t}"
				if comma:
					temp += ","
			else:
				temp += ","
			i+=1
		temp += "\n"

		return temp

	def from_JSON(self, file_name):
		f = open(file_name, 'r')
		temp = ""
		for line in f.readlines():
			temp += line
		f.close()
		return temp

	def create_events_from_JSON_string(self, json_str: str, db: Database):
		# temp = json_str[2:7]
		# print(temp)
		i = 1
		j = 0
		parsed = json.loads(json_str)
		temp = True
		while (temp):
			zone_string = "Zone" + str(i)
			try:
				parsed[zone_string]
				# Anything after this line will not execute if a KeyError is raised
				zone = Zone(i)
				db.add_zone(zone)
				temp2 = True
				while (temp2):
					event_string = "event" + str(j)
					try:
						day = parsed[zone_string][event_string]['day']
						zone_id = parsed[zone_string][event_string]['zone_ID']
						temp_start = parsed[zone_string][event_string]['start_time']
						start_time = float(temp_start.replace(":", "."))
						temp_stop = parsed[zone_string][event_string]['stop_time']
						stop_time = float(temp_stop.replace(":", "."))
						if (zone_id != i):
							zone_id = i
						valve = parsed[zone_string][event_string]['valve_num']
						event = Event(start_time, stop_time, day, zone_id, valve)
						#print(start_time)
						#print(parsed[zone_string][event_string])
						#print(event)
						db.add_event(event)
						j += 1
					except(KeyError):
						temp2 = False
						j = 0
			except(KeyError):
				temp = False

			i += 1

	def all_events_from_DB_to_JSON(self, db:Database):
		i = 1
		temp = True
		string = "{"
		while(temp):
			events= []
			try:
				try:
					events = db.get_events_on_day_for_zone('Sunday', i)
				except ValueError:
					pass
				try:
					events += db.get_events_on_day_for_zone('Monday', i)
				except ValueError:
					pass
				try:
					events += db.get_events_on_day_for_zone('Tuesday', i)
				except ValueError:
					pass
				try:
					events += db.get_events_on_day_for_zone('Wednesday', i)
				except ValueError:
					pass
				try:
					events += db.get_events_on_day_for_zone('Thursday', i)
				except ValueError:
					pass
				try:
					events += db.get_events_on_day_for_zone('Friday', i)
				except ValueError:
					pass
				try:
					events += db.get_events_on_day_for_zone('Saturday', i)
				except ValueError:
					pass
				try:
					db.get_all_events_for_zone(i+1)
					string += self.to_JSON(events, Zone(i), True)
				except:
					string += self.to_JSON(events, Zone(i), False)
			except KeyError:
				temp = False
			i += 1
		string += "}"
		return string



if __name__ == "__main__":
	# e = Event(1.0, 2.0, 'Monday', 1)
	# e2 = Event(1.0, 2.0, 'Monday', 1)
	# list_e = [e, e2]
	# z = Zone(1)
	# json = JSON_Interface()
	#
	# json_string = json.to_JSON(list_e, z)
	# print(json_string)

	myjson = JSON_Interface()
	name = 'input_JSON.txt'

	string = myjson.from_JSON(name)
	parsed = json.loads(string)

	print(parsed['zone3']['event0']['day'])
