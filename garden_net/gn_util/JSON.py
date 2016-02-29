import json

from garden_net.gn_util.event import Event
from garden_net.gn_util.zone import Zone
from garden_net.database.database import Database

class JSON_Interface:
	def __init__(self):
		pass

	def convert_to_JSON(self, event: Event, zone: Zone):
		return "{" + str(zone.to_JSON()) + "[\n\t" + event.to_JSON() + "\n]}"

	def to_JSON(self, event_list: list, zone: Zone):
		temp = "{" + str(zone.to_JSON()) + ""
		i = 0
		for event in event_list:
			if i == 0:
				temp += "\n\t{\"event" + str(i) + "\":"
			else:
				temp += "\n\t\"event" + str(i) + "\":"
			temp += "\n\t\t" + event.to_JSON()
			if (i == (len(event_list)-1)):
				temp += "\n\t}"
			else:
				temp += "\n\t,"
			i+=1
		temp += "\n}"

		return temp

	def from_JSON(self, file_name):
		f = open(file_name, 'r')
		temp = ""
		for line in f.readlines():
			temp += line

		return temp

	def create_events_from_JSON_string(self, json_str: str, db: Database):
		# temp = json_str[2:7]
		# print(temp)
		i = 1
		j = 0
		parsed = json.loads(json_str)
		temp = True
		while (temp):
			zone_string = "zone" + str(i)
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
						event = Event(start_time, stop_time, day, zone_id)
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