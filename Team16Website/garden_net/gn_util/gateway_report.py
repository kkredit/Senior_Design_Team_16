from historical_database_interface import Historical_Database_Interface
from historical_database import Historical_Database

from send_email import Alert

class Report():

	def __init__(self, value: str):
		self._opcode = value.split('%')[0]
		if self._opcode == "00":
			db = Historical_Database(False)
			#print(value)
			new_event = Historical_Database_Interface(value)


			#db.clear_database()


			db.add_row(new_event)
			all_rows = db.get_all()
			#print(all_rows)
			to_write = ""
			for item in all_rows:
				it = WriteReport(item)
				to_write += str(it) + "\n\n"

			f = open("historical_data.txt", "w")
			f.write(to_write)
			f.close()
		elif self._opcode == "01":
			node = value.split('%')[1]
			status = value.split('%')[2]
			currentFlowRate = value.split('%')[3]
			if status == "5":
				status_text = "stuck on"
			else:
				status_text = "stuck off"
			TEXT = "Hello GardeNet User," \
							"\n\nIt seems that your gateway has notified the alert system that node " + node + \
							" is " + status_text + " and is recording a flow rate of: " + currentFlowRate + \
							".\n\nRegards, \nGardeNet"
			Alert(TEXT)
		elif self._opcode == "02":
			TEXT = "Hello GardeNet User," \
				"\n\nYour gateway has reported that the radio network has gone down and is not " \
							"correctly communicating with the nodes.\n\nRegards, \nGardeNet"
			Alert(TEXT)
		elif self._opcode == "03":
			TEXT = "Hello GardeNet User," \
				"\n\nYour gateway has reset itself.\n\nRegards, \nGardeNet"
			Alert(TEXT)
		elif self._opcode == "04":
			node = value.split('%')[1]
			voltageState = value.split('%')[2]
			if voltageState == "1":
				voltageState_text = "low"
			else:
				voltageState_text = "high"
			TEXT = "Hello GardeNet User," \
				"\n\nYour gateway has reported that node " + node +" has a " + voltageState_text + \
					" voltage level.\n\nRegards, \nGardeNet"
			Alert(TEXT)

class WriteReport:
	def __init__(self, string: str):
		self._date = str(string).split('\t')[0]
		temp = str(string).split('\t')[1].split('{')
		global_gardern = temp[0]
		self._opcode = global_gardern.split('%')[0]
		self._percentMeshUp = global_gardern.split('%')[1]
		self._percent3GUpTime = global_gardern.split('%')[2]
		self._node_list = []
		i = 1
		while True:
			try:
				it = temp[i]
				new_node = Node(it)
				self._node_list.append(new_node)
			except:
				break
			i += 1

	def __str__(self):
		temp = "date: " + self.date + "\n\topcode: " + self.opcode + "\n\tpercentMeshUp: " + self.percentMeshUp + "\n\tpercent3GUp: " \
			   + self._percent3GUpTime
		for node in self.node_list:
			temp += str(node)
		return temp + "\n"

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

class Node():
	def __init__(self, value: str):
		self._nodeId = int(value.split('%')[1])
		self._meshState = int(value.split('%')[2])
		self._percentNodeAwake = float(value.split('%')[3])
		self._accumulatedFlow = float(value.split('%')[4])
		if int(value.split('%')[5]) == 0:
			self._maxedOutFlowMeter = False
		else:
			self._maxedOutFlowMeter = True
		temp = value.split('[')
		self._valve_list = []
		i = 1
		while True:
			try:
				it = temp[i]
				new_valve = Valve(it)
				self._valve_list.append(new_valve)
			except:
				break
			i += 1



	def __str__(self):
		temp = "\n\t\tnode: " + str(self.nodeID) + " meshState: " + str(self.meshState) + " percentNodeAwake: " \
			   + str(self.percentNodeAwake) + " accumulatedFlow: " + str(self.accumulatedFlow)
		for valve in self.valve_list:
			temp += str(valve)
		return temp
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


class Valve():
	def __init__(self, value: str):
		self._valveNum = int(value.split('%')[1])
		self._totalWateringTime = int(value.split('%')[2])
		#print(self._valveNum)
		#print(self._totalWateringTime)

	def __str__(self):
		return "\n\t\t\tvalve_num: " + str(self.valveNum) + " totalWateringTime: " + str(self._totalWateringTime)

	@property
	def valveNum(self):
		return self._valveNum

	@property
	def totalWateringTime(self):
		return self._totalWateringTime

if __name__ == "__main__":
	it = Report("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
	#print(it)
	r = Report("01%2%5%0.00")
	t = Report("02")
	e = Report("03")
	p = Report("04%1%2")
	#print(it._opcode)
	#print(it._percentMeshUp)
	#print(it._percent3GUpTime)