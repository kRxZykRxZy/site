import asyncio
from .database import SessionLocal
from .models import PendingWrite, File
import time

async def worker_loop(poll_seconds: float = 0.5):
    while True:
        db = SessionLocal()
        try:
            pending = db.query(PendingWrite).filter(PendingWrite.processed == False).order_by(PendingWrite.created_at).all()
            for p in pending:
                f = db.query(File).filter(File.project_id == p.project_id, File.path == p.path).first()
                if not f:
                    f = File(project_id=p.project_id, path=p.path, content=p.content)
                    db.add(f)
                else:
                    f.content = p.content
                p.processed = True
                db.commit()
        except Exception as e:
            db.rollback()
        finally:
            db.close()
        await asyncio.sleep(poll_seconds)
