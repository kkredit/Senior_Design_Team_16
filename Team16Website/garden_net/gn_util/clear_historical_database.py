"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	clear_historical_database.py:

	This is a script that was used during testing so
	that the database could be cleared quickly of the
	information not needed

"""
# Third party imports

from historical_database import Historical_Database

db = Historical_Database(False)

db.clear_database()