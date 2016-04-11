from zone import Zone
from database import Database
from event import Event
from JSON import JSON_Interface
import json

db = Database(True)
z = Zone(1)
db.add_zone(z)
z2 = Zone(2)
db.add_zone(z2)
z3 = Zone(3)
db.add_zone(z3)
z4 = Zone(1)

e = Event(1.0, 2.0, 'Monday', 1, 1)

db.add_event(e)

it = db.get_all_events_on_day('Monday')

# for item in it:
# 	print(item)

myjson = JSON_Interface()
name = 'input_JSON.txt'

string = myjson.from_JSON(name)
#parsed = json.loads(string)

myjson.create_events_from_JSON_string(string, db)

convert = JSON_Interface()
converted = convert.all_events_from_DB_to_JSON(db)



#print(parsed['zone1']['event0']['day'])
