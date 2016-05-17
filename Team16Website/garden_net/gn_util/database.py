"""
	Date: 04/12/2016
	Author: Charles A. Kingston

	database.py:

	Script creates a class for the Database. It allows the user to interact
	with the database from an abstracted level in other scripts avoiding
	any other complicated interactions that may exists. SQLAlchemy is used
	and allows for Object Relational Mapping (ORM) to allow all interface
	with the Database to be used through python objects

"""

# Third Party Imports
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import and_, or_, exists

# Local Imports
from base import Base, Meta
from zone import Zone
from event import Event


class Database:
	"""
		A explicit constructor that creates the connection to the database

		@:param self: passes ourselves in as an object
		@:param bool new: a boolean to tell whether to start a new database or use the already saved one

	"""
	def __init__(self, new: bool):
		# if True, create a new database
		if new:
			self.engine = create_engine('sqlite://')
		# else, extend off of the already existing database
		else:
			self.engine = create_engine('sqlite:///database.db')

		# Set the Meta, Base
		self.meta = Meta
		self.base = Base
		# Bind the metadata and base to the engine and create it
		self.base.metadata.bind = self.engine
		self.base.metadata.create_all()

		# Start the session bound to our engine
		create_session = sessionmaker()
		create_session.configure(bind=self.engine)
		self.session = create_session()

	"""
		A function to clear the database

		@:param: self: passes ourself in to delete all items in the database
	"""
	def clear_database(self):
		self.meta.drop_all()
		self.meta.create_all()
		self.session.commit()
	"""
		A function to add a new Zone to the database

		@:param self: passes self in as an object
		@:param Zone zone: passes a Zone object in to add the zone

	"""
	def add_zone(self, zone: Zone):
		if self.session.query(Zone).filter(Zone._zone_id == zone._zone_id).all():
			raise ValueError("That zone already exists")
		else:
			self.session.add(zone)
			self.session.commit()
	"""
		A function to add a new Event to the database

		@:param self: passes self in as an object
		@:param Event event: passes an Event object in to add the event

	"""
	def add_event(self, event: Event):
		if not(self.session.query(Zone).filter(Zone._zone_id == event.owner).all()):
			raise ValueError("That zone does not exist")
		elif self.session.query(Event).filter(and_(Event._owner == event.owner,\
		Event._stop_time == event.stop_time, Event._start_time == event.start_time, \
		Event._day == event.day, Event.valve_num == event.valve_num)).all():
			raise ValueError("That event already exits")
		self.session.add(event)
		self.session.commit()

	"""
		A function to get all of the events in the database on a certain day

		@:param self: passes self in as an object
		@:param str day: a string of the day we are looking for in the database

		@:return: returns a list of the events on the given day
	"""
	def get_all_events_on_day(self, day: str):
		if self.session.query(Event).filter(or_(Event._day == day, Event._day == "Everyday")).all():
			return sort_event_list(self.session.query(Event).filter(or_(Event._day == day, Event._day == "Everyday")).all())
		else:
			raise ValueError("That day does not exist in the database")
	"""
		A function to get all of the events in the database for a certain Zone

		@:param self: passes self in as an object
		@:param int Zone: a int of the zone we are looking for in the database

		@:return: returns a list of the events for a given zone
	"""
	def get_all_events_for_zone(self, zone: int):
		if self.session.query(Event).filter(Event._owner == zone).all():
			return sort_event_list(self.session.query(Event).filter(Event._owner == zone).all())
		else:
			raise ValueError("That zone does not exist in the database")
	"""
		A function to get all of the events in the database for a certain Zone on a certain day

		@:param self: passes self in as an object
		@:param int Zone: a int of the zone we are looking for in the database
		@:param str day: a string of the day we are looking for in the database

		@:return: returns a list of the events for a given zone	on a certain day
	"""
	def get_events_on_day_for_zone(self, day, zone: int):
		# for zone in zone_query:
		# 	print(zone)
		if self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all():
			#if self.session.query(Event).filter(and_(Event._owner == zone, or_(Event._day == day, Event._day == "Everyday"))).all():
			return sort_event_list(self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all())
		elif len(self.session.query(Event).filter(Event._owner == zone).all()) == 0:
			raise KeyError("Zone does not exist")
		elif len(self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all()) == 0:
			raise ValueError("No events at zone for the given day")
		# WE SHOULD NEVER EVER GET HERE
		else:
			raise OverflowError("Something went terribly wrong...")

	def get_events_on_day_for_zone_and_valve(self, day, zone: int, valve: int):
		# for zone in zone_query:
		# 	print(zone)
		if self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day, Event._valve_num == valve)).all():
			return sort_event_list(self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day, Event._valve_num == valve)).all())
		elif len(self.session.query(Event).filter(Event._owner == zone).all()) == 0:
			raise KeyError("Zone does not exist")
		elif len(self.session.query(Event).filter(and_(Event._owner == zone, Event._day == day)).all()) == 0:
			raise ValueError("No events at zone for the given day")
		# WE SHOULD NEVER EVER GET HERE
		else:
			raise OverflowError("Something went terribly wrong...")

"""
	A helper function to sort the list of events

	@:param list list: a list of events to be sorted

	@:return: returns a sorted list of events
"""
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
