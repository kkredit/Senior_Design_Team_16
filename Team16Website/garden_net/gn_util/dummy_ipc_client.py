import socket

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5538
print("My local hostname: " + host)


try:
	#soc = socket.create_connection((host, port))
	soc = socket.create_connection(('localhost', port))
	#soc = socket.create_connection(('153.106.112.199', port))
	#socket.create_connection((host, port))
except:
	print("Unable to connect")
	exit()

print('Connected to remote host. You can start sending messages')
print('Type quit to close the connection with the server')

#msg = input("Client: ")
#print(msg)
msg = "Ello Server!"
new = msg.encode('utf-8')
soc.send(new)
soc.close()