"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	base.py:

	This is a script to declare the Base for the Database
	that allows classes to extend the same Base in order
	that the classes will be able to directly interact with
	each other. See SQLAlchemey's documentation for more
	information.

"""
# Third party imports
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import MetaData

Meta = MetaData()
Base = declarative_base(metadata=Meta)