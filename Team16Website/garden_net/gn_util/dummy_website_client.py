import socket

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#host = socket.gethostbyname('gardenet')
port = 5510
#print(host)

soc = socket.create_connection(('153.106.112.199', port))