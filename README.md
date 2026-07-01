# Uno Q Headset

Uno Q Headset is a prototype control system for a wearable headset built around an Arduino Uno Q and a Python web app. The goal is to coordinate physical hardware behavior on the microcontroller with remote control from a browser over the local network.

This repository currently contains two main parts:

- A Python FastAPI app that exposes the web UI and API
- An Arduino/C++ sketch that owns the hardware-side behavior and RPC handlers

## What The Project Does

- Serves a lightweight web UI for controlling the headset
- Sends commands from the browser to Python
- Forwards those commands from Python to the microcontroller through the Arduino bridge
- Polls hardware state in the background so the UI can display live status
- Handles immediate hardware behavior on the MCU, where timing and safety matter most

## Architecture

The active control path is:

`browser -> python/main.py -> python/controller.py -> python/bridge_client.py -> sketch/sketch.ino`

More specifically:

- `python/main.py` defines the FastAPI app and the HTTP routes
- `python/controller.py` owns runtime state and background polling
- `python/bridge_client.py` wraps bridge calls to the microcontroller
- `sketch/sketch.ino` registers RPC functions with `Bridge.provide(...)`
- `python/arduino/` contains the vendored Arduino bridge support library

## Repository Layout

- `python/` - Python application code, API routes, controller logic, and bridge wrapper
- `sketch/` - Arduino Uno Q firmware and hardware logic
- `assets/` - Project assets
- `.agents/` - Assistant instructions and related guidance
- `README.md` - This overview

## Current UI And API

The current Python app serves an inline HTML page from `python/main.py`. It exposes API endpoints for:

- `GET /api/status`
- `GET /api/state/{side}`
- `POST /api/state/{side}`
- `GET /api/temp/{side}`
- `POST /api/led`
- `POST /api/mode`
- `POST /api/stop`

The page polls status periodically and sends commands back to the controller. The controller also mirrors the arm state and temperatures returned by the firmware bridge so the browser can read them back without talking to the MCU directly.

## Working On The Codebase

- Keep changes small and readable.
- Update the firmware RPC, `BridgeClient`, controller, route handler, and UI together when adding a new command.
- Treat `python/arduino/` as vendored framework code unless you are intentionally fixing the shared bridge library.
- Keep hardware timing and safety-sensitive behavior in the firmware, not in the web server.
- Validate inputs at the API boundary.
- Preserve thread safety in `ProjectController`, which owns the live state and polling loop.

## Notes

- This is still a prototype, so some bridge calls and hardware behaviors may be placeholders or incomplete.
- If a Python command does not line up with a firmware RPC, update both sides together instead of assuming one side is correct.
- The root README is intentionally focused on orientation. The deeper implementation details belong in code comments and the assistant instructions.
