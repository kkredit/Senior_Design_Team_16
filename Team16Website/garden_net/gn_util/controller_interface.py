"""
	Date: 04/12/2016
	Author: Charles A. Kingston

	controller_interface.py:

	Script is the main hub for all communication through the server portion
	of the GardeNet system. It listens for connections made from other pieces
	of the system

	OUTSIDE PORTS: These ports accept connections from any machines
		PORT: 5530
			This socket connection is a socket to the controller on site. The connection
			is initiated from the controller. Data is sent in JSON form to the on-site
			controller.

	INNER PROCESS COMMUNICATION (IPC) PORTS: These ports only accept connections from
	machines that are on localhost.

		PORT: 5537
			This socket connection looks for a connection from the website. When the
			connection is made, it immediately closes the socket and views a user_info
			and alerts text file in order to figure out which alerts the user wants.

		PORT: 5538
			This socket connection looks for a connection from the website. When the
			connection is made, it immediately closes the socket and views the JSON
			file that the website has written to.

		PORT: 5539
			This socket connection looks for a connection from a test script. When a
			a connection is received from the test script, it sends a true to the
			client, then closes the socket to verify that the server is still
			running correctly
"""

# Third Party imports
import select
import socket
import json
import argparse
import time

# Local Imports
from JSON import JSON_Interface
from database import Database
from event import Event
from gateway_report import Report
from socket import error as SocketError
from weather_forecast import Forecast
from zone import Zone
import errno

global last_message
last_message = False

global begin_message
begin_message = False

"""
############################################# GLOBAL DEFINITIONS ######################################################
"""

# GLOBAL VARIABLEs
RECV_BUFFER = 8192
ipc_file = "ipc_file.txt"
user_info_file = "user_info.txt"
alerts_file = "alerts.txt"
file_data = ""
db = Database(False)
json_conversion = JSON_Interface()
SOCKET_LIST = []
EVENT_LIST = []
forecast = Forecast()
sending_todays_schedule = False

# Parse the command line for the options on ways to run the script
parser = argparse.ArgumentParser(description="A prattle server")
# -v allows the user to view all of the output of the server
parser.add_argument("-v", "--verbose", action="store_true", dest="verbose", help="turn verbose output on")
args = parser.parse_args()

"""******************* START THE SOCKETS ***********************"""
# @server_socket: Socket that listens for new connections
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
host = socket.gethostname()
port = 5530
if args.verbose:
	print("The server hostname is: " + host + " on port: " + str(port))
# Bind it to our host name
server_socket.bind(('', port))
server_socket.listen(5)
# Append it to the list for the select statement
SOCKET_LIST.append(server_socket)
server_socket.settimeout(0)

# @demo_one_socket: Socket that listens for connections from the demo1 page
demo_one_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
demo_one_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
host = socket.gethostname()
demo_one_port = 5531
if args.verbose:
	print("The demo_one_socket port number for IPC communication is ", + demo_one_port)
# Bind it to our host name
demo_one_socket.bind(('localhost', demo_one_port))
demo_one_socket.listen(5)
# Append it to the list for the select statement
SOCKET_LIST.append(demo_one_socket)

# @demo_two_socket: Socket that listens for connections from the demo1 page
demo_two_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
demo_two_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
host = socket.gethostname()
demo_two_port = 5532
if args.verbose:
	print("The demo_two_socket port number for IPC communication is ", + demo_two_port)
# Bind it to our host name
demo_two_socket.bind(('localhost', demo_two_port))
demo_two_socket.listen(5)
# Append it to the list for the select statement
SOCKET_LIST.append(demo_two_socket)

# @garden_status_socket: Socket that listens to a connection from the slider
garden_status_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
garden_status_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
garden_status_port = 5535
# Bind it to the localhost for ipc communication
garden_status_socket.bind(('localhost', garden_status_port))
garden_status_socket.listen(5)
if args.verbose:
	print("The garden_status_socket port number for IPC communication is ", + garden_status_port)
# Append it to the list for the select statement
SOCKET_LIST.append(garden_status_socket)

