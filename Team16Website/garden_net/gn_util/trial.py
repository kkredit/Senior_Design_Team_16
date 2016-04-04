import datetime

current = datetime.datetime.now()

f = open('/var/www/Team16Website/garden_net/gn_util/trial.txt', 'w')
f.write(str(current))
f.close()
#print(current)
