import sys

from interface import Interface

interface = Interface()

try:
	data = sys.argv[1]
	f = open('ipc_file.txt', 'w')
	print("PENIS")
	f.write(data)
	f.close()
	interface.run('website')
except:
	print("ERROR")


