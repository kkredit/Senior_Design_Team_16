import unittest

from garden_net.gn_util.event import Event

class TestEvent(unittest.TestCase):
	def setUp(self):
		pass

	def test_create_event(self):
		e = Event(1.0, 2.0, 'Monday', 1)
		self.assertEqual(e.start_time, 1.0)
		self.assertEqual(e.stop_time, 2.0)
		self.assertEqual(e.day, 'Monday')

	def test_create_event_bad_initial_start_time(self):
		with self.assertRaises(ValueError):
			e = Event(3.0, 2.0, 'Sunday', 1)
		with self.assertRaises(ValueError):
			r = Event(24.0, 3.0, 'Sunday', 1)

	def test_create_event_bad_changed_start_time(self):
		e = Event(1.0, 2.0, 'Tuesday', 1)
		with self.assertRaises(ValueError):
			e.start_time = 3.0
		with self.assertRaises(ValueError):
			e.start_time = 24.0

	def test_create_event_bad_initial_end_time(self):
		with self.assertRaises(ValueError):
			e = Event(2.0,1.0, 'Wednesday', 1)
		with self.assertRaises(ValueError):
			r = Event(4.0,24.00, 'Thursday', 1)

	def test_create_event_with_bad_changed_end_time(self):
		e = Event(2.0, 3.0, 'Friday', 1)
		with self.assertRaises(ValueError):
			e.stop_time = 1.0
		with self.assertRaises(ValueError):
			e.stop_time = 200.00

	def test_greater_than_operator(self):
		e1 = Event(5.0, 6.0, 'Monday', 1)
		e2 = Event(6.0, 7.0, 'Tuesday', 1)
		assert(e2 > e1)
		assert(not(e1 > e2))
		e2.start_time = 5.0
		assert(not(e2 > e1))
		assert(not(e1 > e2))

	def test_less_than_operator(self):
		e1 = Event(5.0, 6.0, 'Monday', 1)
		e2 = Event(6.0, 7.0, 'Tuesday', 1)
		assert(not(e2 < e1))
		assert(e1 < e2)
		e2.start_time = 5.0
		assert(not(e2 < e1))
		assert(not(e1 < e2))

	def test_days(self):
		e = Event(1.0, 2.0, 'Monday', 1)
		self.assertEqual(e.day, 'Monday')
		e.day = 'Tuesday'
		self.assertEqual(e.day, 'Tuesday')
		e.day = 'Wednesday'
		self.assertEqual(e.day, 'Wednesday')
		e.day = 'Thursday'
		self.assertEqual(e.day, 'Thursday')
		e.day = 'Friday'
		self.assertEqual(e.day, 'Friday')
		e.day = 'Saturday'
		self.assertEqual(e.day, 'Saturday')
		e.day = 'Sunday'
		self.assertEqual(e.day, 'Sunday')

		with self.assertRaises(ValueError):
			e.day = 'Funday'
		self.assertEqual(e.day,'Sunday')

	def tearDown(self):
		pass

if __name__ == "__main__":
	unittest.main()