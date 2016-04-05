import socket
import subprocess

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5539
print("My local hostname: " + host)

def kill():
	print("Killing and restarting the server")
	subprocess.call(['./server_script.sh'])
	exit()

try:
	soc = socket.create_connection(('localhost', port))
except:
	kill()

print('Connected to the local host for IPC communication on port ', port, '.')

try:
	soc.settimeout(10)
	data = soc.recv(1024)
	if data:
		print("Sever is good")
	else:
		kill()
except:
	kill()


soc.close()

print('Socket was closed.')
