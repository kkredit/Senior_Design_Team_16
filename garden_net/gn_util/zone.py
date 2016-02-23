from sqlalchemy import Column, Integer, Sequence

from garden_net.database.base import Base

class Zone(Base):
	__tablename__ = 'zones'

	def __init__(self, zone_id):
		self._zone_id = zone_id

	id = Column(Integer, Sequence('zone_id_sequence'), primary_key=True)
	_zone_id = Column(Integer)
