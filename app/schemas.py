from pydantic import BaseModel
from typing import Optional

class UserCreate(BaseModel):
    username: str
    password: str

class UserOut(BaseModel):
    id: int
    username: str

    class Config:
        orm_mode = True

class ProjectCreate(BaseModel):
    name: str

class FileWrite(BaseModel):
    path: str
    content: str
