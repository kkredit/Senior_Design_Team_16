import datetime
from pytz import timezone
from sqlalchemy import Column, Float, String, Integer, ForeignKey
from historical_database import HistoricalBase


class Historical_Database_Interface(HistoricalBase):
	__tablename__ = "reports"

	id = Column(Integer, primary_key=True)
	_date = Column(String(20))
	_long_ugly = Column(String(120))

	def __init__(self, long_ugly: str):
		self._long_ugly_string = long_ugly
		fmt = "%m-%d-%Y"
		now_time = datetime.datetime.now(timezone('US/Eastern'))
		time = now_time.strftime(fmt)
		#print(time)
		self._date = time
	def __str__(self):
		return str(self.date) + "\t" + str(self.long_ugly_string)

	@property
	def long_ugly_string(self):
		return self._long_ugly_string

	@property
	def date(self):
		return self._date

if __name__ == "__main__":
	it = Historical_Database_Interface("chaw")