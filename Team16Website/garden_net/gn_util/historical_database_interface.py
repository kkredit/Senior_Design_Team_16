"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	historical_database_interface.py:

	This script allows for the interactions between the reports sent from the gateway and
	the database by saving a date and the string of the report that the gateway sends.

	The Historical_Database_Interface class also creates a new row in the 'reports' table.
	It adds an id to each row.

	@_date: Class variable to track the date the report was received as well as add a column in
				the 'report' table of type String
	@_stop_time: Class variable to track the str of the report sent by the gateway as well as add
				a column in the 'reports' table of type String
"""
# Third Party Imports
import datetime
from datetime import timedelta
from pytz import timezone
from sqlalchemy import Column, Float, String, Integer, ForeignKey

# Local Imports
from base_historical import HistoricalBase


class Historical_Database_Interface(HistoricalBase):
	__tablename__ = "reports"

	id = Column(Integer, primary_key=True)
	_date = Column(String(20))
	_long_ugly = Column(String(120))
	"""
		An Explicit Constructor for the Historical_Database_Interface class

		@:param long_ugly_string, str of the report received from the gateway
	"""
	def __init__(self, long_ugly_string: str):
		self._long_ugly = long_ugly_string
		fmt = "%m-%d-%Y"
		current_hour_fmt = "%H"
		current_minute_fmt = "%M"
		current_time = datetime.datetime.now(timezone('US/Eastern'))
		current_hour = int(current_time.strftime(current_hour_fmt))
		print(current_hour)
		current_minute = int(current_time.strftime(current_minute_fmt))
		# between 12 and 12:05
		if current_hour == 0 and current_minute < 5:
			now_time = datetime.datetime.now(timezone('US/Eastern')) - timedelta(days=1)
			time = now_time.strftime(fmt)
		else:
			now_time = datetime.datetime.now(timezone('US/Eastern'))
			time = now_time.strftime(fmt)
		self._date = time

	"""
		The string method for the class

		@:return a string of the date then a tab to be delimited on plus
					the long_ugly string of the report
	"""
	def __str__(self):
		return str(self.date) + "\t" + str(self.long_ugly)

	######## GETTERS ##########
	@property
	def long_ugly(self):
		return self._long_ugly

	@property
	def date(self):
		return self._date
	############################
if __name__ == "__main__":
	# it = Historical_Database_Interface("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
	# print(str(it).split('\t')[1])
	fmt = "%m-%d-%Y"
	current_hour_fmt = "%H"
	current_minute_fmt = "%M"
	current_time = datetime.datetime.now(timezone('US/Eastern')) + timedelta(hours=13)
	current_hour = int(current_time.strftime(current_hour_fmt))
	print(current_hour)
	current_minute = int(current_time.strftime(current_minute_fmt))
	# between 12 and 12:05
	if current_hour == 0 and current_minute < 5:
		now_time = datetime.datetime.now(timezone('US/Eastern')) - timedelta(days=1)
		time = now_time.strftime(fmt)
	else:
		now_time = datetime.datetime.now(timezone('US/Eastern'))
		time = now_time.strftime(fmt)

	print(time)

