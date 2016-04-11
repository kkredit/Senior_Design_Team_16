from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import and_, exists

from base import Base, Meta
from zone import Zone
from event import Event


class Database:
	def __init__(self, new: bool):
		if new:
			self.engine = create_engine('sqlite://')
		else:
			self.engine = create_engine('sqlite:///database.db')

		self.meta = Meta
		self.base = Base
		self.base.metadata.bind = self.engine
		self.base.metadata.create_all()

		# self.session_factory = sessionmaker(bind=self.engine)
		# self.session = scoped_session(self.session_factory)()

		create_session = sessionmaker()
		create_session.configure(bind=self.engine)
		self.session = create_session()

	def clear_database(self):
		self.meta.drop_all()
		self.meta.create_all()
		self.session.commit()

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

	def get_events_on_day_for_zone(self, day, zone: int):
		# for zone in zone_query:
		# 	print(zone)
		if self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all():
			return sort_event_list(self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all())
		elif len(self.session.query(Event).filter(Event._owner == zone).all()) == 0:
			raise KeyError("Zone does not exist")
		elif len(self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all()) == 0:
			raise ValueError("No events at zone for the given day")
		# WE SHOULD NEVER EVER GET HERE
		else:
			raise OverflowError("Something went terribly wrong...")



def sort_event_list(event_list: list):
	event_list.sort()
	return event_list

if __name__ == "__main__":
	db = Database()

	z = Zone(1)
	z2 = Zone(2)
	z3 = Zone(3)

	db.add_zone(z)
	db.add_zone(z2)
	db.add_zone(z3)

	test_event0 = Event(3.0, 4.0, 'Monday', 3)
	test_event1 = Event(1.0, 2.0, 'Monday', 3)
	test_event2 = Event(2.0, 3.0, 'Monday', 3)
	test_event3 = Event(1.5, 2.5, 'Monday', 3)
	test_event4 = Event(22.0, 23.59, 'Monday', 3)
	test_event5 = Event(1.0, 2.0, 'Tuesday', 1)
	test_event6 = Event(1.0, 3.0, 'Tuesday', 3)
	test_event7 = Event(1.0, 2.0, 'Saturday', 1)

	db.add_event(test_event0)
	db.add_event(test_event1)
	db.add_event(test_event2)
	db.add_event(test_event3)
	db.add_event(test_event4)
	db.add_event(test_event5)
	db.add_event(test_event6)
	db.add_event(test_event7)

	results = db.get_all_events_for_zone(1)

	assert (results != [])

	db.clear_database()

	try:
		results = db.get_all_events_for_zone(1)
	except ValueError:
		print("The DB is empty")