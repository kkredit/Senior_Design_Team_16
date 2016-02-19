import unittest

from garden_net.gn_util.schedule import Schedule
from garden_net.gn_util.event import Event

class (unittest.TestCase):
	def setUp(self):
		pass

	def test_constructor_empty_array_for_each_key(self):
		s = Schedule()
		for key in s:
			self.assertEqual(len(s[key]),0)

	def tearDown(self):
		pass

if __name__ == "__main__":
	unittest.main()