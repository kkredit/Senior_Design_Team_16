import socket
import select

from interface import Interface

interface = Interface()
SOCKET_LIST = []

# soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# host = socket.gethostname()
# print(host)
# port = 5555
# print(port)
# soc.listen(5)
# soc.bind((host, port))

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#soc.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
host = socket.gethostname()
port = 5522
print("The server hostname is: " + host)

soc.bind(('', port))
soc.listen(5)
x = 1

while True:
	client_sock, addr = soc.accept()
	print("Client (%s, %s) connected" % addr)

	while x == 1:
		data = client_sock.recv(8192)
		if data:
			msg = "Server Response: " + data.decode('utf-8')
			print("Recieved: " + data.decode('utf-8'))
			if data.decode('utf-8') == 'quit':
				print("Closing connection with: " + str(addr))
				client_sock.close()
				x = 0
			else:
				client_sock.send(msg.encode('utf-8'))
			# interface.run("controller")
		else:
			print("Connection Lost")
			client_sock.close()
	x = 1