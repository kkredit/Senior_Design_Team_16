#!/usr/bin/env python3

import sys

from interface import Interface

interface = Interface()

try:
	data = sys.argv[1]
	f = open('ipc_file.txt', 'w')
	try:
		f.write(str(data))
		print("Wrote: ", str(data))
		f.close()
	except:
		print(sys.exc_info())

	interface.run('website')
except:
	print("ERROR: ", sys.exc_info())
	print("The data that was received was: ", str(data))


