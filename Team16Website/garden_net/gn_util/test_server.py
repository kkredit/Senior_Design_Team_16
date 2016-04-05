import socket
import subprocess

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5539
print("My local hostname: " + host)

try:
	soc = socket.create_connection(('localhost', port))
except:
	print("Killing and restarting the server")
	subprocess.call(['./server_script.sh'])
	exit()

print('Connected to the local host for IPC communication on port ', port, '.')

try:
	data = soc.recv(1024)
except:
	print("Killing and restarting the server")
	subprocess.call(['./server_script.sh'])
	exit()


soc.close()

print('Socket was closed.')