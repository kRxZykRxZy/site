import asyncio
from .queue_worker import worker_loop

def run_worker():
    loop = asyncio.get_event_loop()
    loop.run_until_complete(worker_loop())

if __name__ == '__main__':
    run_worker()
