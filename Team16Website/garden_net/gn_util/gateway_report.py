"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	gateway_report.py:

	This script is used to decode the string that is sent by the gateway. It
	determines what opcode was sent and takes the actions that are needed for
	each opcode. See alert_engine_protocol.txt for details on the opcodes.

"""
# Local Imports
from historical_database_interface import Historical_Database_Interface
from historical_database import Historical_Database
from send_email import Alert

class Report():
	"""
		Explicit constructor for the Report class that checks the opcode of the
		message sent and determines the actions taken.

		@:param value, str that is passed in that the gateway sends to the server
	"""
	def __init__(self, value: str):
		# get the opcode
		self._opcode = value.split('%')[0]
		# open the alert settings file and read in each line to an array
		f = open('alerts.txt', 'r')
		alert_settings = []
		for line in f:
			alert_settings.append(line)
		# if the opcode is 00 we just have a generic report for historical data
		if self._opcode == "00":
			# create a database instance and add the event to the database
			db = Historical_Database(False)
			new_event = Historical_Database_Interface(value)
			db.add_row(new_event)
			# get all of the events
			all_rows = db.get_all()
			#print(all_rows)
			to_write = ""
			for item in all_rows:
				# Call write report to get the string methods so that the website can
				# decipher all of the data being written in the file
				it = WriteReport(item)
				to_write += str(it) + "\n\n"
			# write the historical data file with all the events in the database
			f = open("historical_data.txt", "w")
			f.write(to_write)
			f.close()
		# if opcode is 01 we have an alert for a valve being stuck on or off
		elif self._opcode == "01":
			# get the node and valve where the issue is happening
			node = value.split('%')[1]
			status = value.split('%')[2]
			currentFlowRate = value.split('%')[3]
			# determine if stuck on or off
			if status == "5":
				status_text = "stuck on"
			else:
				status_text = "stuck off"
			# Set the text for the Alert email
			TEXT = "Hello GardeNet User," \
							"\n\nIt seems that your gateway has notified the alert system that node " + node + \
							" is " + status_text + " and is recording a flow rate of: " + currentFlowRate + \
							".\n\nRegards, \nGardeNet"
			#check if the user has selected to be notified for this alert
			if str(alert_settings[0]).split('\t')[2].split('\n')[0].upper() == "TRUE":
				# send the email if the user has selected this alert
				Alert(TEXT)
		# if opcode is 02 we have an alert that the mesh network has gone down
		elif self._opcode == "02":
			mesh_status = value.split('%')[1]
			mesh_status_text = ""
			if mesh_status == "0":
				mesh_status_text = "All nodes are correctly functioning."
			elif mesh_status == "1":
				mesh_status_text = "The mesh did not begin correctly."
			elif mesh_status == "2":
				mesh_status_text = "Some of the nodes are reported to be down."
			elif mesh_status == "3":
				mesh_status_text = "All of the nodes have been reported as down."
			# set the text for the email
			TEXT = "Hello GardeNet User," \
				"\n\nYour gateway has reported a radio network issue. The message received from " \
				   "the gateway was:\n\n" + mesh_status_text + "\n\nRegards, \nGardeNet"
			print(TEXT)
			# if the user has signed up for this alert
			if str(alert_settings[3]).split('\t')[2].split('\n')[0].upper() == "TRUE":
				# send the email
				Alert(TEXT)
		# if opcode is 03 we have an alert that the gateway has reset itself
		elif self._opcode == "03":
			# set the text for the email
			TEXT = "Hello GardeNet User," \
				"\n\nYour gateway has reset itself.\n\nRegards, \nGardeNet"
			# if the user has signed up for this alert
			if str(alert_settings[2]).split('\t')[2].split('\n')[0].upper() == "TRUE":
				# send the email
				Alert(TEXT)
		# if the opcode is 04 we have an alert for a voltage level issue
		elif self._opcode == "04":
			# get the node and valve were the issue is occuring
			node = value.split('%')[1]
			voltageState = value.split('%')[2]
			# determine the voltage level issue
			if voltageState == "1":
				voltageState_text = "low"
			else:
				voltageState_text = "high"
			# set the text for the email
			TEXT = "Hello GardeNet User," \
				"\n\nYour gateway has reported that node " + node +" has a " + voltageState_text + \
					" voltage level.\n\nRegards, \nGardeNet"
			# if the user has signed up for this alert
			if str(alert_settings[1]).split('\t')[2].split('\n')[0].upper() == "TRUE":
				# send the email
				Alert(TEXT)
		elif self._opcode == "05":
			power_file = open("garden_power_status.txt", "w")
			power_status = value.split('%')[1]
			if power_status == "0":
				power_file.write("OFF")
			elif power_status == "1":
				power_file.write("ON")
"""
	This class is used to take the information from a general report that the
	gateway sends and format the data so that the website can read the file
	and display the data for the user.
