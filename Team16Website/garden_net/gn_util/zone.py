"""
	Date: 04/12/2016
	Author: Charles A. Kingston

	zone.py:

	Script that defines a class that is used throughout the GardeNet system. The zone class
	extends the BASE that is defined. This means that it will get correctly linked to the
	Database and any other classes that are linked to it as well.

"""
# Third Party Imports
from sqlalchemy import Column, Integer, Sequence

# Local Imports
from base import Base

class Zone(Base):
	__tablename__ = 'zones'
	"""
		An explicit constructor to set the zone_id

		@:param zone_id an int to be set to the zone_id
	"""
	def __init__(self, zone_id):
		self._zone_id = zone_id

	id = Column(Integer, Sequence('zone_id_sequence'), primary_key=True)
	_zone_id = Column(Integer)

	"""
		A method used to convert the Zone object to a JSON string

		@:return a string of JSON for the zone
	"""
	def to_JSON(self):
		return "\"Zone" + str(self._zone_id) + "\":"

	# getter
	@property
	def zone_id(self):
		return self._zone_id

if __name__ == "__main__":
	z = Zone(1)
	z_JSON = z.to_JSON()
	print(z_JSON)

