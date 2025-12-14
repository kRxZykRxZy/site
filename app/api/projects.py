from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from ..deps import get_db
from .. import models

router = APIRouter(prefix='/api/projects')

@router.post('/')
def create_project(name: str, db: Session = Depends(get_db)):
    p = models.Project(name=name, owner_id=1)
    db.add(p)
    db.commit()
    db.refresh(p)
    return {'id': p.id, 'name': p.name}

@router.get('/')
def list_projects(db: Session = Depends(get_db)):
    ps = db.query(models.Project).all()
    return [{'id': p.id, 'name': p.name, 'owner_id': p.owner_id} for p in ps]

@router.get('/{project_id}')
def get_project(project_id: int, db: Session = Depends(get_db)):
    p = db.query(models.Project).get(project_id)
    if not p:
        raise HTTPException(status_code=404)
    files = db.query(models.File).filter(models.File.project_id == p.id).all()
    return {'id': p.id, 'name': p.name, 'files': [{'path': f.path, 'id': f.id} for f in files]}
