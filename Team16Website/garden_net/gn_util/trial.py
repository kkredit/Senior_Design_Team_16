from datetime import datetime
from pytz import timezone
fmt = "%Y-%m-%d %H:%M:%S %Z%z"
now_time = datetime.now(timezone('US/Eastern'))
print(now_time.strftime(fmt))



# current = datetime.datetime.now()
#
# f = open('/var/www/Team16Website/garden_net/gn_util/trial.txt', 'w')
# f.write(str(current))
# f.close()
# #print(current)
