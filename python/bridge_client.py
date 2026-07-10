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
 *  File: controller.py
 *  Author: Noah Haskell
 *  Description: The BridgeClient's role is to handle all commuication with the microcontroller through the Arduino Bridge.
"""

# app/bridge_client.py
from arduino.app_utils import *

class BridgeClient:
    """
    Thin wrapper around the Uno Q Bridge/RPC API.

    These methods have the actual calls from the
    Arduino Uno Q Bridge Python library.
    """

    def __init__(self):
        pass

    def get_state(self, side: int) -> bool:
        # 0 = left, 1 = right
        print(f"[bridge] getState({side})")
        return Bridge.call("getState", side)

    def get_mcu_status(self) -> dict:
        # Keep a stable shape for the web controller while still querying firmware state.
        return {
            "connected": True,
            "status": self.get_status(),
        }

    def get_status(self) -> str:
        print("[bridge] getStatus()")
        return Bridge.call("getStatus")

    def get_sensor(self) -> int:
        print(f"[bridge] getSensor()")
        return Bridge.call("getSensor")

    def get_temp(self, side: int) -> float:
        print(f"[bridge] getTemp({side})")
        return Bridge.call("getTemp", side)

    def set_led(self, on: bool) -> None:
        # Set the state of the builtin LED
        print(f"[bridge] setLed({on})")
        Bridge.call("setLed", on)

    def set_mode(self, mode: int) -> None:
        # Set the "mode of the Arduino". Test function only.
        print(f"[bridge] setMode({mode})")
        Bridge.call("setMode", mode)

    def set_state(self, side: int, on: bool) -> None:
        # Set the state of an arm, where 1 = right and 0 = left. When on == True, arm goes down.
        print(f"[bridge] setState({side} side and {on})")
        Bridge.call("setState", side, on)

    def stop_all(self) -> None:
        print("[bridge] stop_all()")
        Bridge.call("stopAll")

    def toggle_state(self) -> None:
        raise NotImplementedError("toggle_state requires a side-specific bridge command")
