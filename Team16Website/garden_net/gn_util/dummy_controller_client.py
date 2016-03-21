import socket

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5531
print("My local hostname: " + host)


try:
	#soc = socket.create_connection((host, port))
	soc = socket.create_connection(('gardenet.ddns.net', port))
	#soc = socket.create_connection(('153.106.112.199', port))
	#socket.create_connection((host, port))
except:
	print("Unable to connect")
	exit()

print('Connected to remote host. You can start sending messages')
print('Type quit to close the connection with the server')

i = 0
x = 1
while x == 1:
	if i == 0:
		try:
			welcome_message = soc.recv(8192)
			i = 1
		except:
			continue
	else:
		msg = input("Client: ")
		#print(msg)
		new = msg.encode('utf-8')
		soc.send(new)
		try:
			data = soc.recv(8192)
			if data:
				print(data.decode())
			else:
				print("Connection with server closed")
				soc.close()
				x = 0
		except:
			continue

