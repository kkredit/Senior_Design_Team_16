from garden_net.gn_util.event import Event
from garden_net.gn_util.zone import Zone

class JSON_Interface:
	def __init__(self):
		pass

	def convert_to_JSON(self, event: Event, zone: Zone):
		return "{" + zone.to_JSON() + "[\n\t" + event.to_JSON() + "\n]}"

if __name__ == "__main__":
	e = Event(1.0, 2.0, 'Monday', 1)
	z = Zone(1)
	json = JSON_Interface()

	json_string = json.convert_to_JSON(e, z)
	print(json_string)