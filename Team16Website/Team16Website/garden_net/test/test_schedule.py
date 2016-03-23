import unittest

from garden_net.gn_util.schedule import Schedule
from garden_net.gn_util.event import Event

class TestSchedule(unittest.TestCase):
	def setUp(self):
		pass

	def test_constructor_empty_array_for_each_key(self):
		s = Schedule()
		for key in s.mySchedule.keys():
			self.assertEqual(len(s.mySchedule[key]), 0)

	def test_insert_event_into_schedule(self):
		s = Schedule()
		e1 = Event(1.00, 2.00, 'Monday', 1)
		s.insert_event('Monday', e1)
		self.assertEqual(len(s.mySchedule['Monday']), 1)

	def tearDown(self):
		pass

if __name__ == "__main__":
	unittest.main()