# @user_alert_socket: Socket that listens to a connection from the website
user_alert_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
user_alert_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
user_alert_port = 5537
# Bind it to the localhost for ipc communication
user_alert_socket.bind(('localhost', user_alert_port))
user_alert_socket.listen(5)
if args.verbose:
	print("The user_alert_socket port number for IPC communication is ", + user_alert_port)
# Append it to the list for the select statement
SOCKET_LIST.append(user_alert_socket)

#@ipc_socket: Socket that listens to a connection from the website
ipc_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ipc_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
ipc_port = 5538
# Bind it to the localhost for ipc communication
ipc_socket.bind(('localhost', ipc_port))
ipc_socket.listen(5)
if args.verbose:
	print("The localhost port number for IPC communication is ", + ipc_port)
# Append it to the list for the select statement
SOCKET_LIST.append(ipc_socket)

# @test_server_socket: Socket that listens to a connection from the test_server.py script
test_server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
test_server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
test_server_port = 5539
# Bind it to the localhost for ipc communication
test_server_socket.bind(('localhost', test_server_port))
test_server_socket.listen(5)
if args.verbose:
	print("The test_server_socket port number for IPC communication is ", + test_server_port)
# Append it to the list for the select statement
SOCKET_LIST.append(test_server_socket)

"""
#######################################################################################################################
"""


"""
############################################# HELPER FUNCTIONS ########################################################
"""

"""
	A function to broadcast a message to all the connected sockets

	@:param str message: the string of the message to be sent
	@:param list soc_list: the list of connected sockets
"""
def broadcast(message: str, soc_list: list):
	# iterate over the socket list
	for s in soc_list:
		# If the socket is not us, not the ipc_socket, or not the test_server_socket
		if s != server_socket and s != ipc_socket and s != test_server_socket and s != user_alert_socket\
				and s != demo_one_socket and s != garden_status_socket and s != demo_two_socket:
			# try to send the message
			try:
				# send the message to the controller
				#encoded = base64.b64encode(message.encode('ascii'))
				s.sendall(message.encode('utf-8'))
				# close the socket
				#s.close()
				# remove it from our list
				#soc_list.remove(s)
			# catch the TimeoutError and remove the socket from out list
			except SocketError as e:
				if e.errno == errno.ECONNRESET:
					broadcast(message, soc_list)
				else:
					soc_list.remove(s)

"""
	A function to create a list of events given a JSON string

	@:param str json_str: a JSON formatted string to be converted to python Event objects
"""
def create_event_list(json_str: str):
	# temp = json_str[2:7]
	# print(temp)
	i = 1
	j = 0
	parsed = json.loads(json_str)
	temp = True
	while (temp):
		zone_string = "Zone" + str(i)
		try:
			parsed[zone_string]
			# Anything after this line will not execute if a KeyError is raised
			temp2 = True
			while (temp2):
				event_string = "event" + str(j)
				try:
					day = parsed[zone_string][event_string]['day']
					zone_id = parsed[zone_string][event_string]['zone_ID']
					temp_start = parsed[zone_string][event_string]['start_time']
					start_time = float(temp_start.replace(":", "."))
					temp_stop = parsed[zone_string][event_string]['stop_time']
					stop_time = float(temp_stop.replace(":", "."))
					valve = parsed[zone_string][event_string]['valve_num']
					event = Event(start_time, stop_time, day, zone_id, valve)
					EVENT_LIST.append(event)
					# print(start_time)
					# print(parsed[zone_string][event_string])
					# print(event)
					j += 1
				except(KeyError):
					temp2 = False
					j = 0
		except(KeyError):
			temp = False

		i += 1
