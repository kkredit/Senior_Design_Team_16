import datetime

now = datetime.datetime.now().ctime()

f = open('trial.txt', 'w')
f.write(now)
f.close()

#print(now)