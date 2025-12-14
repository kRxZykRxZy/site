from fastapi import FastAPI, Request
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from .database import engine, SessionLocal
from . import models
from .auth import router as auth_router
from .api import users as users_api, projects as projects_api, files as files_api
import asyncio
from .queue_worker import worker_loop

models.Base.metadata.create_all(bind=engine)

app = FastAPI()
templates = Jinja2Templates(directory='templates')
app.mount('/static', StaticFiles(directory='static'), name='static')

app.include_router(auth_router)
app.include_router(users_api.router)
app.include_router(projects_api.router)
app.include_router(files_api.router)

@app.on_event('startup')
async def startup():
    # start background queue worker task
    loop = asyncio.get_event_loop()
    loop.create_task(worker_loop(0.5))

@app.get('/')
def index(request: Request):
    return templates.TemplateResponse('index.html', {'request': request})

@app.get('/editor/{project_id}')
def editor(request: Request, project_id: int):
    return templates.TemplateResponse('editor.html', {'request': request, 'project_id': project_id})

@app.get('/projects/{project_id}')
def project_view(request: Request, project_id: int):
    return templates.TemplateResponse('project_view.html', {'request': request, 'project_id': project_id})

@app.get('/users/{username}')
def user_profile(request: Request, username: str):
    return templates.TemplateResponse('user_profile.html', {'request': request, 'username': username})
