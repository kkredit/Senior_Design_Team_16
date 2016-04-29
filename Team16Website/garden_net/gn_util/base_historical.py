"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	base_historical.py:

	This is a script to declare the HistoricalBase for the
	Historical_Database that allows classes to extend the same
	HistoricalBase in order that the classes will be able to
	directly interact with each other. See SQLAlchemey's
	documentation for more information.

"""
# Third party imports
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import MetaData

HistoricalMeta = MetaData()
HistoricalBase = declarative_base(metadata=HistoricalMeta)