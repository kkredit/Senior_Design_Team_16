import unittest

from garden_net.database.database import Database
from garden_net.gn_util.zone import Zone
from garden_net.gn_util.event import Event
from garden_net.gn_util.JSON import JSON_Interface

class TestDataBase(unittest.TestCase):
	def setUp(self):
		self.db = Database()

	def test_add_zone(self):
		z = Zone(1)
		self.db.add_zone(z)
		z2 = Zone(2)
		self.db.add_zone(z2)
		z3 = Zone(3)
		self.db.add_zone(z3)
		z4 = Zone(1)
		with self.assertRaises(ValueError):
			self.db.add_zone(z4)

	def test_add_event(self):
		z = Zone(1)
		self.db.add_zone(z)
		test_event = Event(1.0, 2.0, 'Monday', 3)
		with self.assertRaises(ValueError):
			self.db.add_event(test_event)

		test_event2 = Event(1.0, 2.0, 'Monday', 1)
		self.db.add_event(test_event2)
		test_event3 = Event(1.0, 2.0, 'Monday', 1)
		with self.assertRaises(ValueError):
			self.db.add_event(test_event3)

		test_event4 = Event(1.0, 2.0, 'Tuesday', 1)
		self.db.add_event(test_event4)

	def test_getting_all_events_for_day(self):
		with self.assertRaises(ValueError):
			self.db.get_all_events_on_day('Monday')

		z = Zone(1)
		z2 = Zone(2)
		z3 = Zone(3)

		self.db.add_zone(z)
		self.db.add_zone(z2)
		self.db.add_zone(z3)

		test_event0 = Event(3.0, 4.0, 'Monday', 3)
		test_event1 = Event(1.0, 2.0, 'Monday', 3)
		test_event2 = Event(2.0, 3.0, 'Monday', 3)
		test_event3 = Event(1.5, 2.5, 'Monday', 3)
		test_event4 = Event(22.0, 23.59, 'Monday', 3)
		test_event5 = Event(1.0, 2.0, 'Tuesday', 1)
		test_event6 = Event(1.0, 3.0, 'Tuesday', 3)
		test_event7 = Event(1.0, 2.0, 'Saturday', 1)

		self.db.add_event(test_event0)
		self.db.add_event(test_event1)
		self.db.add_event(test_event2)
		self.db.add_event(test_event3)
		self.db.add_event(test_event4)
		self.db.add_event(test_event5)
		self.db.add_event(test_event6)
		self.db.add_event(test_event7)

		with self.assertRaises(ValueError):
			self.db.get_all_events_on_day('Friday')

		results = self.db.get_all_events_on_day('Monday')
		# for event in results:
		# 	print(event)

	def test_get_events_for_a_certain_zone(self):
		z = Zone(3)

		self.db.add_zone(z)

		test_event0 = Event(3.0, 4.0, 'Monday', 3)
		test_event1 = Event(1.0, 2.0, 'Monday', 3)
		test_event2 = Event(2.0, 3.0, 'Monday', 3)
		test_event3 = Event(1.5, 2.5, 'Monday', 3)
		test_event4 = Event(1.45, 2.0, 'Monday', 3)
		test_event5 = Event(1.0, 2.0, 'Tuesday', 3)
		test_event6 = Event(1.0, 3.0, 'Tuesday', 3)
		test_event7 = Event(1.0, 2.0, 'Saturday', 3)

		self.db.add_event(test_event0)
		self.db.add_event(test_event1)
		self.db.add_event(test_event2)
		self.db.add_event(test_event3)
		self.db.add_event(test_event4)
		self.db.add_event(test_event5)
		self.db.add_event(test_event6)
		self.db.add_event(test_event7)

		with self.assertRaises(ValueError):
			self.db.get_all_events_for_zone(1)

		with self.assertRaises(ValueError):
			self.db.get_all_events_for_zone(2)

		results = self.db.get_all_events_for_zone(3)
		# for event in results:
		# 	print(event)

	def test_get_events_for_a_certain_zone_and_day(self):
		with self.assertRaises(ValueError):
			self.db.get_all_events_on_day('Monday')

		with self.assertRaises(ValueError):
			self.db.get_all_events_on_day('Sunday')

		z = Zone(1)
		z2 = Zone(2)
		z3 = Zone(3)

		self.db.add_zone(z)
		self.db.add_zone(z2)
		self.db.add_zone(z3)

		test_event0 = Event(3.0, 4.0, 'Monday', 3)
		test_event1 = Event(1.0, 2.0, 'Monday', 3)
		test_event2 = Event(2.0, 3.0, 'Monday', 3)
		test_event3 = Event(1.5, 2.5, 'Monday', 3)
		test_event4 = Event(1.0, 2.0, 'Monday', 2)
		test_event5 = Event(1.0, 2.0, 'Tuesday', 1)
		test_event6 = Event(1.5, 3.0, 'Tuesday', 1)
		test_event7 = Event(1.0, 2.0, 'Saturday', 1)

		self.db.add_event(test_event0)
		self.db.add_event(test_event1)
		self.db.add_event(test_event2)
		self.db.add_event(test_event3)
		self.db.add_event(test_event4)
		self.db.add_event(test_event5)
		self.db.add_event(test_event6)
		self.db.add_event(test_event7)

		with self.assertRaises(ValueError):
			self.db.get_all_events_on_day('Friday')

		results0 = self.db.get_events_on_day_for_zone('Monday', 3)
		self.assertEqual(results0[0], test_event1)
		self.assertEqual(results0[1], test_event3)
		self.assertEqual(results0[2], test_event2)
		self.assertEqual(results0[3], test_event0)

		results1 = self.db.get_events_on_day_for_zone('Tuesday', 1)
		self.assertEqual(results1[0], test_event5)
		self.assertEqual(results1[1], test_event6)

		f = open('test_JSON.txt', 'w')
		toJSON = JSON_Interface()
		string = toJSON.to_JSON(results0, z3)
		print(string)
		f.write(string)
		f.close()

		results2 = self.db.get_events_on_day_for_zone('Saturday', 1)
		self.assertEqual(results2[0],test_event7)

		# for event in results0:
		# 	print(event)

		test_event8 = Event(17.0,20.0, 'Tuesday', 3)
		self.db.add_event(test_event8)

		results3 = self.db.get_events_on_day_for_zone('Tuesday',3)
		self.assertEqual(results3[0], test_event8)

if __name__ == "__main__":
	unittest.main()