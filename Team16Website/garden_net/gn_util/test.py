# from historical_database import Historical_Database
#
# from historical_database_interface import Historical_Database_Interface
#
#
# it = Historical_Database_Interface("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
# db = Historical_Database(True)
#
# db.add_row(it)
#
# the = db.get_all()
#
# for item in the:
# 	the

from database import Database
from event import Event
from zone import Zone

db = Database(True)

z = Zone(1)
z2 = Zone(2)
z3 = Zone(3)

db.add_zone(z)
db.add_zone(z2)
db.add_zone(z3)

test_event0 = Event(3.0, 4.0, 'Monday', 1, 1)
test_event1 = Event(1.0, 2.0, 'Monday', 1, 1)
test_event2 = Event(2.0, 3.0, 'Monday', 1, 1)
test_event3 = Event(1.5, 2.5, 'Monday', 1, 1)
test_event4 = Event(1.0, 2.0, 'Monday', 1, 2)
test_event5 = Event(1.0, 2.0, 'Monday', 1, 3)
test_event6 = Event(1.5, 3.0, 'Monday', 1, 3)
test_event7 = Event(1.0, 2.0, 'Monday', 1, 3)

db.add_event(test_event0)
db.add_event(test_event1)
db.add_event(test_event2)
db.add_event(test_event3)
db.add_event(test_event4)
db.add_event(test_event5)
db.add_event(test_event6)
db.add_event(test_event7)

it = db.get_events_on_day_for_zone_and_valve('Monday', 1, 1)

for item in it:
	print(item)