"""
	A function to send events one by one
"""
def send_event(begin_message):
	# check to see if the controller is listening
	if len(SOCKET_LIST) > 3:
		# if we have sent all of the events in EVENT_LIST, send the DONE EOT message
		if len(EVENT_LIST) == 0:
			last_message = False
			if args.verbose:
				print("Sending the EOT message")
			done_message = "DONE"
			broadcast(done_message, SOCKET_LIST)
			time.sleep(5)
			if args.verbose:
				print("Successfully sent!")
			global sending_todays_schedule
			sending_todays_schedule = True
		elif begin_message:
			begin = "START" + str(len(EVENT_LIST))
			if args.verbose:
				print("Sending the BOT message: " + begin)
			broadcast(begin, SOCKET_LIST)
			time.sleep(5)
			if args.verbose:
				print("Successfully sent!")
		# there's still events in the EVENT_LIST
		else:
			# pop the front of the list
			it = EVENT_LIST.pop()
			if args.verbose:
				print("Sending: " + str(it))
				print("The length of the event list is: " + str(len(EVENT_LIST)))
			broadcast(str(it), SOCKET_LIST)
			time.sleep(5)
			if args.verbose:
				print("Successfully sent!")
			# set the EOT flag
			if len(EVENT_LIST) == 0:
				global last_message
				last_message = True

def get_todays_schedule():
	day = forecast.get_current_day()
	if args.verbose:
		print("Today is " + day)
	try:
		events = db.get_all_events_on_day(day)
	except Exception as e:
		if args.verbose:
			print(e)
	if args.verbose:
		print("Found " + str(len(events)) + " Events today.")
	EVENT_LIST.clear()
	for item in events:
		EVENT_LIST.append(item)
		if args.verbose:
			print(item)
	send_event(True)


def convert_to_zero_or_one(value: str):
	if value == "true":
		return "1"
	else:
		return "0"

"""
#######################################################################################################################
"""

"""
################################################# MAIN LOOP ###########################################################
"""

