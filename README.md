# Uno Q Headset

Uno Q Headset is a prototype control system for a wearable headset built around an Arduino Uno Q and a Python web app. The goal is to coordinate physical hardware behavior on the microcontroller with remote control from a browser over the local network, in order to automate the application of a cold compress on the wearer's eyes for the goal of treating eye itching and pain caused by allergies.

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

## Hardware Control In The Sketch

The C++ side is structured as a small embedded control loop rather than a collection of one-off callbacks. In `sketch/sketch.ino`, the firmware keeps the live hardware state in a few shared variables (arm state, LED state, current mode) and updates them through RPC handlers registered with the Bridge. Those handlers are intentionally lightweight: they change control state or request a mode transition, while the `periodic()` loop is responsible for quickly applying that state to the hardware.

The supporting C++ files reinforce that split: 
- Pin mapping and board-specific constants live in `sketch/Constants.hpp`
- Button handling is wrapped in `DigitalInput` for debouncing and edge detection
- Temperature sensing is isolated in `Thermometer` so the main sketch can treat sensing as a service rather than embedding hardware details inline. 

This way, the firmware can keep safety-sensitive behavior local to the microcontroller and avoid relying on web requests for timing.

Motor control is still in progress, but the architecture already points toward the intended structure: `sketch.ino` computes desired setpoints from the current arm state, and the lower-level motion implementation is expected to consume those setpoints once it is fully wired up.

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

## License
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.