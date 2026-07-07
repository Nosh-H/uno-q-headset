"""
 *  Copyright (C) 2026 Noah Haskell
 *  
 *  This program is free software: you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software Foundation, either version 3 of the
 *  License, or any later version.
 *  
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 *  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program. If
 *  not, see <https://www.gnu.org/licenses/>.
 *  
 *  File: main.py
 *  Author: Noah Haskell
 *  Description: "The Headset" is currently a prototype to automate the application of a cold compress on the wearer's eyes. 
 *  This is intended to treat eye itching and pain caused by allergies.
 *  This is the main python file for the Headset. Its key role is to run the app bridging the web interface with the microcontroller code.
"""
# app/main.py

from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

from bridge_client import BridgeClient
from controller import ProjectController
import uvicorn

BASE_DIR = Path(__file__).resolve().parent
bridge = BridgeClient()
controller = ProjectController(bridge)

# 1. Define the lifespan context manager
@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup logic (before yield)
    print("App is starting")
    controller.start()
    yield
    # Shutdown logic (after yield)
    print("App is shutting down")
    controller.shutdown()

# 2. Initialize the app with the lifespan
app = FastAPI(lifespan=lifespan)
# 3. Mount static files
app.mount("/static", StaticFiles(directory=BASE_DIR / "static"), name="static")

class LedCommand(BaseModel):
    on: bool

class ModeCommand(BaseModel):
    mode: int


class StateCommand(BaseModel):
    state: bool


@app.get("/")
def index():
    return FileResponse(BASE_DIR / "templates" / "index.html")


@app.get("/api/status")
def get_status():
    return controller.get_status()


@app.get("/api/state/{side}")
def get_state(side: int):
    try:
        return {"side": side, "state": controller.get_state(side)}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc))
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc))


@app.post("/api/state/{side}")
def set_state(side: int, command: StateCommand):
    try:
        return controller.set_state(side, command.state)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc))
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc))


@app.get("/api/temp/{side}")
def get_temp(side: int):
    try:
        return {"side": side, "temp_f": controller.get_temp(side)}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc))
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc))


@app.post("/api/led")
def set_led(command: LedCommand):
    try:
        return controller.set_led(command.on)
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc))


@app.post("/api/mode")
def set_mode(command: ModeCommand):
    try:
        return controller.set_mode(command.mode)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc))
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc))


@app.post("/api/stop")
def stop_all():
    try:
        return controller.emergency_stop()
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc))

# 4. Start the server (Crucial step to prevent exit code 0)
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)