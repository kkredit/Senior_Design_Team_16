from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import MetaData

Meta = MetaData()
Base = declarative_base(metadata=Meta)