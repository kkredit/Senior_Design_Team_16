"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	test_server.py

	This script is used by crontab to run every 5 minutes in order to
	connect to the server and receive a message back from it in order
	to assure that it is still up and running correctly. If it is not,
	it runs a bash script in order to find the PID of the sockets that
	the server is hogging the resources on and kill it. That bash script
	then restarts the server. Once all of that is complete, this script
	rights the log file the time that the server restarted
"""
# Third party Imports
import socket
import subprocess
import datetime
from pytz import timezone

# Create the socket
soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = socket.gethostname()
port = 5539
#print("My local hostname: " + host)
"""
	A function to kill the server and free up the resources it
	has tied down and restart the server from a bash script. It then
	writes a log file with the current time that this function
	was executed to keep track of failures.
"""
def kill():
<<<<<<< HEAD
	# Call the bashscript to restart the server
	subprocess.call(['./server_script.sh'])
	# get the current time and date
=======
	print("Killing and restarting the server")
	subprocess.call(['/var/www/Team16Website/garden_net/gn_util/server_script.sh'])
>>>>>>> c4c25c4e0881f4c0f3b7fdbba9466a1de1fec849
	fmt = "%Y-%m-%d %H:%M:%S %Z%z"
	now_time = datetime.datetime.now(timezone('US/Eastern'))
	time = now_time.strftime(fmt)
	data = "\nRestarted the server: " + time
	# append it to the log file
	f = open('/var/www/Team16Website/garden_net/gn_util/server_restarts.txt', 'a')
	f.write(data)
	f.close()
	exit()

# first try to connect. If we cannot except it and call the kill function
try:
	soc = socket.create_connection(('localhost', port))
except:
	kill()

print('Connected to the local host for IPC communication on port ', port, '.')

# Try to receive the message back from the server with a timeout of 10 seconds.
try:
	soc.settimeout(10)
	data = soc.recv(1024)
	# if we got our response, go ahead and exit. Our work here is done
	if data:
		soc.close()
		exit()
	# else kill it
	else:
		kill()
# we gave it our best shot. Go ahead and kill it now
except:
	kill()


soc.close()

#print('Socket was closed.')