"""
class WriteReport:
	"""
		Explicit constructor of the string that the gateway sends and we save in the
		database above.

		@:param string, str that is the message the gateway sent
	"""
	def __init__(self, string: str):
		# get the date
		self._date = str(string).split('\t')[0]
		# get all of the data up to { because that is global garden data
		temp = str(string).split('\t')[1].split('{')
		global_gardern = temp[0]
		self._opcode = global_gardern.split('%')[0]
		self._percentMeshUp = global_gardern.split('%')[1]
		self._percent3GUpTime = global_gardern.split('%')[2]
		# create a a list of nodes and dynamically assign as many nodes
		# as are present in the string
		self._node_list = []
		i = 1
		while True:
			try:
				it = temp[i]
				# format the node information
				new_node = Node(it)
				# append it to our node list
				self._node_list.append(new_node)
			except:
				break
			i += 1
	"""
		The string method that formats the data so that the website can read it when
		it is written to the file. It calls on the Node class's string method in order
		to dynamically write the number of nodes present in the string sent by the gateway
		in correct formatting.
	"""
	def __str__(self):
		temp = "date: " + self.date + "\n\topcode: " + self.opcode + "\n\tpercentMeshUp: " + self.percentMeshUp + "\n\tpercent3GUp: " \
			   + self._percent3GUpTime
		for node in self.node_list:
			temp += str(node)
		return temp + "\n"
	######### GETTERS ############
	@property
	def date(self):
		return self._date

	@property
	def opcode(self):
		return self._opcode

	@property
	def percentMeshUp(self):
		return self._percentMeshUp

	@property
	def percent3GUpTime(self):
		return self._percent3GUpTime

	@property
	def node_list(self):
		return self._node_list
	##############################

"""
	This class is used to dynamically get all of the data for a dynamic
	number of nodes present in the string and then properly format the
	data so that it may be written to the file that the website can read
"""
class Node():
	"""
		Explicit constructor of the brokendown string that the gateway sends and
		we save in the database above.

		@:param string, str that is a substring of the message the gateway sends
						containing only the Node and Valve parts
	"""
	def __init__(self, value: str):
		# get all of the data up to the [
		self._nodeId = int(value.split('%')[1])
		self._meshState = int(value.split('%')[2])
		self._percentNodeAwake = float(value.split('%')[3])
		self._accumulatedFlow = float(value.split('%')[4])
		# detemine if the flow meter is maxed out
		if int(value.split('%')[5]) == 0:
			self._maxedOutFlowMeter = False
		else:
			self._maxedOutFlowMeter = True
		temp = value.split('[')
		# dynamically discover how many valves are present in the string
		self._valve_list = []
		i = 1
		while True:
			try:
				it = temp[i]
				# call on the valve class to get the valve data and reformat it
				new_valve = Valve(it)
				# append it to our list
				self._valve_list.append(new_valve)
			except:
				break
			i += 1

	"""
		The string method that formats the data so that the website can read it when
		it is written to the file. It calls on the Valve class's string method in order
		to dynamically write the number of valves present in the string sent by the gateway
		in correct formatting.
	"""
	def __str__(self):
		temp = "\n\t\tnode: " + str(self.nodeID) + " meshState: " + str(self.meshState) + " percentNodeAwake: " \
			   + str(self.percentNodeAwake) + " accumulatedFlow: " + str(self.accumulatedFlow)
		for valve in self.valve_list:
			temp += str(valve)
		return temp
	######### GETTERS ############
	@property
	def nodeID(self):
		return self._nodeId

	@property
	def meshState(self):
		return self._meshState

	@property
	def percentNodeAwake(self):
		return self._percentNodeAwake

	@property
	def accumulatedFlow(self):
		return self._accumulatedFlow

	@property
	def valve_list(self):
		return self._valve_list
	##############################

"""
	This class is used to dynamically get all of the data for a dynamic
	number of valves present in the string and then properly format the
	data so that it may be written to the file that the website can read
"""
class Valve():
	"""
		Explicit constructor of the brokendown string that the gateway sends and
		we save in the database above.

		@:param string, str that is a substring of the message the gateway sends
						containing only the Valve parts
	"""
	def __init__(self, value: str):
		# get the data
		self._valveNum = value.split('%')[1]
		self._totalWateringTime = value.split('%')[2].split("]")[0]
		#print(self._valveNum)
		#print(self._totalWateringTime)

	"""
		The string method that formats the data so that the website can read it when
		it is written to the file.
	"""
	def __str__(self):
		return "\n\t\t\tvalve_num: " + str(self._valveNum) + " totalWateringTime: " + str(self._totalWateringTime)
	######### GETTERS ############
	@property
	def valveNum(self):
		return self._valveNum

	@property
	def totalWateringTime(self):
		return self._totalWateringTime
	##############################

if __name__ == "__main__":
	#it = Report("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
	#print(it)
	#r = Report("01%2%5%0.00")
	#t = Report("02")
	#e = Report("03")
	#p = Report("04%1%2")
	#print(it._opcode)
	#print(it._percentMeshUp)
	#print(it._percent3GUpTime)
	#it = Report("00%100.00%77.78%100.00%{%1%0%100.00%0.00%0%[%1%0.00]%[%2%0.00]%[%3%0.00]%}")
	#print(str(r))
	Report("05%0")