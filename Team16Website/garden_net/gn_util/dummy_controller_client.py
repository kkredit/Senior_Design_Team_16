import socket

def start_connection(sock: socket):
	try:
		port = 5530
		#soc = socket.create_connection((host, port))
		sock = socket.create_connection(('gardenet.ddns.net', port))
		#soc = socket.create_connection(('153.106.112.199', port))
		#socket.create_connection((host, port))
		host = socket.gethostname()
		print("My local hostname: " + host)
		return sock
	except:
		print("Unable to connect")
		exit()
soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc = start_connection(soc)
host = socket.gethostname()
port = 5530
#print("My local hostname: " + host)


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
	try:
		data = soc.recv(8192)
		if data:
			print(data.decode())
		else:
			print("Connection with server closed")
			soc.close()
			start_connection(soc)
			#x = 0
	except:
		continue


