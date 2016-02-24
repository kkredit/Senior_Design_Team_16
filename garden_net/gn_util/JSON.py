from garden_net.gn_util.event import Event
from garden_net.gn_util.zone import Zone

class JSON_Interface:
	def __init__(self):
		pass

	def convert_to_JSON(self, event: Event, zone: Zone):
		return "{" + str(zone.to_JSON()) + "[\n\t" + event.to_JSON() + "\n]}"

	def to_JSON(self, event_list: list, zone: Zone):
		temp = "{" + zone.to_JSON()
		i = 0
		for event in event_list:
			if (i == (len(event_list)-1)):
				temp += "\n\t" + event.to_JSON()
			else:
				temp += "\n\t" + event.to_JSON() + ","
			i+=1
		temp += "\n]}"

		return temp

if __name__ == "__main__":
	e = Event(1.0, 2.0, 'Monday', 1)
	e2 = Event(1.0, 2.0, 'Monday', 1)
	list_e = [e, e2]
	z = Zone(1)
	json = JSON_Interface()

	json_string = json.to_JSON(list_e, z)
	print(json_string)