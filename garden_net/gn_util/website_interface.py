import socket
import select

from interface import Interface

interface = Interface()

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
host = socket.gethostname()
port = 80

soc.bind((host, port))

soc.listen(5)

while True:
	client_sock, addr = soc.accept()
	print("Got a connection from %s" % str(addr))
	interface.run("website")