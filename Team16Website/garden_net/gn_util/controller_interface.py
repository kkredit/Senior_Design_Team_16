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

global last_message
last_message = False

global begin_message
begin_message = False

"""
############################################# GLOBAL DEFINITIONS ######################################################
"""

# GLOBAL VARIABLE
RECV_BUFFER = 8192
ipc_file = "ipc_file.txt"
file_data = ""
db = Database(False)
json_conversion = JSON_Interface()
SOCKET_LIST = []
EVENT_LIST = []

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

# @ipc_socket: Socket that listens to a connection from the website
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
		if s != server_socket and s != ipc_socket and s != test_server_socket:
			# try to send the message
			try:
				# send the message to the controller
				s.send(message.encode('utf-8'))
				# close the socket
				s.close()
				# remove it from our list
				soc_list.remove(s)
			# catch the TimeoutError and remove the socket from out list
			except TimeoutError:
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
					if (zone_id != i):
						zone_id = i
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
				print("Last event was sent, sending the DONE message")
			done_message = "DONE"
			broadcast(done_message, SOCKET_LIST)

			if args.verbose:
				print("Sent successfully ")
		elif begin_message:
			begin = "START"
			broadcast(begin, SOCKET_LIST)
		# there's still events in the EVENT_LIST
		else:
			# pop the front of the list
			it = EVENT_LIST.pop()
			if args.verbose:
				print("Sending: " + str(it))
			broadcast(str(it), SOCKET_LIST)
			time.sleep(5)
			if args.verbose:
				print("Successfully sent!")
			# set the EOT flag
			if len(EVENT_LIST) == 0:
				global last_message
				last_message = True

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
<<<<<<< HEAD
			if args.verbose:
				print("Client (%s, %s) connected" % addr)
			welcome = "Welcome to GardeNet"
		# client_sock.send(welcome.encode('utf-8'))
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
		elif sock == ipc_socket:
			if args.verbose:
				print("Got a connection from myself")
			ipc, addr = ipc_socket.accept()
			if args.verbose:
				print("Accepted the socket")
			ipc.close()
			if args.verbose:
				print("Closed the socket")
=======
			print("Client (%s, %s) connected" % addr)
			welcome = "BIG COCK"
			#client_sock.send(welcome.encode('utf-8'))
		elif sock == test_server_socket:
			test, test_addr = test_server_socket.accept()
			print("Got a connection from test")
			test_response = "true"
			print("Sending true")
			test.send(test_response.encode('utf-8'))
			test.close()
			print("Closed the test socket")
		elif sock == ipc_socket:
			print("Got a connection from myself")
			ipc, addr = ipc_socket.accept()
			print("Accepted the socket")
			ipc.close()
			print("Closed the socket")
>>>>>>> d20f819b9b7fb347fa957b5924e73131db3fb0df
			f = open(ipc_file, 'r')
			file_data = ""
			for line in f:
				file_data += line
<<<<<<< HEAD
			if args.verbose:
				print(file_data)
=======
			print(file_data)
>>>>>>> d20f819b9b7fb347fa957b5924e73131db3fb0df
			f.close()

			if file_data == "true" or file_data == "false":
				broadcast(file_data, SOCKET_LIST)
			else:
				db.clear_database()
				json_conversion.create_events_from_JSON_string(file_data, db)

				convert = JSON_Interface()
				converted = convert.all_events_from_DB_to_JSON(db)

				f2 = open("current_schedule_in_db.txt", 'w')
				f2.write(file_data)
				f2.close()

<<<<<<< HEAD
				# broadcast(converted, SOCKET_LIST)
				# broadcast(file_data, SOCKET_LIST)
				if args.verbose:
					print("Creating the event list")
				for item in EVENT_LIST:
					EVENT_LIST.remove(item)
				create_event_list(converted)
				send_event(True)
				if args.verbose:
					print("Event list created")
				# send_event()
				if args.verbose:
					for event in EVENT_LIST:
						print(str(event))
				# sock.close()
				# print(file_data)
				# broadcast(file_data, SOCKET_LIST)
				# print(file_data)
				# broadcast(file_data, SOCKET_LIST)
=======
				#broadcast(converted, SOCKET_LIST)
				#broadcast(file_data, SOCKET_LIST)
				#print("Creating the event list")
				for item in EVENT_LIST:
					EVENT_LIST.remove(item)
				create_event_list(converted)
				#print("Event list created")
				#send_event()
				for event in EVENT_LIST:
					print(str(event))
				#sock.close()
				#print(file_data)
				#broadcast(file_data, SOCKET_LIST)
				#print(file_data)
				#broadcast(file_data, SOCKET_LIST)
>>>>>>> d20f819b9b7fb347fa957b5924e73131db3fb0df

		try:
			data = sock.recv(RECV_BUFFER)
			if data:
				msg = "Server Response: " + data.decode('utf-8')
				print("Received: " + data.decode('utf-8'))
				if str(data.decode('utf-8')) == 'AT+CREG?' or str(data.decode('utf-8')) == 'ATZ':
					print("Closing connection with: " + str(sock))
					sock.close()
					SOCKET_LIST.remove(sock)
				else:
					sock.send(msg)

		except:
			continue

<<<<<<< HEAD
"""
#######################################################################################################################
"""
=======

>>>>>>> d20f819b9b7fb347fa957b5924e73131db3fb0df
