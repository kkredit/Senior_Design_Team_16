#!/Python34/python

# Import modules for CGI handling 
import cgi, cgitb

cgitb.enable()
# Create instance of FieldStorage 
data= cgi.FieldStorage(None, None, )

# Get data from fields
output = data.getvalue("checks")

# This will print to stdout for testing
print("Hello World!!!")
print(output)