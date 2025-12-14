from sqlalchemy import Column, Integer, String, Text, ForeignKey, DateTime, Boolean
from sqlalchemy.orm import relationship, declarative_base
import datetime

Base = declarative_base()

class User(Base):
    __tablename__ = 'users'
    id = Column(Integer, primary_key=True)
    username = Column(String(128), unique=True, index=True, nullable=False)
    password_hash = Column(String(256), nullable=False)
    created_at = Column(DateTime, default=datetime.datetime.utcnow)
    followers = Column(Integer, default=0)

class Project(Base):
    __tablename__ = 'projects'
    id = Column(Integer, primary_key=True)
    name = Column(String(256), nullable=False)
    owner_id = Column(Integer, ForeignKey('users.id'))
    created_at = Column(DateTime, default=datetime.datetime.utcnow)
    files = relationship('File', back_populates='project')

class File(Base):
    __tablename__ = 'files'
    id = Column(Integer, primary_key=True)
    project_id = Column(Integer, ForeignKey('projects.id'))
    path = Column(String(1024), nullable=False, index=True)
    content = Column(Text, default='')
    updated_at = Column(DateTime, default=datetime.datetime.utcnow)
    project = relationship('Project', back_populates='files')

class PendingWrite(Base):
    __tablename__ = 'pending_writes'
    id = Column(Integer, primary_key=True)
    project_id = Column(Integer, nullable=False, index=True)
    path = Column(String(1024), nullable=False)
    content = Column(Text, nullable=False)
    created_at = Column(DateTime, default=datetime.datetime.utcnow)
    processed = Column(Boolean, default=False, index=True)

class Follow(Base):
    __tablename__ = 'follows'
    id = Column(Integer, primary_key=True)
    follower_id = Column(Integer, nullable=False)
    followee_id = Column(Integer, nullable=False)
    created_at = Column(DateTime, default=datetime.datetime.utcnow)
