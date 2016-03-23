import sys, json

# Load the data that PHP sent us
try:
    data = sys.argv[1]
except:
    print "ERROR"
    sys.exit(1)

# Generate some data to send to PHP
result = {'status': 'Yes!'}

# Send it to stdout (to PHP)
if data == "true" :
	print "The data came back true"
elif data == "false":
	print "The data came back false"
else : 
	print data