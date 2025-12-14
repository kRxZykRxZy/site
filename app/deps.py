from .database import SessionLocal
from fastapi import Depends, HTTPException
from sqlalchemy.orm import Session
from starlette.requests import Request
from . import models

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

def get_user_by_session(request: Request, db: Session = Depends(get_db)):
    user_id = request.session.get('user_id') if hasattr(request, 'session') else None
    if user_id:
        return db.query(models.User).filter(models.User.id == user_id).first()
    return None
