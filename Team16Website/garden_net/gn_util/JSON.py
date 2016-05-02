"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	JSON.py:

	This script is used to encode, decode, and interact with the database
	in order to write and save data that is being shared about scheduling
	events that are sent from the website.

"""

# Third Party Imports
import json

# Local Imports
from event import Event
from zone import Zone
from database import Database

class JSON_Interface:
	"""
		A Constructor for the JSON class
	"""
	def __init__(self):
		pass
	"""
		A wraper function that calls on both the Event and Zone class's JSON
		methods in order to write out a fully combined JSON string

		@:param event, Event object to be written in JSON form
		@:param zone, Zone object to be written in JSON form

		@:return a string that has an event and zone in JSON form
	"""
	def convert_to_JSON(self, event: Event, zone: Zone):
		return "{" + str(zone.to_JSON()) + "[\n\t" + event.to_JSON() + "\n]}"

	"""
		A wraper function that calls on both the Event and Zone class's JSON
		methods in order to write out a fully combined JSON string given an
		event list.

		@:param event_list, list of event objects to be written in JSON form
		@:param zone, Zone object to be written in JSON form
		@:param comma, bool determines whether or not a comma needs to be placed
						at the end of the line

		@:return a string that has an event and zone in JSON form
	"""
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
	"""
		A function that reads a JSON file and returns it's contents

		@:param file_name to be read from

		@:return a string of the files contents
	"""
	def from_JSON(self, file_name):
		f = open(file_name, 'r')
		temp = ""
		for line in f.readlines():
			temp += line
		f.close()
		return temp

	"""
		A function that creates a list of events given a JSON string
		and then save it in the database. It uses try catch blocks in
		order to dynamically assign the events.

		@:param json_str, str of json
		@:param db, Database is the database in which to save the events
	"""
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
						valve = int(parsed[zone_string][event_string]['valve_num'])
						event = Event(start_time, stop_time, day, zone_id, valve)
						#print(start_time)
						#print(parsed[zone_string][event_string])
						#print(event)
						try:
							if str(event.start_time) != "0.0":
								print("Adding: " + str(event))
								db.add_event(event)
						except Exception as e:
							print(e)
							pass
						j += 1
					except(KeyError):
						temp2 = False
						j = 0
			except(KeyError):
				temp = False

			i += 1
	"""
		A function that takes all of the events from the database,
		and writes them to a JSON string

		@:param db, Database is the database in which to get the events from

		@:return a JSON formatted string of all of the events in the db
	"""
	def all_events_from_DB_to_JSON(self, db:Database):
		i = 1
		temp = True
		string = "{"
		while(temp):
			events= []
			try:
				try:
					events += db.get_events_on_day_for_zone('Sunday', i)
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
					events += db.get_events_on_day_for_zone('Everyday', i)
				except ValueError:
					pass
				try:
					db.get_all_events_for_zone(i+1)
					for item in events:
						print(item)
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

	# myjson = JSON_Interface()
	# name = 'input_JSON.txt'
	#
	# string = myjson.from_JSON(name)
	# parsed = json.loads(string)
	#
	# print(parsed['zone3']['event0']['day'])

	db = Database(False)
	json_conversion = JSON_Interface()
	ipc_file = "ipc_file.txt"
	f = open(ipc_file, 'r')
	file_data = ""
	for line in f:
		file_data += line

	db.clear_database()
	json_conversion.create_events_from_JSON_string(file_data, db)

	convert = JSON_Interface()
	converted = convert.all_events_from_DB_to_JSON(db)
	#print(converted)


