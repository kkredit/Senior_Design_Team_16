import socket
import subprocess
import datetime
from pytz import timezone


soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5539
print("My local hostname: " + host)

def kill():
	print("Killing and restarting the server")
	subprocess.call(['/var/www/Team16Website/garden_net/gn_util/server_script.sh'])
	fmt = "%Y-%m-%d %H:%M:%S %Z%z"
	now_time = datetime.datetime.now(timezone('US/Eastern'))
	time = now_time.strftime(fmt)
	data = "\nRestarted the server: " + time
	f = open('/var/www/Team16Website/garden_net/gn_util/server_restarts.txt', 'a')
	f.write(data)
	f.close()
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
