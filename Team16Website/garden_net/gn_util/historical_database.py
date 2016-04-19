from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import and_

from base_historical import HistoricalBase, HistoricalMeta
from historical_database_interface import Historical_Database_Interface
from gateway_report import Garden

class Historical_Database:
	"""
		A explicit constructor that creates the connection to the database

		@:param self: passes ourselves in as an object
		@:param bool new: a boolean to tell whether to start a new database or use the already saved one

	"""
	def __init__(self, new: bool):
		# if True, create a new database
		if new:
			self.engine = create_engine('sqlite://')
		# else, extend off of the already existing database
		else:
			self.engine = create_engine('sqlite:///historical_database.db')

		# Set the Meta, Base
		self.meta = HistoricalMeta
		self.base = HistoricalBase
		# Bind the metadata and base to the engine and create it
		self.base.metadata.bind = self.engine
		self.base.metadata.create_all()

		# Start the session bound to our engine
		create_session = sessionmaker()
		create_session.configure(bind=self.engine)
		self.session = create_session()


	"""
		A function to clear the database

		@:param: self: passes ourself in to delete all items in the database
	"""
	def clear_database(self):
		self.meta.drop_all()
		self.meta.create_all()
		self.session.commit()

	def add_row(self, row: Historical_Database_Interface):
		if self.session.query(Historical_Database_Interface).filter(Historical_Database_Interface.date == row.date).all():
			raise ValueError("Already an event for that date")
		else:
			self.session.add(row)
			self.session.commit()

	def get_all(self):
		data_list = self.session.query(Historical_Database_Interface).all()
		for item in data_list:
			new = Garden(str(item).split("\t")[1])
			print(new)


if __name__ == "__main__":
	pass