#from historical_database import Historical_Database

#from historical_database_interface import Historical_Database_Interface
#from gateway_report import Report
#Report("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
# to = Historical_Database_Interface("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
# db = Historical_Database(True)
#
# db.add_row(it)
# db.add_row(to)
#
# the = db.get_all()
# for item in the:
# 	print(item)

# from database import Database
# from event import Event
# from zone import Zone
#
# db = Database(True)
#
# z = Zone(1)
# z2 = Zone(2)
# z3 = Zone(3)
#
# db.add_zone(z)
# db.add_zone(z2)
# db.add_zone(z3)
#
# test_event0 = Event(3.0, 4.0, 'Monday', 1, 1)
# test_event1 = Event(1.0, 2.0, 'Monday', 1, 1)
# test_event2 = Event(2.0, 3.0, 'Monday', 1, 1)
# test_event3 = Event(1.5, 2.5, 'Monday', 1, 1)
# test_event4 = Event(1.0, 2.0, 'Monday', 1, 2)
# test_event5 = Event(1.0, 2.0, 'Monday', 1, 3)
# test_event6 = Event(1.5, 3.0, 'Monday', 1, 3)
# test_event7 = Event(1.0, 2.0, 'Monday', 1, 3)
#
# db.add_event(test_event0)
# db.add_event(test_event1)
# db.add_event(test_event2)
# db.add_event(test_event3)
# db.add_event(test_event4)
# db.add_event(test_event5)
# db.add_event(test_event6)
# db.add_event(test_event7)
#
# it = db.get_events_on_day_for_zone_and_valve('Monday', 1, 1)
#
# for item in it:
# 	print(item)

""" Testing the alert engine """
"""
from gateway_report import Report
#it = Report("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
#print(it)
Report("01%2%5%0.00")
# Report("02")
# Report("03")
# Report("04%1%2")
"""

user_info_file = "user_info.txt"
alerts_file = "alerts.txt"
user_info = []
f = open(user_info_file, 'r')
for line in f:
	user_info.append(line)
phone_number = str(user_info[1]).split(" ")[1].split("\n")[0]
print(phone_number)

f2 = open(alerts_file, 'r')
alerts = []
for line in f2:
	alerts.append(line)
opcode_01 = str(alerts[0]).split('\t')[1]
if opcode_01.upper() == "TRUE":
	opcode_01 = "1"
else:
	opcode_01 = "0"
opcode_02 = str(alerts[3]).split('\t')[1]
if opcode_02.upper() == "TRUE":
	opcode_02 = "1"
else:
	opcode_02 = "0"
opcode_03 = str(alerts[2]).split('\t')[1]
if opcode_03.upper() == "TRUE":
	opcode_03 = "1"
else:
	opcode_03 = "0"
opcode_04 = str(alerts[1]).split('\t')[1]
if opcode_04.upper() == "TRUE":
	opcode_04 = "1"
else:
	opcode_04 = "0"

send_string = phone_number + "%" + opcode_01 + "%" + opcode_02 + "%" + opcode_03 + "%" + opcode_04

print(send_string)