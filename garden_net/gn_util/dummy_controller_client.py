import socket

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5520
print(host)


try:
	#soc.connect((host, port))
	soc = socket.create_connection(('153.106.112.199', port))
	#socket.create_connection((host, port))
except:
	print("Unable to connect")
	exit()

print('Connected to remote host. You can start sending messages')

while 1:
	msg = input("Client: ")
	#print(msg)
	new = msg.encode('utf-8')
	soc.send(new)
	data = soc.recv(4096)
	if data:
		print(data.decode())
	else:
		print("idk what happened")
		soc.close()


