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

# app/controller.py

import threading
import time
from dataclasses import dataclass, asdict
from typing import Optional

from bridge_client import BridgeClient


@dataclass
class ProjectState:
    running: bool = False
    led_on: bool = False
    mode: int = 0
    sensor_value: Optional[int] = None
    left_arm_state: Optional[bool] = None
    right_arm_state: Optional[bool] = None
    left_temp_f: Optional[float] = None
    right_temp_f: Optional[float] = None
    mcu_status: Optional[str] = None
    last_error: Optional[str] = None
    mcu_connected: bool = False


class ProjectController:
    """
    Main application logic.

    The web app talks to this class.
    This class talks to the BridgeClient.
    """

    def __init__(self, bridge: BridgeClient):
        self.bridge = bridge
        self.state = ProjectState()
        self._lock = threading.Lock()
        self._stop_event = threading.Event()
        self._thread: threading.Thread | None = None

    @staticmethod
    def _validate_side(side: int) -> None:
        if side not in (0, 1):
            raise ValueError("side must be 0 (left) or 1 (right)")

    def _set_side_state(self, side: int, value: bool) -> None:
        if side == 0:
            self.state.left_arm_state = value
        else:
            self.state.right_arm_state = value

    def _set_side_temp(self, side: int, value: float) -> None:
        if side == 0:
            self.state.left_temp_f = value
        else:
            self.state.right_temp_f = value

    def start(self) -> None:
        with self._lock:
            if self.state.running:
                return

            self.state.running = True
            self.state.last_error = None

        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def shutdown(self) -> None:
        self._stop_event.set()
        try:
            self.bridge.stop_all()
        finally:
            with self._lock:
                self.state.running = False
                self.state.led_on = False
                self.state.mode = 0
                self.state.mcu_connected = False
                self.state.mcu_status = None

    def _loop(self) -> None:
        while not self._stop_event.is_set():
            try:
                sensor = self.bridge.get_sensor()
                mcu_status = self.bridge.get_mcu_status()
                left_arm_state = self.bridge.get_state(0)
                right_arm_state = self.bridge.get_state(1)
                left_temp_f = self.bridge.get_temp(0)
                right_temp_f = self.bridge.get_temp(1)

                with self._lock:
                    self.state.sensor_value = sensor
                    self.state.mcu_connected = bool(mcu_status.get("connected"))
                    self.state.mcu_status = mcu_status.get("status")
                    self.state.left_arm_state = left_arm_state
                    self.state.right_arm_state = right_arm_state
                    self.state.left_temp_f = left_temp_f
                    self.state.right_temp_f = right_temp_f
                    self.state.last_error = None

            except Exception as exc:
                with self._lock:
                    self.state.last_error = str(exc)
                    self.state.mcu_connected = False

            time.sleep(0.25)

    def get_status(self) -> dict:
        with self._lock:
            return asdict(self.state)

    def get_state(self, side: int) -> bool:
        self._validate_side(side)
        value = self.bridge.get_state(side)

        with self._lock:
            self._set_side_state(side, value)

        return value

    def set_state(self, side: int, state: bool) -> dict:
        self._validate_side(side)
        self.bridge.set_state(side, state)

        with self._lock:
            self._set_side_state(side, state)

        return self.get_status()

    def get_temp(self, side: int) -> float:
        self._validate_side(side)
        value = self.bridge.get_temp(side)

        with self._lock:
            self._set_side_temp(side, value)

        return value

    def set_led(self, on: bool) -> dict:
        self.bridge.set_led(on)

        with self._lock:
            self.state.led_on = on

        return self.get_status()

    def set_mode(self, mode: int) -> dict:
        if mode < 0 or mode > 3:
            raise ValueError("mode must be between 0 and 3")

        self.bridge.set_mode(mode)

        with self._lock:
            self.state.mode = mode

        return self.get_status()

    def emergency_stop(self) -> dict:
        self.bridge.stop_all()

        with self._lock:
            self.state.led_on = False
            self.state.mode = 0

        return self.get_status()
