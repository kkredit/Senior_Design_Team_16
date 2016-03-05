import socket
import json
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((socket.gethostname(), 80))
s.listen(5)


while 1:
	# try:
	ns, na = s.accept()
	data = ns.recv(8192)
	# except:
	# 	ns.close()
	# 	s.close()
	# 	break

	data = json.loads(data)
	print(data)