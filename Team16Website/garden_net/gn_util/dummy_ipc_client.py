import socket

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5537
print("My local hostname: " + host)

try:
	#soc = socket.create_connection((host, port))
	soc = socket.create_connection(('gardenet.ddns.net', 5530))
	#soc = socket.create_connection(('153.106.112.199', port))
	#socket.create_connection((host, port))
except:
	print("Unable to connect")
	exit()

print('Connected to the local host for IPC communication on port ', port, '.')
msg = "STATE?"
soc.sendall(msg.encode('utf-8'))
soc.close()

print('Socket was closed.')
