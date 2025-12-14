from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from ..deps import get_db
from .. import models

router = APIRouter(prefix='/api/users')

@router.get('/')
def list_users(db: Session = Depends(get_db)):
    users = db.query(models.User).all()
    return [{'id': u.id, 'username': u.username} for u in users]

@router.get('/{username}')
def get_user_profile(username: str, db: Session = Depends(get_db)):
    u = db.query(models.User).filter(models.User.username == username).first()
    if not u:
        raise HTTPException(status_code=404)
    followers = db.query(models.Follow).filter(models.Follow.followee_id == u.id).count()
    following = db.query(models.Follow).filter(models.Follow.follower_id == u.id).count()
    return {'id': u.id, 'username': u.username, 'followers': followers, 'following': following}

@router.post('/{username}/follow')
def follow_user(username: str, db: Session = Depends(get_db)):
    target = db.query(models.User).filter(models.User.username == username).first()
    if not target:
        raise HTTPException(status_code=404)
    # naive follow by id for demo
    f = models.Follow(follower_id=1, followee_id=target.id)
    db.add(f)
    db.commit()
    return {'status': 'ok'}
