import datetime

current = datetime.datetime.now()

f = open('trial.txt', 'w')
f.write(str(current))
f.close()
#print(current)