while True:
	try:
		ready_to_read, ready_to_write, in_error = select.select(SOCKET_LIST, [], [], 0)
	except:
		continue

	if len(EVENT_LIST) != 0 or last_message:
		send_event(False)

	for sock in ready_to_read:
		if sock == server_socket:
			client_sock, addr = server_socket.accept()
			SOCKET_LIST.append(client_sock)
			client_sock.settimeout(1)
			if args.verbose:
				print("Client (%s, %s) connected" % addr)
			welcome = "Welcome to GardeNet"
		# client_sock.send(welcome.encode('utf-8'))
		elif sock == user_alert_socket:
			if args.verbose:
				print("Got a connection from user_alert_socket")
			user_soc, user_address = user_alert_socket.accept()
			if args.verbose:
				print("Accepted the user_alert_socket")
			user_soc.close()
			if args.verbose:
				print("Closed the user_alert_socket")
			user_info = []
			alerts = []

			f = open(user_info_file, 'r')
			for line in f:
				user_info.append(line)
			phone_number = str(user_info[1]).split(" ")[1].split("\n")[0]
			f.close()
			f2 = open(alerts_file, 'r')
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
			alert_string = "SMS" + phone_number + "%" + opcode_01 + "%" + opcode_02 + "%" \
						   + opcode_03 + "%" + opcode_04
			if args.verbose:
				print("Sending alert string to the gateway: " + alert_string)
			broadcast(alert_string, SOCKET_LIST)
			if args.verbose:
				print("Successfully sent!")
		elif sock == test_server_socket:
			test, test_addr = test_server_socket.accept()
			if args.verbose:
				print("Got a connection from test")
			test_response = "true"
			if args.verbose:
				print("Sending true")
			test.send(test_response.encode('utf-8'))
			test.close()
			if args.verbose:
				print("Closed the test socket")
		elif sock == garden_status_socket:
			if args.verbose:
				print("Got a connection from the slider")
			status_client, status_addr = garden_status_socket.accept()
			status_file = open("garden_power_status.txt", "r")
			status = status_file.readline()
			if args.verbose:
				print("Sending: " + status)
			broadcast(str(status), SOCKET_LIST)
			if args.verbose:
				print("Successfully sent!")
			status_client.close()
			if args.verbose:
				print("Closed the test garden_power_status.txt")
		elif sock == demo_one_socket:
			if args.verbose:
				print("Got a connection from the demo1")
			demo1_client, demo1_addr = demo_one_socket.accept()
			demo1_client.close()
			demo1_file = open("demo1_file.txt", "r")
			if args.verbose:
				print("Closed demo1 socket")
			demo1_file_contents = []
			for line in demo1_file:
				demo1_file_contents.append(line)
			valve1 = str(demo1_file_contents[1]).split('\t')[0]
			valve2 = str(demo1_file_contents[1]).split('\t')[1]
			valve3 = str(demo1_file_contents[1]).split('\t')[2]
			send_string = "DEMO1%"+convert_to_zero_or_one(valve1) + "%" + convert_to_zero_or_one(valve2) + "%" + convert_to_zero_or_one(valve3)
			if args.verbose:
				print("Sending: " + send_string)
			broadcast(send_string, SOCKET_LIST)
			if args.verbose:
				print("Successfully sent!")
		elif sock == demo_two_socket:
			if args.verbose:
				print("Got a connection from the demo2")
			demo2_client, demo2_addr = demo_two_socket.accept()
			demo2_client.close()
			demo2_file = open("demo2_file.txt", "r")
			if args.verbose:
				print("Closed demo2 socket")
			demo2_file = open("demo2_file.txt", "r")
			string = ""
			for line in demo2_file:
				string += line
			create_event_list(string)
			send_event(True)
		elif sock == ipc_socket:
			if args.verbose:
				print("Got a connection from myself")
			ipc, addr = ipc_socket.accept()
			if args.verbose:
				print("Accepted the socket")
			ipc.close()
			if args.verbose:
				print("Closed the socket")
			f = open(ipc_file, 'r')
			file_data = ""
			for line in f:
				file_data += line
			if args.verbose:
				print(file_data)
			f.close()

			if str(file_data.split('\n')[0].upper()) == "TRUE" \
					or str(file_data.split('\n')[0].upper()) == "FALSE" \
					or str(file_data.split('\n')[0].upper()) == "NOEVENTS":
				broadcast(file_data, SOCKET_LIST)
			else:
				db.clear_database()
				json_conversion.create_events_from_JSON_string(file_data, db)

				convert = JSON_Interface()
				converted = convert.all_events_from_DB_to_JSON(db)
				if args.verbose:
					print(converted)

				f2 = open("current_schedule_in_db.txt", 'w')
				f2.write(file_data)
				f2.close()

				get_todays_schedule()
				print("The length of the event list is: " + str(len(EVENT_LIST)))

		try:
			data = sock.recv(RECV_BUFFER)
			if data:
				print("Received: " + data.decode('utf-8'))
				if str(data.decode('utf-8')).upper() == "RESENDSCHEDULE":
					if args.verbose:
						print("I need to resend the schedule")
					resend_port = 5538
					try:
						resend = socket.create_connection(('localhost', resend_port))
						if args.verbose:
							print("Reconnecting to local host")
					except:
						if args.verbose:
							print("Unable to connect")
					resend.close()
				elif str(data.decode('utf-8')).upper() == "SCHEDULE?":
					if args.verbose:
						print("Getting today's schedule")
					get_todays_schedule()
					if args.verbose:
						print("Successfully sent today's schedule")
				elif str(data.decode('utf-8')).upper() == "ALERT?":
					if args.verbose:
						print("Opening alert port")
					awaken_alerts_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
					awaken_alerts_socket = socket.create_connection(('localhost', 5537))
					awaken_alerts_socket.close()
					if args.verbose:
						print("Successfully sent alerts")
				elif str(data.decode('utf-8')).upper() == "STATE?":
					if args.verbose:
						print("Sending the current state!")
					state_file = open("garden_power_status.txt", "r")
					state = state_file.readline().split("\n")[0]
					if args.verbose:
						print("Sending: " + state + " .........")
					broadcast(str(state), SOCKET_LIST)
					if args.verbose:
						print("Successfully sent the state!")
				else:
					report = Report(data.decode('utf-8'))
					if args.verbose:
						if report.opcode == "00":
							print(str(report))

		except:
			continue

"""
#######################################################################################################################
"""


