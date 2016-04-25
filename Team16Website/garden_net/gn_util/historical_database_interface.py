import datetime
from pytz import timezone
from sqlalchemy import Column, Float, String, Integer, ForeignKey
from base_historical import HistoricalBase


class Historical_Database_Interface(HistoricalBase):
	__tablename__ = "reports"

	id = Column(Integer, primary_key=True)
	_date = Column(String(20))
	_long_ugly = Column(String(120))

	def __init__(self, long_ugly_string: str):
		self._long_ugly = long_ugly_string
		fmt = "%m-%d-%Y"
		now_time = datetime.datetime.now(timezone('US/Eastern'))
		time = now_time.strftime(fmt)
		#print(time)
		self._date = time

	@property
	def long_ugly(self):
		return self._long_ugly

	def __str__(self):
		return str(self.date) + "\t" + str(self.long_ugly)

	@property
	def date(self):
		return self._date

if __name__ == "__main__":
	it = Historical_Database_Interface("00%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")
	print(str(it).split('\t')[1])