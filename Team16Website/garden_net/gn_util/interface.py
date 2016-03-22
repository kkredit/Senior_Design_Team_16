import socket

from JSON import JSON_Interface

class Interface:
	def __init__(self):
		pass
		self.json = JSON_Interface()
		self.website_file = "test_input.txt"
		self.test_basic_functionality = "functionality_test.txt"

	def run(self, connection: str):
		if connection == 'website':
			soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			host = socket.gethostname()
			port = 5539
			print("My local hostname: " + host)
			try:
				#soc = socket.create_connection((host, port))
				soc = socket.create_connection(('localhost', port))
				#soc = socket.create_connection(('153.106.112.199', port))
				#socket.create_connection((host, port))
				print('Connected to the local host for IPC communication on port ', port, '.')
				soc.close()
				print('Socket was closed.')
			except:
				print("Unable to connect")
				exit()

		elif connection == 'controller':
			print("Got a connection from the controller on port 5555")
		else:
			print("Not sure what happened")