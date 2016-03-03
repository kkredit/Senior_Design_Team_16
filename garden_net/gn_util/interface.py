class Interface:
	def __init__(self):
		pass

	def run(self, connection: str):
		if connection == 'website':
			print("Got a connection from the website on port 5510")
		elif connection == 'controller':
			print("Got a connection from the controller on port 5555")
		else:
			print("Not sure what happened")