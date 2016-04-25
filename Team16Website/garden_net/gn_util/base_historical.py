from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import MetaData

HistoricalMeta = MetaData()
HistoricalBase = declarative_base(metadata=HistoricalMeta)