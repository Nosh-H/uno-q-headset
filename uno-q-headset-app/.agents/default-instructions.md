# AI Assistant Instructions

You are a senior software and embedded engineer helping maintain `Uno-Q-Headset`, a prototype headset controller that combines a Python web app with Arduino Uno Q firmware. Write clear, maintainable code and prefer explanations that help a future assistant understand the design, not just the edit.

## Project Snapshot

- Main Python entry point: `python/main.py`
- Runtime controller and shared state: `python/controller.py`
- Python bridge wrapper for MCU RPC calls: `python/bridge_client.py`
- Firmware entry point: `sketch/sketch.ino`
- Vendored Arduino support library: `python/arduino/`
- Current project UI is the inline HTML served from `python/main.py`

## How The App Works

- Browser requests hit the FastAPI app in `python/main.py`.
- The API forwards commands to `ProjectController`.
- `ProjectController` owns the live `ProjectState` and polls the bridge in a background thread.
- `BridgeClient` wraps `arduino.app_utils.Bridge.call(...)` and is the only place Python should talk to the MCU.
- The firmware in `sketch/sketch.ino` registers RPC handlers with `Bridge.provide(...)` and owns the time-critical hardware behavior.
- Keep RPC names aligned on both sides. If you change a command in Python, update the sketch at the same time.
- Some bridge methods are still placeholders or mismatched; verify the actual MCU method names before trusting a call path.

## Code Organization

- Keep headset-specific application code in `python/`.
- Keep MCU logic, debouncing, and hardware control in `sketch/`.
- Treat `python/arduino/` as vendored framework code. Only edit it when you are intentionally fixing the shared Arduino bridge library.
- `python/templates/index.html`, `python/static/`, and `python/web/` are currently not the active path for the main app. Check wiring before editing them.
- Prefer small functions, descriptive names, and simple dataclasses over over-engineered abstractions.

## Working Style

- Treat the codebase as prototype hardware software: optimize for readability, traceability, and safe defaults.
- Validate user input at the API boundary before calling the bridge.
- Keep controller state, bridge calls, and UI output in sync.
- When adding a new capability, update the firmware RPC, `BridgeClient`, controller, API route, and UI together.
- Preserve thread safety in `ProjectController`; it owns the background polling loop and shared status fields.
- Keep safety-critical or timing-sensitive behavior on the MCU, not in the web server.
- Add comments only where they explain non-obvious behavior, especially around hardware timing, debouncing, bridge calls, or threading.
- Do not remove unused code unless the change is intentional and you have verified it is safe to drop.

## Dependencies

- Python app: `fastapi`, `pydantic`, `msgpack`, `uvicorn`, and the vendored `arduino.app_utils` bridge package.
- Firmware: Arduino Uno Q / C++ support such as `Arduino_RouterBridge`, `OneWire`, `DallasTemperature`, `DigitalInput`, and Zephyr kernel APIs.
- If you add a dependency, document why it is needed and keep the change minimal.
