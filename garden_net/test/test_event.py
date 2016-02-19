import unittest
from garden_net.gn_util.event import Event

class TestEvent(unittest.TestCase):
	def setUp(self):
		pass

	def test_create_event(self):
		e = Event(1.0, 2.0)
		self.assertEqual(e.start_time, 1.0)
		self.assertEqual(e.stop_time, 2.0)

	def test_create_event_bad_initial_start_time(self):
		with self.assertRaises(ValueError):
			e = Event(3.0, 2.0)

		with self.assertRaises(ValueError):
			r = Event(10.0, 2.0)

	def test_create_event_bad_changed_start_time(self):
		e = Event(1.0, 2.0)

		with self.assertRaises(ValueError):
			e.start_time = 3.0

	def tearDown(self):
		pass

if __name__ == "__main__":
	unittest.main()