from fastapi import APIRouter, Depends, Request, Response, HTTPException, Form
from sqlalchemy.orm import Session
from passlib.context import CryptContext
from . import models
from .deps import get_db
from jose import jwt
import os

router = APIRouter(prefix='/api/auth')
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")
SECRET_KEY = os.getenv('SECRET_KEY', 'devsecret')

@router.post('/register')
def register(username: str = Form(...), password: str = Form(...), db: Session = Depends(get_db)):
    if db.query(models.User).filter(models.User.username == username).first():
        raise HTTPException(status_code=400, detail='username exists')
    u = models.User(username=username, password_hash=pwd_context.hash(password))
    db.add(u)
    db.commit()
    db.refresh(u)
    return {'id': u.id, 'username': u.username}

@router.post('/login')
def login(response: Response, username: str = Form(...), password: str = Form(...), db: Session = Depends(get_db)):
    u = db.query(models.User).filter(models.User.username == username).first()
    if not u or not pwd_context.verify(password, u.password_hash):
        raise HTTPException(status_code=400, detail='invalid credentials')
    # set simple secure cookie session (signed value optional)
    response.set_cookie('session_user', str(u.id), httponly=True)
    # return JWT as well
    token = jwt.encode({'sub': str(u.id)}, SECRET_KEY, algorithm='HS256')
    return {'access_token': token, 'token_type': 'bearer'}

@router.post('/logout')
def logout(response: Response):
    response.delete_cookie('session_user')
    return {'status': 'logged_out'}

@router.get('/me')
def me(request: Request, db: Session = Depends(get_db)):
    uid = request.cookies.get('session_user')
    if not uid:
        raise HTTPException(status_code=401, detail='not authenticated')
    u = db.query(models.User).get(int(uid))
    return {'id': u.id, 'username': u.username}
