from sqlalchemy import Column, Float, String, Integer, ForeignKey

from base_historical import HistoricalBase

class Garden():

	def __init__(self, value: str):
		temp = value.split('{')
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
		#for node in self._node_list
		#print(self._node_list)

	def __str__(self):
		temp = "opcode: " + self.opcode + "\npercentMeshUp: " + self.percentMeshUp + "\npercent3GUp: " \
			   + self._percent3GUpTime
		for node in self.node_list:
			temp += str(node)
		return temp

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
		temp = "\n\tnode: " + str(self.nodeID) + " meshState: " + str(self.meshState) + " percentNodeAwake: " \
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
		return "\n\t\tvalve_num: " + str(self.valveNum) + " totalWateringTime: " + str(self._totalWateringTime)

	@property
	def valveNum(self):
		return self._valveNum

	@property
	def totalWateringTime(self):
		return self._totalWateringTime

if __name__ == "__main__":
	it = Gateway_Report("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
	print(it)
	#print(it._opcode)
	#print(it._percentMeshUp)
	#print(it._percent3GUpTime)