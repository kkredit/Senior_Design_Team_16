from sqlalchemy import Column, Integer, Sequence

from garden_net.database.base import Base

class Zone(Base):
	__tablename__ = 'zones'

	def __init__(self, zone_id):
		self._zone_id = zone_id

	id = Column(Integer, Sequence('zone_id_sequence'), primary_key=True)
	_zone_id = Column(Integer)

	def to_JSON(self):
		return "\"zone" + str(self._zone_id) + "\":"

	@property
	def zone_id(self):
		return self._zone_id

if __name__ == "__main__":
	z = Zone(1)
	z_JSON = z.to_JSON()
	print(z_JSON)
