from JSON import JSON_Interface

class Interface:
	def __init__(self):
		pass
		self.json = JSON_Interface()
		self.website_file = "test_input.txt"
		self.test_basic_functionality = "functionality_test.txt"

	def run(self, connection: str):
		if connection == 'website':
			f = open('functionality_test.txt', 'r')
			input = f.readline()
			print("Got a connection from the website on port 5510")
		elif connection == 'controller':
			print("Got a connection from the controller on port 5555")
		else:
			print("Not sure what happened")