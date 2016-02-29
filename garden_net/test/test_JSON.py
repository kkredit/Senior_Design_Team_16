import unittest

from garden_net.database.database import Database
from garden_net.gn_util.JSON import JSON_Interface
from garden_net.gn_util.event import Event
from garden_net.gn_util.zone import Zone

class TestJSON(unittest.TestCase):
	def setUp(self):
		self.db = Database()

	def test_add_entry_from_JSON(self):
		json = JSON_Interface()
		string = json.from_JSON('test_input.txt')
		json.create_events_from_JSON_string(string, self.db)

		events_on_day = self.db.get_all_events_for_zone(1)

		self.assertEqual(events_on_day[0].start_time, 1.0)
		self.assertEqual(events_on_day[0].stop_time, 2.0)
		self.assertEqual(events_on_day[0].day, 'Monday')
		self.assertEqual(events_on_day[0].owner, 1)
		self.assertEqual(events_on_day[1].start_time, 1.5)
		self.assertEqual(events_on_day[1].stop_time, 2.5)
		self.assertEqual(events_on_day[1].day, 'Monday')
		self.assertEqual(events_on_day[1].owner, 1)
		self.assertEqual(events_on_day[2].start_time, 2.0)
		self.assertEqual(events_on_day[2].stop_time, 3.0)
		self.assertEqual(events_on_day[2].day, 'Monday')
		self.assertEqual(events_on_day[2].owner, 1)
		self.assertEqual(events_on_day[3].start_time, 3.0)
		self.assertEqual(events_on_day[3].stop_time, 4.0)
		self.assertEqual(events_on_day[3].day, 'Monday')
		self.assertEqual(events_on_day[3].owner, 1)

		events_on_day = self.db.get_all_events_for_zone(2)

		self.assertEqual(events_on_day[0].start_time, 1.0)
		self.assertEqual(events_on_day[0].stop_time, 2.0)
		self.assertEqual(events_on_day[0].day, 'Monday')
		self.assertEqual(events_on_day[0].owner, 2)
		self.assertEqual(events_on_day[1].start_time, 1.5)
		self.assertEqual(events_on_day[1].stop_time, 2.5)
		self.assertEqual(events_on_day[1].day, 'Monday')
		self.assertEqual(events_on_day[1].owner, 2)
		self.assertEqual(events_on_day[2].start_time, 2.0)
		self.assertEqual(events_on_day[2].stop_time, 3.0)
		self.assertEqual(events_on_day[2].day, 'Monday')
		self.assertEqual(events_on_day[2].owner, 2)
		self.assertEqual(events_on_day[3].start_time, 3.0)
		self.assertEqual(events_on_day[3].stop_time, 4.0)
		self.assertEqual(events_on_day[3].day, 'Monday')
		self.assertEqual(events_on_day[3].owner, 2)

		events_on_day = self.db.get_all_events_for_zone(3)

		self.assertEqual(events_on_day[0].start_time, 1.0)
		self.assertEqual(events_on_day[0].stop_time, 2.0)
		self.assertEqual(events_on_day[0].day, 'Monday')
		self.assertEqual(events_on_day[0].owner, 3)
		self.assertEqual(events_on_day[1].start_time, 1.5)
		self.assertEqual(events_on_day[1].stop_time, 2.5)
		self.assertEqual(events_on_day[1].day, 'Monday')
		self.assertEqual(events_on_day[1].owner, 3)
		self.assertEqual(events_on_day[2].start_time, 2.0)
		self.assertEqual(events_on_day[2].stop_time, 3.0)
		self.assertEqual(events_on_day[2].day, 'Monday')
		self.assertEqual(events_on_day[2].owner, 3)
		self.assertEqual(events_on_day[3].start_time, 3.0)
		self.assertEqual(events_on_day[3].stop_time, 4.0)
		self.assertEqual(events_on_day[3].day, 'Monday')
		self.assertEqual(events_on_day[3].owner, 3)

	def test_all_JSON_functionality(self):
		event_list = []
		event_list.append(Event(3.0, 4.0, 'Monday', 1))
		event_list.append(Event(1.0, 2.0, 'Monday', 1))
		event_list.append(Event(2.0, 3.0, 'Monday', 1))
		event_list.append(Event(1.5, 2.5, 'Monday', 1))
		event_list.append(Event(22.0, 23.59, 'Monday', 1))
		event_list.append(Event(1.0, 2.0, 'Tuesday', 1))
		event_list.append(Event(1.0, 3.0, 'Tuesday', 1))
		event_list.append(Event(1.0, 2.0, 'Saturday', 1))

		event_list.append(Event(3.0, 4.0, 'Monday', 2))
		event_list.append(Event(1.0, 2.0, 'Monday', 2))
		event_list.append(Event(2.0, 3.0, 'Monday', 2))
		event_list.append(Event(1.5, 2.5, 'Monday', 2))
		event_list.append(Event(22.0, 23.59, 'Monday', 2))
		event_list.append(Event(1.0, 2.0, 'Tuesday', 2))
		event_list.append(Event(1.0, 3.0, 'Tuesday', 2))
		event_list.append(Event(1.0, 2.0, 'Saturday', 2))

		event_list.append(Event(3.0, 4.0, 'Monday', 3))
		event_list.append(Event(1.0, 2.0, 'Monday', 3))
		event_list.append(Event(2.0, 3.0, 'Monday', 3))
		event_list.append(Event(1.5, 2.5, 'Monday', 3))
		event_list.append(Event(22.0, 23.59, 'Monday', 3))
		event_list.append(Event(1.0, 2.0, 'Tuesday', 3))
		event_list.append(Event(1.0, 3.0, 'Tuesday', 3))
		event_list.append(Event(1.0, 2.0, 'Saturday', 3))

		# print(event_list)
		#
		# convert = JSON_Interface()
		#
		# converted = convert.to_JSON(event_list, 3)
		# print(converted)

if __name__ == "__main__":
	unittest.main()