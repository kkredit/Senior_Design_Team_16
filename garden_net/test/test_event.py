import unittest

from garden_net.gn_util.event import Event

class TestEvent(unittest.TestCase):
	def setUp(self):
		pass

	def test_create_event(self):
		e = Event(1.0, 2.0, 'Monday')
		self.assertEqual(e.start_time, 1.0)
		self.assertEqual(e.stop_time, 2.0)
		self.assertEqual(e.day, 'Monday')

	def test_create_event_bad_initial_start_time(self):
		with self.assertRaises(ValueError):
			e = Event(3.0, 2.0, 'Sunday')
		with self.assertRaises(ValueError):
			r = Event(24.0, 3.0, 'Sunday')

	def test_create_event_bad_changed_start_time(self):
		e = Event(1.0, 2.0, 'Tuesday')
		with self.assertRaises(ValueError):
			e.start_time = 3.0
		with self.assertRaises(ValueError):
			e.start_time = 24.0

	def test_create_event_bad_initial_end_time(self):
		with self.assertRaises(ValueError):
			e = Event(2.0,1.0, 'Wednesday')
		with self.assertRaises(ValueError):
			r = Event(4.0,24.00, 'Thursday')

	def test_create_event_with_bad_changed_end_time(self):
		e = Event(2.0, 3.0, 'Friday')
		with self.assertRaises(ValueError):
			e.stop_time = 1.0
		with self.assertRaises(ValueError):
			e.stop_time = 200.00

	def tearDown(self):
		pass

if __name__ == "__main__":
	unittest.main()