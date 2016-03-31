import select
import socket
from JSON import JSON_Interface
from database import Database

from interface import Interface

def broadcast(message: str, soc_list: list):
	for s in soc_list:
		if s != server_socket and s != ipc_socket:
			try:
				s.send(message.encode('utf-8'))
			except TimeoutError:
				soc_list.remove(s)


interface = Interface()
SOCKET_LIST = []
RECV_BUFFER = 8192
ipc_file = "ipc_file.txt"
file_data = ""
db = Database()
json_conversion = JSON_Interface()



SOCKET_LIST = []
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
host = socket.gethostname()
port = 5533
print("The server hostname is: " + host + " on port: " + str(port))

server_socket.bind(('', port))
server_socket.listen(5)

SOCKET_LIST.append(server_socket)
server_socket.settimeout(1)

ipc_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ipc_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
ipc_port = 5538
ipc_socket.bind(('localhost', ipc_port))
ipc_socket.listen(5)
print("The localhost port number for IPC communication is ", + ipc_port)

SOCKET_LIST.append(ipc_socket)

local_ip = socket.gethostbyname(socket.gethostname())
print(local_ip)

x = 1

while True:
	try:
		ready_to_read, ready_to_write, in_error = select.select(SOCKET_LIST, [], [], 0)
	except:
		continue

	for sock in ready_to_read:
		if sock == server_socket:
			client_sock, addr = server_socket.accept()
			SOCKET_LIST.append(client_sock)
			client_sock.settimeout(1)
			print("Client (%s, %s) connected" % addr)
		elif sock == ipc_socket:
			print("Got a connection from myself")
			ipc, addr = ipc_socket.accept()
			print("Accepted the socket")
			ipc.close()
			print("Closed the socket")
			f = open(ipc_file, 'r')
			file_data = ""
			for line in f:
				file_data += line
			print(file_data)
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

				broadcast(converted, SOCKET_LIST)
			try:
				data = sock.recv(RECV_BUFFER)
				if data:
					msg = "Server Response: " + data.decode('utf-8')
					print("Received: " + data.decode('utf-8'))
					if data.decode('utf-8') == 'quit':
						print("Closing connection with: " + str(sock))
						sock.close()
						SOCKET_LIST.remove(sock)
					else:
						sock.send(msg)

			except:
				continue
