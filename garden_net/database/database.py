from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import and_
from garden_net.database.base import Base, Meta
from garden_net.gn_util.zone import Zone
from garden_net.gn_util.event import Event


class Database:
	def __init__(self):
		self.engine = create_engine('sqlite://')

		self.meta = Meta
		self.base = Base
		self.base.metadata.bind = self.engine
		self.base.metadata.create_all()

		# self.session_factory = sessionmaker(bind=self.engine)
		# self.session = scoped_session(self.session_factory)()

		create_session = sessionmaker()
		create_session.configure(bind=self.engine)
		self.session = create_session()

	def add_zone(self, zone: Zone):
		if self.session.query(Zone).filter(Zone._zone_id == zone._zone_id).all():
			raise ValueError("That zone already exists")
		else:
			self.session.add(zone)
			self.session.commit()

	def add_event(self, event: Event):
		if not(self.session.query(Zone).filter(Zone._zone_id == event.owner).all()):
			raise ValueError("That zone does not exist")
		elif self.session.query(Event).filter(and_(Event._owner == event.owner,\
		Event._stop_time == event.stop_time, Event._start_time == event.start_time, \
		Event._day == event.day)).all():
			raise ValueError("That event already exits")
		self.session.add(event)
		self.session.commit()

	def get_all_events_on_day(self, day: str):
		if self.session.query(Event).filter(Event._day == day).all():
			return sort_event_list(self.session.query(Event).filter(Event._day == day).all())
		else:
			raise ValueError("That day does not exist in the database")

	def get_all_events_for_zone(self, zone: int):
		if self.session.query(Event).filter(Event._owner == zone).all():
			return sort_event_list(self.session.query(Event).filter(Event._owner == zone).all())
		else:
			raise ValueError("That zone does not exist in the database")

	def get_events_on_day_for_zone(self, day, zone):
		if self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)):
			return sort_event_list(self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all())
		else:
			raise ValueError


def sort_event_list(event_list: list):
	event_list.sort()
	return event_list
