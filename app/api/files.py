from fastapi import APIRouter, Depends, HTTPException, Form
from sqlalchemy.orm import Session
from ..deps import get_db
from .. import models

router = APIRouter(prefix='/api/projects')

@router.get('/{project_id}/files')
def list_files(project_id: int, db: Session = Depends(get_db)):
    files = db.query(models.File).filter(models.File.project_id == project_id).all()
    return [{'id': f.id, 'path': f.path, 'updated_at': f.updated_at.isoformat()} for f in files]

@router.get('/{project_id}/files/raw/{path:path}')
def get_file_raw(project_id: int, path: str, db: Session = Depends(get_db)):
    f = db.query(models.File).filter(models.File.project_id == project_id, models.File.path == path).first()
    if not f:
        raise HTTPException(status_code=404)
    return {'path': f.path, 'content': f.content}

@router.post('/{project_id}/files/enqueue')
def enqueue_write(project_id: int, path: str = Form(...), content: str = Form(...), db: Session = Depends(get_db)):
    pw = models.PendingWrite(project_id=project_id, path=path, content=content)
    db.add(pw)
    db.commit()
    return {'status': 'enqueued', 'id': pw.id}
